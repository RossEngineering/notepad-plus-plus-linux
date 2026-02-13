#include "MainWindow.h"
#include "LanguageDetection.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>

#include <QAction>
#include <QCheckBox>
#include <QCloseEvent>
#include <QColor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QKeySequence>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

#include "Scintilla.h"
#include "ScintillaEditBase.h"
#include "SciLexer.h"

#if defined(NPP_HAVE_LEXILLA) && NPP_HAVE_LEXILLA
#include "ILexer.h"
#include "Lexilla.h"
#endif

namespace {

QString FileNameFromPath(const std::string &pathUtf8) {
    if (pathUtf8.empty()) {
        return QStringLiteral("Untitled");
    }
    return QString::fromStdString(std::filesystem::path(pathUtf8).filename().string());
}

sptr_t LineNumberMarginWidth(ScintillaEditBase *editor) {
    return editor->send(
        SCI_TEXTWIDTH,
        STYLE_LINENUMBER,
        reinterpret_cast<sptr_t>("_99999"));
}

const std::map<std::string, QKeySequence> &DefaultShortcutMap() {
    static const std::map<std::string, QKeySequence> kShortcuts = {
        {"file.new", QKeySequence(QStringLiteral("Ctrl+N"))},
        {"file.open", QKeySequence(QStringLiteral("Ctrl+O"))},
        {"file.reload", QKeySequence(QStringLiteral("Ctrl+Shift+R"))},
        {"file.save", QKeySequence(QStringLiteral("Ctrl+S"))},
        {"file.saveAs", QKeySequence(QStringLiteral("Ctrl+Shift+S"))},
        {"tab.close", QKeySequence(QStringLiteral("Ctrl+W"))},
        {"file.quit", QKeySequence(QStringLiteral("Ctrl+Q"))},
        {"edit.find", QKeySequence(QStringLiteral("Ctrl+F"))},
        {"search.findInFiles", QKeySequence(QStringLiteral("Ctrl+Shift+F"))},
        {"edit.replace", QKeySequence(QStringLiteral("Ctrl+H"))},
        {"edit.gotoLine", QKeySequence(QStringLiteral("Ctrl+G"))},
        {"edit.preferences", QKeySequence(QStringLiteral("Ctrl+Comma"))},
        {"language.autoDetect", QKeySequence(QStringLiteral("Ctrl+Alt+L"))},
        {"language.lockCurrent", QKeySequence(QStringLiteral("Ctrl+Alt+Shift+L"))},
        {"tools.runCommand", QKeySequence(QStringLiteral("F5"))},
        {"tools.shortcuts.open", QKeySequence(QStringLiteral("Ctrl+Alt+K"))},
        {"tools.shortcuts.reload", QKeySequence(QStringLiteral("Ctrl+Alt+R"))},
    };
    return kShortcuts;
}

bool IsValidUtf8(std::string_view bytes) {
    size_t index = 0;
    while (index < bytes.size()) {
        const unsigned char lead = static_cast<unsigned char>(bytes[index]);
        if ((lead & 0x80U) == 0U) {
            ++index;
            continue;
        }

        int continuationCount = 0;
        if ((lead & 0xE0U) == 0xC0U) {
            continuationCount = 1;
            if (lead < 0xC2U) {
                return false;
            }
        } else if ((lead & 0xF0U) == 0xE0U) {
            continuationCount = 2;
        } else if ((lead & 0xF8U) == 0xF0U) {
            continuationCount = 3;
            if (lead > 0xF4U) {
                return false;
            }
        } else {
            return false;
        }

        if (index + static_cast<size_t>(continuationCount) >= bytes.size()) {
            return false;
        }

        for (int offset = 1; offset <= continuationCount; ++offset) {
            const unsigned char tail = static_cast<unsigned char>(bytes[index + static_cast<size_t>(offset)]);
            if ((tail & 0xC0U) != 0x80U) {
                return false;
            }
        }

        index += static_cast<size_t>(continuationCount) + 1U;
    }
    return true;
}

QString DecodeUtf16(const std::string &bytes, bool littleEndian) {
    const size_t pairCount = bytes.size() / 2U;
    std::u16string codeUnits;
    codeUnits.reserve(pairCount);
    for (size_t i = 0; i + 1U < bytes.size(); i += 2U) {
        const unsigned char b0 = static_cast<unsigned char>(bytes[i]);
        const unsigned char b1 = static_cast<unsigned char>(bytes[i + 1U]);
        const char16_t value = littleEndian
            ? static_cast<char16_t>(b0 | (static_cast<char16_t>(b1) << 8))
            : static_cast<char16_t>((static_cast<char16_t>(b0) << 8) | b1);
        codeUnits.push_back(value);
    }
    return QString::fromUtf16(codeUnits.data(), static_cast<int>(codeUnits.size()));
}

std::string AsciiLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string PercentString(double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(0) << (value * 100.0);
    return stream.str();
}

std::string PreferredExtensionForLexer(const std::string &lexerName) {
    if (lexerName == "markdown") {
        return ".md";
    }
    if (lexerName == "xml") {
        return ".html";
    }
    if (lexerName == "cpp") {
        return ".cpp";
    }
    if (lexerName == "python") {
        return ".py";
    }
    if (lexerName == "bash") {
        return ".sh";
    }
    if (lexerName == "json") {
        return ".json";
    }
    if (lexerName == "yaml") {
        return ".yaml";
    }
    if (lexerName == "sql") {
        return ".sql";
    }
    return ".txt";
}

std::string LanguageActionIdForLexer(const std::string &lexerName) {
    if (lexerName == "markdown") {
        return "language.set.markdown";
    }
    if (lexerName == "xml") {
        return "language.set.html";
    }
    if (lexerName == "cpp") {
        return "language.set.cpp";
    }
    if (lexerName == "python") {
        return "language.set.python";
    }
    if (lexerName == "bash") {
        return "language.set.bash";
    }
    return "language.set.plain";
}

bool IsIdentifierChar(char ch) {
    const unsigned char uch = static_cast<unsigned char>(ch);
    return std::isalnum(uch) != 0 || ch == '-' || ch == '_' || ch == ':';
}

int ParseThemeColor(const QJsonObject &obj, const char *key, int fallback) {
    const QJsonValue value = obj.value(QString::fromLatin1(key));
    if (!value.isString()) {
        return fallback;
    }
    const QColor color(value.toString());
    if (!color.isValid()) {
        return fallback;
    }
    return color.blue() << 16 | color.green() << 8 | color.red();
}

}  // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), _diagnosticsService(_pathService, _fileSystemService) {
    BuildUi();
    LoadEditorSettings();
    EnsureThemeFile();
    LoadTheme();
    EnsureShortcutConfigFile();
    LoadShortcutOverrides();
    ApplyShortcuts();
    if (!RestoreSession()) {
        NewTab();
    }
    UpdateWindowTitle();
    UpdateCursorStatus();
    UpdateLanguageActionState();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    _closingApplication = true;
    for (int index = _tabs->count() - 1; index >= 0; --index) {
        if (!CloseTabAt(index)) {
            _closingApplication = false;
            event->ignore();
            return;
        }
    }

    SaveSession();
    _closingApplication = false;
    event->accept();
}

void MainWindow::BuildUi() {
    resize(1100, 760);
    BuildTabs();
    BuildActions();
    BuildMenus();
    BuildStatusBar();
}

void MainWindow::BuildMenus() {
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(_actionsById.at("file.new"));
    fileMenu->addAction(_actionsById.at("file.open"));
    fileMenu->addAction(_actionsById.at("file.reload"));
    fileMenu->addSeparator();
    fileMenu->addAction(_actionsById.at("file.save"));
    fileMenu->addAction(_actionsById.at("file.saveAs"));
    fileMenu->addSeparator();
    fileMenu->addAction(_actionsById.at("tab.close"));
    fileMenu->addSeparator();
    fileMenu->addAction(_actionsById.at("file.quit"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(_actionsById.at("edit.find"));
    editMenu->addAction(_actionsById.at("search.findInFiles"));
    editMenu->addAction(_actionsById.at("edit.replace"));
    editMenu->addAction(_actionsById.at("edit.gotoLine"));
    editMenu->addSeparator();
    editMenu->addAction(_actionsById.at("edit.preferences"));

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(_actionsById.at("tools.runCommand"));
    toolsMenu->addSeparator();
    toolsMenu->addAction(_actionsById.at("tools.shortcuts.open"));
    toolsMenu->addAction(_actionsById.at("tools.shortcuts.reload"));

    QMenu *languageMenu = menuBar()->addMenu(tr("&Language"));
    languageMenu->addAction(_actionsById.at("language.autoDetect"));
    languageMenu->addAction(_actionsById.at("language.lockCurrent"));
    languageMenu->addSeparator();
    languageMenu->addAction(_actionsById.at("language.set.plain"));
    languageMenu->addAction(_actionsById.at("language.set.markdown"));
    languageMenu->addAction(_actionsById.at("language.set.html"));
    languageMenu->addAction(_actionsById.at("language.set.cpp"));
    languageMenu->addAction(_actionsById.at("language.set.python"));
    languageMenu->addAction(_actionsById.at("language.set.bash"));
}

void MainWindow::BuildStatusBar() {
    _cursorStatusLabel = new QLabel(this);
    statusBar()->addPermanentWidget(_cursorStatusLabel);
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::BuildActions() {
    const auto registerAction = [this](const std::string &id, const QString &label, auto handler) {
        QAction *action = new QAction(label, this);
        connect(action, &QAction::triggered, this, handler);
        _actionsById.insert_or_assign(id, action);
    };

    registerAction("file.new", tr("New Tab"), [this]() { NewTab(); });
    registerAction("file.open", tr("Open..."), [this]() { OpenFile(); });
    registerAction("file.reload", tr("Reload"), [this]() { ReloadCurrentFile(); });
    registerAction("file.save", tr("Save"), [this]() { SaveCurrentFile(); });
    registerAction("file.saveAs", tr("Save As..."), [this]() { SaveCurrentFileAs(); });
    registerAction("tab.close", tr("Close Tab"), [this]() { CloseTab(); });
    registerAction("file.quit", tr("Quit"), [this]() { close(); });

    registerAction("edit.find", tr("Find..."), [this]() { OnFind(); });
    registerAction("search.findInFiles", tr("Find in Files..."), [this]() { OnFindInFiles(); });
    registerAction("edit.replace", tr("Replace..."), [this]() { OnReplace(); });
    registerAction("edit.gotoLine", tr("Go To Line..."), [this]() { OnGoToLine(); });
    registerAction("edit.preferences", tr("Preferences..."), [this]() { OnPreferences(); });
    registerAction("tools.runCommand", tr("Run Command..."), [this]() { OnRunCommand(); });
    registerAction("language.autoDetect", tr("Auto Detect Language"), [this]() { OnAutoDetectLanguage(); });
    registerAction("language.lockCurrent", tr("Lock Current Language"), [this]() { OnToggleLexerLock(); });
    _actionsById.at("language.lockCurrent")->setCheckable(true);
    registerAction("language.set.plain", tr("Plain Text"), [this]() { SetCurrentEditorManualLexer("null"); });
    registerAction("language.set.markdown", tr("Markdown"), [this]() { SetCurrentEditorManualLexer("markdown"); });
    registerAction("language.set.html", tr("HTML/XML"), [this]() { SetCurrentEditorManualLexer("xml"); });
    registerAction("language.set.cpp", tr("C/C++"), [this]() { SetCurrentEditorManualLexer("cpp"); });
    registerAction("language.set.python", tr("Python"), [this]() { SetCurrentEditorManualLexer("python"); });
    registerAction("language.set.bash", tr("Bash/Shell"), [this]() { SetCurrentEditorManualLexer("bash"); });
    _actionsById.at("language.set.plain")->setCheckable(true);
    _actionsById.at("language.set.markdown")->setCheckable(true);
    _actionsById.at("language.set.html")->setCheckable(true);
    _actionsById.at("language.set.cpp")->setCheckable(true);
    _actionsById.at("language.set.python")->setCheckable(true);
    _actionsById.at("language.set.bash")->setCheckable(true);

    registerAction(
        "tools.shortcuts.open",
        tr("Open Shortcut Overrides"),
        [this]() { OpenShortcutConfigFile(); });
    registerAction(
        "tools.shortcuts.reload",
        tr("Reload Shortcut Overrides"),
        [this]() { ReloadShortcuts(); });
}

void MainWindow::BuildTabs() {
    _tabs = new QTabWidget(this);
    _tabs->setTabsClosable(true);
    _tabs->setMovable(true);
    _tabs->setDocumentMode(true);

    connect(_tabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
        CloseTabAt(index);
    });

    connect(_tabs, &QTabWidget::currentChanged, this, [this](int) {
        UpdateWindowTitle();
        UpdateCursorStatus();
        UpdateLanguageActionState();
    });

    setCentralWidget(_tabs);
}

ScintillaEditBase *MainWindow::CreateEditor() {
    auto *editor = new ScintillaEditBase(this);

    editor->send(SCI_SETCODEPAGE, SC_CP_UTF8);
    editor->send(SCI_SETUNDOCOLLECTION, 1);
    editor->sends(SCI_STYLESETFONT, STYLE_DEFAULT, "Noto Sans Mono");
    editor->send(SCI_STYLESETSIZE, STYLE_DEFAULT, 11);
    editor->send(SCI_STYLECLEARALL);
    editor->send(SCI_SETCARETLINEVISIBLE, 1);
    editor->send(SCI_SETCARETLINEBACK, 0xF6F6F6);
    editor->send(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
    editor->send(SCI_SETMARGINWIDTHN, 0, LineNumberMarginWidth(editor));
    editor->send(SCI_SETWRAPMODE, SC_WRAP_NONE);
    editor->send(SCI_SETTABWIDTH, 4);
    editor->send(SCI_GRABFOCUS);

    connect(editor, &ScintillaEditBase::notifyChange, this, [this, editor]() {
        auto it = _editorStates.find(editor);
        if (it == _editorStates.end()) {
            return;
        }
        it->second.dirty = true;
        UpdateTabTitle(editor);
    });

    connect(editor, &ScintillaEditBase::savePointChanged, this, [this, editor](bool dirty) {
        auto it = _editorStates.find(editor);
        if (it == _editorStates.end()) {
            return;
        }
        it->second.dirty = dirty;
        UpdateTabTitle(editor);
    });

    connect(editor, &ScintillaEditBase::updateUi, this, [this](Scintilla::Update) {
        UpdateCursorStatus();
    });

    connect(editor, &ScintillaEditBase::charAdded, this, [this, editor](int ch) {
        MaybeAutoCloseHtmlTag(editor, ch);
    });

    ApplyTheme(editor);
    ApplyLexerByName(editor, "null");
    ApplyEditorSettings(editor);

    return editor;
}

ScintillaEditBase *MainWindow::CurrentEditor() const {
    if (!_tabs) {
        return nullptr;
    }
    return qobject_cast<ScintillaEditBase *>(_tabs->currentWidget());
}

ScintillaEditBase *MainWindow::EditorAt(int index) const {
    if (!_tabs) {
        return nullptr;
    }
    return qobject_cast<ScintillaEditBase *>(_tabs->widget(index));
}

void MainWindow::NewTab() {
    auto *editor = CreateEditor();
    EditorState state;
    state.lexerName = "null";
    _editorStates.insert_or_assign(editor, state);
    ApplyLexerByName(editor, state.lexerName);

    const int index = _tabs->addTab(editor, tr("Untitled"));
    _tabs->setCurrentIndex(index);
    editor->setFocus();
    UpdateLanguageActionState();
}

void MainWindow::OpenFile() {
    const QString chosen = QFileDialog::getOpenFileName(this, tr("Open File"));
    if (chosen.isEmpty()) {
        return;
    }

    auto *editor = CurrentEditor();
    if (!editor) {
        NewTab();
        editor = CurrentEditor();
    }

    auto &state = _editorStates[editor];
    if (!state.filePathUtf8.empty() || !GetEditorText(editor).empty() || state.dirty) {
        NewTab();
        editor = CurrentEditor();
    }

    if (LoadFileIntoEditor(editor, ToUtf8(chosen))) {
        statusBar()->showMessage(tr("Opened %1").arg(chosen), 2000);
    }
}

void MainWindow::ReloadCurrentFile() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end() || stateIt->second.filePathUtf8.empty()) {
        QMessageBox::information(this, tr("Reload"), tr("Current tab has no file on disk."));
        return;
    }

    if (stateIt->second.dirty) {
        const auto choice = QMessageBox::question(
            this,
            tr("Reload"),
            tr("Discard unsaved changes and reload from disk?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (choice != QMessageBox::Yes) {
            return;
        }
    }

    if (LoadFileIntoEditor(editor, stateIt->second.filePathUtf8)) {
        statusBar()->showMessage(tr("Reloaded %1").arg(ToQString(stateIt->second.filePathUtf8)), 2000);
    }
}

bool MainWindow::SaveCurrentFile() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return false;
    }

    auto &state = _editorStates[editor];
    if (state.filePathUtf8.empty()) {
        return SaveCurrentFileAs();
    }

    if (!SaveEditorToFile(editor, state.filePathUtf8)) {
        return false;
    }

    statusBar()->showMessage(tr("Saved %1").arg(ToQString(state.filePathUtf8)), 2000);
    return true;
}

bool MainWindow::SaveCurrentFileAs() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return false;
    }

    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return false;
    }

    const std::string lexerName = stateIt->second.lexerName.empty() ? "null" : stateIt->second.lexerName;
    const std::string suggestedExtension = PreferredExtensionForLexer(lexerName);

    QString suggestedPath;
    if (!stateIt->second.filePathUtf8.empty()) {
        suggestedPath = ToQString(stateIt->second.filePathUtf8);
    } else {
        suggestedPath = QStringLiteral("untitled%1").arg(ToQString(suggestedExtension));
    }

    QString target = QFileDialog::getSaveFileName(this, tr("Save File As"), suggestedPath);
    if (target.isEmpty()) {
        return false;
    }

    const std::filesystem::path chosenPath(ToUtf8(target));
    if (chosenPath.extension().empty() && !suggestedExtension.empty()) {
        target += ToQString(suggestedExtension);
    }

    if (!SaveEditorToFile(editor, ToUtf8(target))) {
        return false;
    }

    statusBar()->showMessage(tr("Saved %1").arg(target), 2000);
    return true;
}

bool MainWindow::CloseTabAt(int index) {
    ScintillaEditBase *editor = EditorAt(index);
    if (!editor) {
        return false;
    }

    auto stateIt = _editorStates.find(editor);
    if (stateIt != _editorStates.end() && stateIt->second.dirty) {
        const QString label = FileNameFromPath(stateIt->second.filePathUtf8);
        const auto choice = QMessageBox::question(
            this,
            tr("Unsaved Changes"),
            tr("Save changes to %1 before closing?").arg(label),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::Yes);

        if (choice == QMessageBox::Cancel) {
            return false;
        }
        if (choice == QMessageBox::Yes && !_tabs->widget(index)->isHidden()) {
            _tabs->setCurrentIndex(index);
            if (!SaveCurrentFile()) {
                return false;
            }
        }
    }

    _tabs->removeTab(index);
    _editorStates.erase(editor);
    editor->deleteLater();

    if (_tabs->count() == 0 && !_closingApplication) {
        NewTab();
    }

    return true;
}

void MainWindow::CloseTab() {
    CloseTabAt(_tabs->currentIndex());
}

void MainWindow::UpdateTabTitle(ScintillaEditBase *editor) {
    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return;
    }

    const QString base = FileNameFromPath(stateIt->second.filePathUtf8);
    const QString title = stateIt->second.dirty ? QStringLiteral("*%1").arg(base) : base;
    const int index = _tabs->indexOf(editor);
    if (index >= 0) {
        _tabs->setTabText(index, title);
    }

    UpdateWindowTitle();
}

void MainWindow::UpdateWindowTitle() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        setWindowTitle(QStringLiteral("Notepad++ Linux"));
        return;
    }

    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        setWindowTitle(QStringLiteral("Notepad++ Linux"));
        return;
    }

    const QString fileName = FileNameFromPath(stateIt->second.filePathUtf8);
    setWindowTitle(tr("%1 - Notepad++ Linux").arg(fileName));
}

void MainWindow::UpdateCursorStatus() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        _cursorStatusLabel->setText(tr("Ln 1, Col 1"));
        return;
    }

    const auto position = static_cast<int>(editor->send(SCI_GETCURRENTPOS));
    const auto line = static_cast<int>(editor->send(SCI_LINEFROMPOSITION, position));
    const auto column = static_cast<int>(editor->send(SCI_GETCOLUMN, position));

    _cursorStatusLabel->setText(tr("Ln %1, Col %2").arg(line + 1).arg(column + 1));
}

void MainWindow::OnFind() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Find"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();
    auto *needleEdit = new QLineEdit(&dialog);
    needleEdit->setText(ToQString(_lastFindUtf8.empty() ? GetSelectedText(editor) : _lastFindUtf8));
    auto *matchCaseCheck = new QCheckBox(tr("Match case"), &dialog);
    form->addRow(tr("Find what:"), needleEdit);
    form->addRow(QString(), matchCaseCheck);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const std::string needleUtf8 = ToUtf8(needleEdit->text());
    if (needleUtf8.empty()) {
        return;
    }

    _lastFindUtf8 = needleUtf8;
    if (!FindNextInEditor(editor, needleUtf8, matchCaseCheck->isChecked())) {
        QMessageBox::information(this, tr("Find"), tr("No matches found."));
    }
}

void MainWindow::OnReplace() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    enum class ReplaceAction {
        kNone = 0,
        kSingle,
        kAll,
    };

    ReplaceAction action = ReplaceAction::kNone;

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Replace"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();
    auto *needleEdit = new QLineEdit(&dialog);
    needleEdit->setText(ToQString(_lastFindUtf8.empty() ? GetSelectedText(editor) : _lastFindUtf8));
    auto *replacementEdit = new QLineEdit(&dialog);
    auto *matchCaseCheck = new QCheckBox(tr("Match case"), &dialog);

    form->addRow(tr("Find what:"), needleEdit);
    form->addRow(tr("Replace with:"), replacementEdit);
    form->addRow(QString(), matchCaseCheck);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, &dialog);
    QPushButton *replaceButton = buttons->addButton(tr("Replace"), QDialogButtonBox::AcceptRole);
    QPushButton *replaceAllButton = buttons->addButton(tr("Replace All"), QDialogButtonBox::ActionRole);
    layout->addWidget(buttons);

    connect(replaceButton, &QPushButton::clicked, &dialog, [&dialog, &action]() {
        action = ReplaceAction::kSingle;
        dialog.accept();
    });
    connect(replaceAllButton, &QPushButton::clicked, &dialog, [&dialog, &action]() {
        action = ReplaceAction::kAll;
        dialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const std::string needleUtf8 = ToUtf8(needleEdit->text());
    if (needleUtf8.empty()) {
        return;
    }

    const std::string replacementUtf8 = ToUtf8(replacementEdit->text());
    const bool matchCase = matchCaseCheck->isChecked();
    _lastFindUtf8 = needleUtf8;

    if (action == ReplaceAction::kSingle) {
        if (!FindNextInEditor(editor, needleUtf8, matchCase)) {
            QMessageBox::information(this, tr("Replace"), tr("No matches found."));
            return;
        }
        editor->sends(SCI_REPLACESEL, 0, replacementUtf8.c_str());
        return;
    }

    const int replacements = ReplaceAllInEditor(editor, needleUtf8, replacementUtf8, matchCase);
    QMessageBox::information(
        this,
        tr("Replace All"),
        tr("Replaced %1 occurrence(s).").arg(replacements));
}

void MainWindow::OnFindInFiles() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Find in Files"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();

    auto *needleEdit = new QLineEdit(&dialog);
    needleEdit->setText(ToQString(_lastFindUtf8));

    auto *folderEdit = new QLineEdit(&dialog);
    QString defaultRoot = QDir::homePath();
    ScintillaEditBase *editor = CurrentEditor();
    if (editor) {
        const auto stateIt = _editorStates.find(editor);
        if (stateIt != _editorStates.end() && !stateIt->second.filePathUtf8.empty()) {
            defaultRoot = QString::fromStdString(
                std::filesystem::path(stateIt->second.filePathUtf8).parent_path().string());
        }
    }
    folderEdit->setText(defaultRoot);

    auto *browseButton = new QPushButton(tr("Browse..."), &dialog);
    auto *folderLayout = new QHBoxLayout();
    folderLayout->addWidget(folderEdit);
    folderLayout->addWidget(browseButton);

    auto *matchCaseCheck = new QCheckBox(tr("Match case"), &dialog);

    form->addRow(tr("Find what:"), needleEdit);
    form->addRow(tr("Folder:"), folderLayout);
    form->addRow(QString(), matchCaseCheck);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(browseButton, &QPushButton::clicked, &dialog, [this, folderEdit]() {
        const QString selected = QFileDialog::getExistingDirectory(
            this,
            tr("Select Search Root"),
            folderEdit->text());
        if (!selected.isEmpty()) {
            folderEdit->setText(selected);
        }
    });
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const std::string needleUtf8 = ToUtf8(needleEdit->text());
    const std::string rootPathUtf8 = ToUtf8(folderEdit->text());
    if (needleUtf8.empty() || rootPathUtf8.empty()) {
        return;
    }

    _lastFindUtf8 = needleUtf8;
    npp::platform::ListDirectoryOptions listOptions;
    listOptions.recursive = true;
    listOptions.includeDirectories = false;

    const auto listed = _fileSystemService.ListDirectory(rootPathUtf8, listOptions);
    if (!listed.ok()) {
        QMessageBox::warning(
            this,
            tr("Find in Files"),
            tr("Unable to enumerate folder:\n%1").arg(ToQString(listed.status.message)));
        return;
    }

    const bool matchCase = matchCaseCheck->isChecked();
    const std::string needleCmp = matchCase ? needleUtf8 : AsciiLower(needleUtf8);

    std::ostringstream report;
    int matchCount = 0;
    constexpr int kMaxReportedMatches = 500;
    for (const std::string &path : *listed.value) {
        const auto contentOr = _fileSystemService.ReadTextFile(path);
        if (!contentOr.ok()) {
            continue;
        }

        TextEncoding fileEncoding = TextEncoding::kUtf8;
        std::string utf8Content;
        if (!DecodeTextForEditor(*contentOr.value, &fileEncoding, &utf8Content)) {
            continue;
        }

        std::istringstream stream(utf8Content);
        std::string line;
        int lineNumber = 0;
        while (std::getline(stream, line)) {
            ++lineNumber;
            const std::string lineCmp = matchCase ? line : AsciiLower(line);
            if (lineCmp.find(needleCmp) == std::string::npos) {
                continue;
            }

            ++matchCount;
            if (matchCount <= kMaxReportedMatches) {
                report << path << ":" << lineNumber << ": " << line << "\n";
            }
        }
    }

    if (matchCount == 0) {
        QMessageBox::information(this, tr("Find in Files"), tr("No matches found."));
        return;
    }

    if (matchCount > kMaxReportedMatches) {
        report << "\n... output truncated after " << kMaxReportedMatches << " matches.\n";
    }

    QDialog resultsDialog(this);
    resultsDialog.resize(900, 600);
    resultsDialog.setWindowTitle(tr("Find in Files Results (%1 matches)").arg(matchCount));
    auto *resultsLayout = new QVBoxLayout(&resultsDialog);
    auto *resultsText = new QTextEdit(&resultsDialog);
    resultsText->setReadOnly(true);
    resultsText->setPlainText(QString::fromUtf8(report.str().c_str()));
    resultsLayout->addWidget(resultsText);
    auto *closeButtons = new QDialogButtonBox(QDialogButtonBox::Close, &resultsDialog);
    connect(closeButtons, &QDialogButtonBox::rejected, &resultsDialog, &QDialog::reject);
    connect(closeButtons, &QDialogButtonBox::accepted, &resultsDialog, &QDialog::accept);
    resultsLayout->addWidget(closeButtons);
    resultsDialog.exec();
}

void MainWindow::OnGoToLine() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const int maxLine = static_cast<int>(editor->send(SCI_GETLINECOUNT));
    bool ok = false;
    const int line = QInputDialog::getInt(
        this,
        tr("Go To Line"),
        tr("Line number:"),
        1,
        1,
        std::max(maxLine, 1),
        1,
        &ok);
    if (!ok) {
        return;
    }

    editor->send(SCI_GOTOLINE, static_cast<uptr_t>(line - 1));
    editor->send(SCI_SCROLLCARET);
    UpdateCursorStatus();
}

void MainWindow::OnPreferences() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Preferences"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();
    auto *tabWidthSpin = new QSpinBox(&dialog);
    tabWidthSpin->setRange(1, 12);
    tabWidthSpin->setValue(_editorSettings.tabWidth);
    auto *wrapCheck = new QCheckBox(tr("Enable line wrap"), &dialog);
    wrapCheck->setChecked(_editorSettings.wrapEnabled);
    auto *lineNumbersCheck = new QCheckBox(tr("Show line numbers"), &dialog);
    lineNumbersCheck->setChecked(_editorSettings.showLineNumbers);
    auto *autoCloseHtmlTagsCheck = new QCheckBox(tr("Auto-close HTML/XML tags"), &dialog);
    autoCloseHtmlTagsCheck->setChecked(_editorSettings.autoCloseHtmlTags);

    form->addRow(tr("Tab width:"), tabWidthSpin);
    form->addRow(QString(), wrapCheck);
    form->addRow(QString(), lineNumbersCheck);
    form->addRow(QString(), autoCloseHtmlTagsCheck);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    _editorSettings.tabWidth = tabWidthSpin->value();
    _editorSettings.wrapEnabled = wrapCheck->isChecked();
    _editorSettings.showLineNumbers = lineNumbersCheck->isChecked();
    _editorSettings.autoCloseHtmlTags = autoCloseHtmlTagsCheck->isChecked();
    ApplyEditorSettingsToAllEditors();
    SaveEditorSettings();
    statusBar()->showMessage(tr("Preferences updated"), 1500);
}

void MainWindow::OnRunCommand() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Run Command"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();

    auto *commandEdit = new QLineEdit(&dialog);
    commandEdit->setText(ToQString(_lastRunCommandUtf8));

    auto *workingDirEdit = new QLineEdit(&dialog);
    if (_lastRunWorkingDirUtf8.empty()) {
        ScintillaEditBase *editor = CurrentEditor();
        if (editor) {
            const auto stateIt = _editorStates.find(editor);
            if (stateIt != _editorStates.end() && !stateIt->second.filePathUtf8.empty()) {
                workingDirEdit->setText(QString::fromStdString(
                    std::filesystem::path(stateIt->second.filePathUtf8).parent_path().string()));
            } else {
                workingDirEdit->setText(QDir::homePath());
            }
        } else {
            workingDirEdit->setText(QDir::homePath());
        }
    } else {
        workingDirEdit->setText(ToQString(_lastRunWorkingDirUtf8));
    }

    auto *browseButton = new QPushButton(tr("Browse..."), &dialog);
    auto *workingDirLayout = new QHBoxLayout();
    workingDirLayout->addWidget(workingDirEdit);
    workingDirLayout->addWidget(browseButton);

    form->addRow(tr("Command:"), commandEdit);
    form->addRow(tr("Working dir:"), workingDirLayout);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(browseButton, &QPushButton::clicked, &dialog, [this, workingDirEdit]() {
        const QString selected = QFileDialog::getExistingDirectory(
            this,
            tr("Select Working Directory"),
            workingDirEdit->text());
        if (!selected.isEmpty()) {
            workingDirEdit->setText(selected);
        }
    });
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString command = commandEdit->text().trimmed();
    const QString workingDir = workingDirEdit->text().trimmed();
    if (command.isEmpty()) {
        return;
    }

    const QStringList parts = QProcess::splitCommand(command);
    if (parts.isEmpty()) {
        QMessageBox::warning(this, tr("Run Command"), tr("Invalid command line."));
        return;
    }

    npp::platform::ProcessSpec spec;
    spec.program = ToUtf8(parts.at(0));
    for (int i = 1; i < parts.size(); ++i) {
        spec.args.push_back(ToUtf8(parts.at(i)));
    }
    if (!workingDir.isEmpty()) {
        spec.workingDirectoryUtf8 = ToUtf8(workingDir);
    }

    _lastRunCommandUtf8 = ToUtf8(command);
    _lastRunWorkingDirUtf8 = ToUtf8(workingDir);

    const auto result = _processService.Run(spec, std::chrono::milliseconds(60000));
    if (!result.ok()) {
        QMessageBox::warning(
            this,
            tr("Run Command"),
            tr("Command failed to run:\n%1").arg(ToQString(result.status.message)));
        return;
    }

    QMessageBox::information(
        this,
        tr("Run Command"),
        tr("Command finished with exit code %1.").arg(result.value->exitCode));
}

void MainWindow::OnAutoDetectLanguage() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }
    auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return;
    }
    stateIt->second.lexerManualLock = false;
    AutoDetectAndApplyLexer(editor, stateIt->second.filePathUtf8, GetEditorText(editor), "manual-auto-detect");
    UpdateLanguageActionState();
}

void MainWindow::OnToggleLexerLock() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }
    auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return;
    }
    stateIt->second.lexerManualLock = !stateIt->second.lexerManualLock;
    if (!stateIt->second.lexerManualLock) {
        AutoDetectAndApplyLexer(editor, stateIt->second.filePathUtf8, GetEditorText(editor), "unlock-auto-detect");
    } else {
        stateIt->second.lexerReason = "manual-lock";
        stateIt->second.lexerConfidence = 1.0;
        statusBar()->showMessage(
            tr("Language lock enabled (%1)").arg(ToQString(stateIt->second.lexerName.empty() ? "null" : stateIt->second.lexerName)),
            2000);
    }
    UpdateLanguageActionState();
}

void MainWindow::SetCurrentEditorManualLexer(const std::string &lexerName) {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }
    auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return;
    }
    stateIt->second.lexerManualLock = true;
    stateIt->second.lexerName = lexerName;
    stateIt->second.lexerConfidence = 1.0;
    stateIt->second.lexerReason = "manual-selection";
    ApplyLexerByName(editor, lexerName);
    statusBar()->showMessage(
        tr("Language set to %1 (manual lock)").arg(ToQString(lexerName)),
        2000);
    UpdateLanguageActionState();
}

void MainWindow::UpdateLanguageActionState() {
    auto lockIt = _actionsById.find("language.lockCurrent");
    if (lockIt == _actionsById.end() || !lockIt->second) {
        return;
    }

    const std::vector<std::string> languageActionIds = {
        "language.set.plain",
        "language.set.markdown",
        "language.set.html",
        "language.set.cpp",
        "language.set.python",
        "language.set.bash",
    };

    const auto setLanguageActionsEnabled = [this, &languageActionIds](bool enabled) {
        for (const std::string &id : languageActionIds) {
            const auto it = _actionsById.find(id);
            if (it != _actionsById.end() && it->second) {
                it->second->setEnabled(enabled);
            }
        }
    };

    const auto clearLanguageChecks = [this, &languageActionIds]() {
        for (const std::string &id : languageActionIds) {
            const auto it = _actionsById.find(id);
            if (it != _actionsById.end() && it->second) {
                QSignalBlocker blocker(it->second);
                it->second->setChecked(false);
            }
        }
    };

    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        lockIt->second->setChecked(false);
        lockIt->second->setEnabled(false);
        clearLanguageChecks();
        setLanguageActionsEnabled(false);
        return;
    }
    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        lockIt->second->setChecked(false);
        lockIt->second->setEnabled(false);
        clearLanguageChecks();
        setLanguageActionsEnabled(false);
        return;
    }

    lockIt->second->setEnabled(true);
    lockIt->second->setChecked(stateIt->second.lexerManualLock);
    setLanguageActionsEnabled(true);
    clearLanguageChecks();

    const std::string activeLexer = stateIt->second.lexerName.empty() ? "null" : stateIt->second.lexerName;
    const std::string activeActionId = LanguageActionIdForLexer(activeLexer);
    const auto activeIt = _actionsById.find(activeActionId);
    if (activeIt != _actionsById.end() && activeIt->second) {
        QSignalBlocker blocker(activeIt->second);
        activeIt->second->setChecked(true);
    }
}

bool MainWindow::FindNextInEditor(
    ScintillaEditBase *editor,
    const std::string &needleUtf8,
    bool matchCase) {
    if (!editor || needleUtf8.empty()) {
        return false;
    }

    const uptr_t flags = matchCase ? SCFIND_MATCHCASE : 0;
    const sptr_t documentLength = editor->send(SCI_GETTEXTLENGTH);
    const sptr_t start = editor->send(SCI_GETSELECTIONEND);
    const sptr_t needleLength = static_cast<sptr_t>(needleUtf8.size());

    editor->send(SCI_SETSEARCHFLAGS, flags);
    editor->send(SCI_SETTARGETRANGE, start, documentLength);
    sptr_t found = editor->sends(SCI_SEARCHINTARGET, needleLength, needleUtf8.c_str());

    if (found < 0 && start > 0) {
        editor->send(SCI_SETTARGETRANGE, 0, start);
        found = editor->sends(SCI_SEARCHINTARGET, needleLength, needleUtf8.c_str());
    }

    if (found < 0) {
        return false;
    }

    const sptr_t targetStart = editor->send(SCI_GETTARGETSTART);
    const sptr_t targetEnd = editor->send(SCI_GETTARGETEND);
    editor->send(SCI_SETSEL, targetStart, targetEnd);
    editor->send(SCI_SCROLLCARET);
    return true;
}

int MainWindow::ReplaceAllInEditor(
    ScintillaEditBase *editor,
    const std::string &needleUtf8,
    const std::string &replacementUtf8,
    bool matchCase) {
    if (!editor || needleUtf8.empty()) {
        return 0;
    }

    const uptr_t flags = matchCase ? SCFIND_MATCHCASE : 0;
    const sptr_t needleLength = static_cast<sptr_t>(needleUtf8.size());
    const sptr_t replacementLength = static_cast<sptr_t>(replacementUtf8.size());

    editor->send(SCI_BEGINUNDOACTION);
    editor->send(SCI_SETSEARCHFLAGS, flags);
    editor->send(SCI_SETTARGETRANGE, 0, editor->send(SCI_GETTEXTLENGTH));

    int replaced = 0;
    while (true) {
        const sptr_t found = editor->sends(SCI_SEARCHINTARGET, needleLength, needleUtf8.c_str());
        if (found < 0) {
            break;
        }

        const sptr_t targetStart = editor->send(SCI_GETTARGETSTART);
        editor->sends(SCI_REPLACETARGET, replacementLength, replacementUtf8.c_str());
        ++replaced;

        const sptr_t nextStart = targetStart + replacementLength;
        const sptr_t docLength = editor->send(SCI_GETTEXTLENGTH);
        editor->send(SCI_SETTARGETRANGE, nextStart, docLength);
    }

    editor->send(SCI_ENDUNDOACTION);
    return replaced;
}

std::string MainWindow::GetEditorText(ScintillaEditBase *editor) const {
    if (!editor) {
        return {};
    }

    const auto textLength = static_cast<size_t>(editor->send(SCI_GETTEXTLENGTH));
    std::string text(textLength + 1, '\0');
    editor->send(SCI_GETTEXT, textLength + 1, reinterpret_cast<sptr_t>(text.data()));
    if (!text.empty()) {
        text.resize(textLength);
    }
    return text;
}

std::string MainWindow::GetSelectedText(ScintillaEditBase *editor) const {
    if (!editor) {
        return {};
    }

    const auto start = static_cast<size_t>(editor->send(SCI_GETSELECTIONSTART));
    const auto end = static_cast<size_t>(editor->send(SCI_GETSELECTIONEND));
    if (end <= start) {
        return {};
    }

    const size_t length = end - start;
    std::string text(length + 1, '\0');
    editor->send(SCI_GETSELTEXT, 0, reinterpret_cast<sptr_t>(text.data()));
    text.resize(length);
    return text;
}

void MainWindow::SetEditorText(ScintillaEditBase *editor, const std::string &textUtf8) {
    if (!editor) {
        return;
    }

    editor->sends(SCI_SETTEXT, 0, textUtf8.c_str());
}

void MainWindow::SetEditorEolMode(ScintillaEditBase *editor, int eolMode) {
    if (!editor) {
        return;
    }
    editor->send(SCI_SETEOLMODE, static_cast<uptr_t>(eolMode));
    editor->send(SCI_CONVERTEOLS, static_cast<uptr_t>(eolMode));
}

int MainWindow::DetectDominantEolMode(const std::string &textUtf8) const {
    size_t crlfCount = 0;
    size_t lfCount = 0;
    size_t crCount = 0;

    for (size_t i = 0; i < textUtf8.size(); ++i) {
        if (textUtf8[i] == '\r') {
            if (i + 1 < textUtf8.size() && textUtf8[i + 1] == '\n') {
                ++crlfCount;
                ++i;
            } else {
                ++crCount;
            }
        } else if (textUtf8[i] == '\n') {
            ++lfCount;
        }
    }

    if (crlfCount >= lfCount && crlfCount >= crCount && crlfCount > 0) {
        return SC_EOL_CRLF;
    }
    if (lfCount >= crCount && lfCount > 0) {
        return SC_EOL_LF;
    }
    if (crCount > 0) {
        return SC_EOL_CR;
    }
    return SC_EOL_LF;
}

void MainWindow::MaybeAutoCloseHtmlTag(ScintillaEditBase *editor, int ch) {
    if (!editor || ch != '>') {
        return;
    }
    if (_suppressAutoCloseHandler || !_editorSettings.autoCloseHtmlTags) {
        return;
    }

    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return;
    }
    if (stateIt->second.lexerName != "xml") {
        return;
    }

    const sptr_t caretPos = editor->send(SCI_GETCURRENTPOS);
    if (caretPos <= 1) {
        return;
    }

    const sptr_t line = editor->send(SCI_LINEFROMPOSITION, caretPos);
    const sptr_t lineStart = editor->send(SCI_POSITIONFROMLINE, line);
    const sptr_t lookbackStart = std::max<sptr_t>(lineStart, caretPos - 512);
    const sptr_t lookbackLength = caretPos - lookbackStart;
    if (lookbackLength <= 0) {
        return;
    }

    std::string before(static_cast<size_t>(lookbackLength), '\0');
    for (sptr_t i = 0; i < lookbackLength; ++i) {
        before[static_cast<size_t>(i)] =
            static_cast<char>(editor->send(SCI_GETCHARAT, lookbackStart + i));
    }

    const size_t ltPos = before.find_last_of('<');
    if (ltPos == std::string::npos || ltPos + 1 >= before.size()) {
        return;
    }

    std::string inside = before.substr(ltPos + 1);
    if (inside.empty()) {
        return;
    }
    if (inside[0] == '/' || inside[0] == '!' || inside[0] == '?') {
        return;
    }
    if (!inside.empty() && inside.back() == '>') {
        inside.pop_back();
    }
    if (inside.empty()) {
        return;
    }
    if (!inside.empty() && inside.back() == '/') {
        return;
    }

    size_t tagEnd = 0;
    while (tagEnd < inside.size() && IsIdentifierChar(inside[tagEnd])) {
        ++tagEnd;
    }
    if (tagEnd == 0) {
        return;
    }

    const std::string tagName = inside.substr(0, tagEnd);
    const std::string tagLower = AsciiLower(tagName);
    static const std::set<std::string> kVoidTags = {
        "area", "base", "br", "col", "embed", "hr", "img",
        "input", "link", "meta", "param", "source", "track", "wbr",
    };
    if (kVoidTags.find(tagLower) != kVoidTags.end()) {
        return;
    }

    const sptr_t lookaheadEnd = std::min<sptr_t>(editor->send(SCI_GETTEXTLENGTH), caretPos + 2 + static_cast<sptr_t>(tagName.size()));
    if (lookaheadEnd > caretPos) {
        std::string after(static_cast<size_t>(lookaheadEnd - caretPos), '\0');
        for (sptr_t i = 0; i < lookaheadEnd - caretPos; ++i) {
            after[static_cast<size_t>(i)] =
                static_cast<char>(editor->send(SCI_GETCHARAT, caretPos + i));
        }
        std::string prefix = "</" + tagName;
        if (after.rfind(prefix, 0) == 0) {
            return;
        }
    }

    const std::string closing = "</" + tagName + ">";
    _suppressAutoCloseHandler = true;
    editor->send(SCI_INSERTTEXT, caretPos, reinterpret_cast<sptr_t>(closing.c_str()));
    editor->send(SCI_GOTOPOS, caretPos);
    _suppressAutoCloseHandler = false;
}

bool MainWindow::LoadFileIntoEditor(ScintillaEditBase *editor, const std::string &pathUtf8) {
    const auto read = _fileSystemService.ReadTextFile(pathUtf8);
    if (!read.ok()) {
        QMessageBox::critical(
            this,
            tr("Open Failed"),
            tr("Unable to open file:\n%1").arg(ToQString(read.status.message)));
        return false;
    }

    TextEncoding encoding = TextEncoding::kUtf8;
    std::string utf8Text;
    if (!DecodeTextForEditor(*read.value, &encoding, &utf8Text)) {
        QMessageBox::critical(
            this,
            tr("Open Failed"),
            tr("Unable to decode file with supported encodings."));
        return false;
    }

    const int eolMode = DetectDominantEolMode(utf8Text);
    SetEditorText(editor, utf8Text);
    SetEditorEolMode(editor, eolMode);
    editor->send(SCI_SETSAVEPOINT);

    auto &state = _editorStates[editor];
    state.filePathUtf8 = pathUtf8;
    state.dirty = false;
    state.encoding = encoding;
    state.eolMode = eolMode;
    AutoDetectAndApplyLexer(editor, pathUtf8, utf8Text, "open");
    UpdateTabTitle(editor);
    statusBar()->showMessage(
        tr("Encoding: %1, EOL: %2")
            .arg(EncodingLabel(encoding))
            .arg(eolMode == SC_EOL_CRLF ? QStringLiteral("CRLF") : (eolMode == SC_EOL_CR ? QStringLiteral("CR") : QStringLiteral("LF"))),
        2000);
    return true;
}

bool MainWindow::SaveEditorToFile(ScintillaEditBase *editor, const std::string &pathUtf8) {
    npp::platform::WriteFileOptions options;
    options.atomic = true;
    options.createParentDirs = true;

    auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return false;
    }
    stateIt->second.eolMode = static_cast<int>(editor->send(SCI_GETEOLMODE));
    const std::string bytes = EncodeForWrite(
        GetEditorText(editor),
        stateIt->second.encoding,
        stateIt->second.eolMode);

    const npp::platform::Status writeStatus =
        _fileSystemService.WriteTextFile(pathUtf8, bytes, options);
    if (!writeStatus.ok()) {
        QMessageBox::critical(
            this,
            tr("Save Failed"),
            tr("Unable to save file:\n%1").arg(ToQString(writeStatus.message)));
        return false;
    }

    auto &state = stateIt->second;
    state.filePathUtf8 = pathUtf8;
    state.dirty = false;
    editor->send(SCI_SETSAVEPOINT);
    AutoDetectAndApplyLexer(editor, pathUtf8, GetEditorText(editor), "save");
    UpdateTabTitle(editor);
    UpdateLanguageActionState();
    return true;
}

bool MainWindow::DecodeTextForEditor(
    const std::string &bytes,
    TextEncoding *encoding,
    std::string *utf8Text) const {
    if (!encoding || !utf8Text) {
        return false;
    }

    if (bytes.size() >= 3 &&
        static_cast<unsigned char>(bytes[0]) == 0xEFU &&
        static_cast<unsigned char>(bytes[1]) == 0xBBU &&
        static_cast<unsigned char>(bytes[2]) == 0xBFU) {
        *encoding = TextEncoding::kUtf8Bom;
        *utf8Text = bytes.substr(3);
        return true;
    }

    if (bytes.size() >= 2 &&
        static_cast<unsigned char>(bytes[0]) == 0xFFU &&
        static_cast<unsigned char>(bytes[1]) == 0xFEU) {
        *encoding = TextEncoding::kUtf16Le;
        const QString text = DecodeUtf16(bytes.substr(2), true);
        *utf8Text = text.toUtf8().toStdString();
        return true;
    }

    if (bytes.size() >= 2 &&
        static_cast<unsigned char>(bytes[0]) == 0xFEU &&
        static_cast<unsigned char>(bytes[1]) == 0xFFU) {
        *encoding = TextEncoding::kUtf16Be;
        const QString text = DecodeUtf16(bytes.substr(2), false);
        *utf8Text = text.toUtf8().toStdString();
        return true;
    }

    if (IsValidUtf8(bytes)) {
        *encoding = TextEncoding::kUtf8;
        *utf8Text = bytes;
        return true;
    }

    *encoding = TextEncoding::kLocal8Bit;
    *utf8Text = QString::fromLocal8Bit(bytes.c_str(), static_cast<int>(bytes.size()))
                    .toUtf8()
                    .toStdString();
    return true;
}

std::string MainWindow::EncodeForWrite(
    const std::string &utf8Text,
    TextEncoding encoding,
    int eolMode) const {
    const std::string normalized = NormalizeEol(utf8Text, eolMode);
    const QString text = QString::fromUtf8(normalized.c_str(), static_cast<int>(normalized.size()));

    switch (encoding) {
        case TextEncoding::kUtf8:
            return normalized;
        case TextEncoding::kUtf8Bom:
            return std::string("\xEF\xBB\xBF") + normalized;
        case TextEncoding::kUtf16Le: {
            std::string out;
            out.push_back(static_cast<char>(0xFF));
            out.push_back(static_cast<char>(0xFE));
            const char16_t *units = reinterpret_cast<const char16_t *>(text.utf16());
            for (int i = 0; i < text.size(); ++i) {
                const char16_t value = units[i];
                out.push_back(static_cast<char>(value & 0xFF));
                out.push_back(static_cast<char>((value >> 8) & 0xFF));
            }
            return out;
        }
        case TextEncoding::kUtf16Be: {
            std::string out;
            out.push_back(static_cast<char>(0xFE));
            out.push_back(static_cast<char>(0xFF));
            const char16_t *units = reinterpret_cast<const char16_t *>(text.utf16());
            for (int i = 0; i < text.size(); ++i) {
                const char16_t value = units[i];
                out.push_back(static_cast<char>((value >> 8) & 0xFF));
                out.push_back(static_cast<char>(value & 0xFF));
            }
            return out;
        }
        case TextEncoding::kLocal8Bit:
            return text.toLocal8Bit().toStdString();
    }
    return normalized;
}

std::string MainWindow::NormalizeEol(const std::string &textUtf8, int eolMode) const {
    const std::string eol = eolMode == SC_EOL_CRLF ? "\r\n" : (eolMode == SC_EOL_CR ? "\r" : "\n");
    std::string normalized;
    normalized.reserve(textUtf8.size() + 16);

    for (size_t i = 0; i < textUtf8.size(); ++i) {
        const char ch = textUtf8[i];
        if (ch == '\r') {
            if (i + 1 < textUtf8.size() && textUtf8[i + 1] == '\n') {
                ++i;
            }
            normalized.append(eol);
            continue;
        }
        if (ch == '\n') {
            normalized.append(eol);
            continue;
        }
        normalized.push_back(ch);
    }
    return normalized;
}

void MainWindow::LoadEditorSettings() {
    EnsureConfigRoot();
    const auto exists = _fileSystemService.Exists(SettingsFilePath());
    if (!exists.ok() || !(*exists.value)) {
        return;
    }

    const auto settingsJson = _fileSystemService.ReadTextFile(SettingsFilePath());
    if (!settingsJson.ok()) {
        return;
    }

    const QByteArray settingsBytes(
        settingsJson.value->c_str(),
        static_cast<int>(settingsJson.value->size()));
    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(settingsBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return;
    }

    const QJsonObject obj = doc.object();
    _editorSettings.tabWidth = std::clamp(obj.value(QStringLiteral("tabWidth")).toInt(4), 1, 12);
    _editorSettings.wrapEnabled = obj.value(QStringLiteral("wrapEnabled")).toBool(false);
    _editorSettings.showLineNumbers = obj.value(QStringLiteral("showLineNumbers")).toBool(true);
    _editorSettings.autoCloseHtmlTags = obj.value(QStringLiteral("autoCloseHtmlTags")).toBool(true);
}

void MainWindow::SaveEditorSettings() const {
    const_cast<MainWindow *>(this)->EnsureConfigRoot();
    QJsonObject settingsObject;
    settingsObject.insert(QStringLiteral("tabWidth"), _editorSettings.tabWidth);
    settingsObject.insert(QStringLiteral("wrapEnabled"), _editorSettings.wrapEnabled);
    settingsObject.insert(QStringLiteral("showLineNumbers"), _editorSettings.showLineNumbers);
    settingsObject.insert(QStringLiteral("autoCloseHtmlTags"), _editorSettings.autoCloseHtmlTags);

    const QJsonDocument doc(settingsObject);
    const auto json = doc.toJson(QJsonDocument::Indented);

    npp::platform::WriteFileOptions options;
    options.atomic = true;
    options.createParentDirs = true;
    const_cast<MainWindow *>(this)->_fileSystemService.WriteTextFile(
        SettingsFilePath(),
        std::string(json.constData(), static_cast<size_t>(json.size())),
        options);
}

void MainWindow::ApplyEditorSettingsToAllEditors() {
    if (!_tabs) {
        return;
    }
    for (int index = 0; index < _tabs->count(); ++index) {
        ApplyEditorSettings(EditorAt(index));
    }
}

void MainWindow::ApplyEditorSettings(ScintillaEditBase *editor) {
    if (!editor) {
        return;
    }
    editor->send(SCI_SETTABWIDTH, _editorSettings.tabWidth);
    editor->send(SCI_SETWRAPMODE, _editorSettings.wrapEnabled ? SC_WRAP_WORD : SC_WRAP_NONE);
    editor->send(SCI_SETMARGINWIDTHN, 0, _editorSettings.showLineNumbers ? LineNumberMarginWidth(editor) : 0);
}

void MainWindow::ApplyTheme(ScintillaEditBase *editor) {
    if (!editor) {
        return;
    }

    editor->send(SCI_STYLESETFORE, STYLE_DEFAULT, _themeSettings.foreground);
    editor->send(SCI_STYLESETBACK, STYLE_DEFAULT, _themeSettings.background);
    editor->send(SCI_STYLECLEARALL);

    editor->send(SCI_STYLESETFORE, STYLE_LINENUMBER, _themeSettings.lineNumberForeground);
    editor->send(SCI_STYLESETBACK, STYLE_LINENUMBER, _themeSettings.lineNumberBackground);
    editor->send(SCI_SETCARETFORE, _themeSettings.foreground);
    editor->send(SCI_SETCARETLINEBACK, _themeSettings.caretLineBackground);
    editor->send(SCI_SETSELBACK, 1, _themeSettings.selectionBackground);
    editor->send(SCI_SETSELFORE, 1, _themeSettings.selectionForeground);
}

void MainWindow::ApplyLexerForPath(ScintillaEditBase *editor, const std::string &pathUtf8) {
    AutoDetectAndApplyLexer(editor, pathUtf8, GetEditorText(editor), "path");
}

void MainWindow::AutoDetectAndApplyLexer(
    ScintillaEditBase *editor,
    const std::string &pathUtf8,
    const std::string &contentUtf8,
    const char *trigger) {
    if (!editor) {
        return;
    }

    auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return;
    }
    if (stateIt->second.lexerManualLock) {
        ApplyLexerByName(editor, stateIt->second.lexerName.empty() ? "null" : stateIt->second.lexerName);
        UpdateLanguageActionState();
        return;
    }

    constexpr double kAutoApplyThreshold = 0.60;
    const npp::ui::LanguageDetectionResult detection = npp::ui::DetectLanguage(pathUtf8, contentUtf8);
    const bool shouldAutoApply = detection.confidence >= kAutoApplyThreshold || detection.reason == "extension";
    const std::string chosenLexer = shouldAutoApply ? detection.lexerName : std::string("null");

    stateIt->second.lexerName = chosenLexer;
    stateIt->second.lexerConfidence = detection.confidence;
    stateIt->second.lexerReason = detection.reason;
    ApplyLexerByName(editor, chosenLexer);

    if (statusBar()) {
        statusBar()->showMessage(
            tr("Language %1 (%2%% via %3, %4)")
                .arg(ToQString(chosenLexer))
                .arg(ToQString(PercentString(detection.confidence)))
                .arg(ToQString(detection.reason))
                .arg(QString::fromLatin1(trigger)),
            2500);
    }
    UpdateLanguageActionState();
}

void MainWindow::ApplyLexerByName(ScintillaEditBase *editor, const std::string &lexerName) {
    if (!editor) {
        return;
    }

#if defined(NPP_HAVE_LEXILLA) && NPP_HAVE_LEXILLA
    auto *lexer = CreateLexer(lexerName.c_str());
    editor->send(SCI_SETILEXER, 0, reinterpret_cast<sptr_t>(lexer));
#else
    static_cast<void>(lexerName);
    editor->send(SCI_SETILEXER, 0, 0);
#endif

    if (lexerName == "cpp") {
        editor->sends(
            SCI_SETKEYWORDS,
            0,
            "alignas alignof and and_eq asm auto bitand bitor bool break case catch char class "
            "const constexpr const_cast continue decltype default delete do double dynamic_cast "
            "else enum explicit export extern false float for friend goto if inline int long "
            "mutable namespace new noexcept not not_eq nullptr operator or or_eq private "
            "protected public register reinterpret_cast return short signed sizeof static "
            "static_assert static_cast struct switch template this thread_local throw true try "
            "typedef typeid typename union unsigned using virtual void volatile wchar_t while xor xor_eq");
    } else if (lexerName == "python") {
        editor->sends(
            SCI_SETKEYWORDS,
            0,
            "False None True and as assert async await break class continue def del elif else "
            "except finally for from global if import in is lambda nonlocal not or pass raise "
            "return try while with yield");
    }

    ApplyTheme(editor);
    ApplyLexerStyles(editor, lexerName);
    editor->send(SCI_COLOURISE, 0, -1);
}

void MainWindow::ApplyLexerStyles(ScintillaEditBase *editor, const std::string &lexerName) {
    if (!editor) {
        return;
    }

    if (lexerName == "cpp" || lexerName == "json") {
        editor->send(SCI_STYLESETFORE, SCE_C_COMMENT, _themeSettings.comment);
        editor->send(SCI_STYLESETFORE, SCE_C_COMMENTLINE, _themeSettings.comment);
        editor->send(SCI_STYLESETFORE, SCE_C_COMMENTDOC, _themeSettings.comment);
        editor->send(SCI_STYLESETFORE, SCE_C_NUMBER, _themeSettings.number);
        editor->send(SCI_STYLESETFORE, SCE_C_WORD, _themeSettings.keyword);
        editor->send(SCI_STYLESETBOLD, SCE_C_WORD, 1);
        editor->send(SCI_STYLESETFORE, SCE_C_STRING, _themeSettings.stringColor);
        editor->send(SCI_STYLESETFORE, SCE_C_CHARACTER, _themeSettings.stringColor);
        editor->send(SCI_STYLESETFORE, SCE_C_OPERATOR, _themeSettings.operatorColor);
        editor->send(SCI_STYLESETFORE, SCE_C_PREPROCESSOR, _themeSettings.keyword);
    } else if (lexerName == "python") {
        editor->send(SCI_STYLESETFORE, SCE_P_COMMENTLINE, _themeSettings.comment);
        editor->send(SCI_STYLESETFORE, SCE_P_NUMBER, _themeSettings.number);
        editor->send(SCI_STYLESETFORE, SCE_P_WORD, _themeSettings.keyword);
        editor->send(SCI_STYLESETBOLD, SCE_P_WORD, 1);
        editor->send(SCI_STYLESETFORE, SCE_P_STRING, _themeSettings.stringColor);
        editor->send(SCI_STYLESETFORE, SCE_P_CHARACTER, _themeSettings.stringColor);
        editor->send(SCI_STYLESETFORE, SCE_P_OPERATOR, _themeSettings.operatorColor);
        editor->send(SCI_STYLESETFORE, SCE_P_DEFNAME, _themeSettings.keyword);
        editor->send(SCI_STYLESETFORE, SCE_P_CLASSNAME, _themeSettings.keyword);
    } else if (lexerName == "bash") {
        editor->send(SCI_STYLESETFORE, SCE_SH_COMMENTLINE, _themeSettings.comment);
        editor->send(SCI_STYLESETFORE, SCE_SH_NUMBER, _themeSettings.number);
        editor->send(SCI_STYLESETFORE, SCE_SH_WORD, _themeSettings.keyword);
        editor->send(SCI_STYLESETBOLD, SCE_SH_WORD, 1);
        editor->send(SCI_STYLESETFORE, SCE_SH_STRING, _themeSettings.stringColor);
        editor->send(SCI_STYLESETFORE, SCE_SH_OPERATOR, _themeSettings.operatorColor);
    }
}

void MainWindow::EnsureConfigRoot() {
    const std::string root = ConfigRootPath();
    if (root.empty()) {
        return;
    }
    _fileSystemService.CreateDirectories(root);
}

void MainWindow::EnsureThemeFile() {
    EnsureConfigRoot();
    const auto exists = _fileSystemService.Exists(ThemeFilePath());
    if (exists.ok() && *exists.value) {
        return;
    }

    QJsonObject themeJson;
    themeJson.insert(QStringLiteral("background"), QStringLiteral("#FFFFFF"));
    themeJson.insert(QStringLiteral("foreground"), QStringLiteral("#1A1A1A"));
    themeJson.insert(QStringLiteral("lineNumberBackground"), QStringLiteral("#F2F2F2"));
    themeJson.insert(QStringLiteral("lineNumberForeground"), QStringLiteral("#616161"));
    themeJson.insert(QStringLiteral("caretLineBackground"), QStringLiteral("#F7FAFF"));
    themeJson.insert(QStringLiteral("selectionBackground"), QStringLiteral("#CFE8FF"));
    themeJson.insert(QStringLiteral("selectionForeground"), QStringLiteral("#000000"));
    themeJson.insert(QStringLiteral("comment"), QStringLiteral("#6A9955"));
    themeJson.insert(QStringLiteral("keyword"), QStringLiteral("#005FB8"));
    themeJson.insert(QStringLiteral("number"), QStringLiteral("#9A3E9D"));
    themeJson.insert(QStringLiteral("stringColor"), QStringLiteral("#B34100"));
    themeJson.insert(QStringLiteral("operatorColor"), QStringLiteral("#1A1A1A"));

    const QJsonDocument doc(themeJson);
    const QByteArray json = doc.toJson(QJsonDocument::Indented);

    npp::platform::WriteFileOptions options;
    options.atomic = true;
    options.createParentDirs = true;
    _fileSystemService.WriteTextFile(
        ThemeFilePath(),
        std::string(json.constData(), static_cast<size_t>(json.size())),
        options);
}

void MainWindow::LoadTheme() {
    EnsureThemeFile();
    const auto themeJson = _fileSystemService.ReadTextFile(ThemeFilePath());
    if (!themeJson.ok()) {
        return;
    }

    QJsonParseError parseError{};
    const QByteArray bytes(themeJson.value->c_str(), static_cast<int>(themeJson.value->size()));
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return;
    }

    const QJsonObject obj = doc.object();
    _themeSettings.background = ParseThemeColor(obj, "background", _themeSettings.background);
    _themeSettings.foreground = ParseThemeColor(obj, "foreground", _themeSettings.foreground);
    _themeSettings.lineNumberBackground = ParseThemeColor(
        obj,
        "lineNumberBackground",
        _themeSettings.lineNumberBackground);
    _themeSettings.lineNumberForeground = ParseThemeColor(
        obj,
        "lineNumberForeground",
        _themeSettings.lineNumberForeground);
    _themeSettings.caretLineBackground = ParseThemeColor(
        obj,
        "caretLineBackground",
        _themeSettings.caretLineBackground);
    _themeSettings.selectionBackground = ParseThemeColor(
        obj,
        "selectionBackground",
        _themeSettings.selectionBackground);
    _themeSettings.selectionForeground = ParseThemeColor(
        obj,
        "selectionForeground",
        _themeSettings.selectionForeground);
    _themeSettings.comment = ParseThemeColor(obj, "comment", _themeSettings.comment);
    _themeSettings.keyword = ParseThemeColor(obj, "keyword", _themeSettings.keyword);
    _themeSettings.number = ParseThemeColor(obj, "number", _themeSettings.number);
    _themeSettings.stringColor = ParseThemeColor(obj, "stringColor", _themeSettings.stringColor);
    _themeSettings.operatorColor = ParseThemeColor(obj, "operatorColor", _themeSettings.operatorColor);
}

void MainWindow::EnsureShortcutConfigFile() {
    EnsureConfigRoot();
    const auto exists = _fileSystemService.Exists(ShortcutFilePath());
    if (exists.ok() && *exists.value) {
        return;
    }

    QJsonObject defaults;
    for (const auto &[actionId, shortcut] : DefaultShortcutMap()) {
        defaults.insert(QString::fromStdString(actionId), shortcut.toString(QKeySequence::PortableText));
    }

    const QJsonDocument doc(defaults);
    const QByteArray json = doc.toJson(QJsonDocument::Indented);

    npp::platform::WriteFileOptions options;
    options.atomic = true;
    options.createParentDirs = true;
    _fileSystemService.WriteTextFile(
        ShortcutFilePath(),
        std::string(json.constData(), static_cast<size_t>(json.size())),
        options);
}

void MainWindow::LoadShortcutOverrides() {
    _shortcutOverrides = QJsonObject{};

    const auto exists = _fileSystemService.Exists(ShortcutFilePath());
    if (!exists.ok() || !(*exists.value)) {
        return;
    }

    const auto jsonFile = _fileSystemService.ReadTextFile(ShortcutFilePath());
    if (!jsonFile.ok()) {
        return;
    }

    const QByteArray bytes(jsonFile.value->c_str(), static_cast<int>(jsonFile.value->size()));
    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return;
    }

    _shortcutOverrides = doc.object();
}

void MainWindow::ApplyShortcuts() {
    const auto &defaults = DefaultShortcutMap();
    for (const auto &[id, action] : _actionsById) {
        if (!action) {
            continue;
        }

        QKeySequence shortcut;
        const auto defaultIt = defaults.find(id);
        if (defaultIt != defaults.end()) {
            shortcut = defaultIt->second;
        }

        if (_shortcutOverrides.contains(QString::fromStdString(id))) {
            const QJsonValue overrideValue = _shortcutOverrides.value(QString::fromStdString(id));
            if (overrideValue.isString()) {
                const QKeySequence overrideShortcut(overrideValue.toString());
                if (!overrideShortcut.isEmpty()) {
                    shortcut = overrideShortcut;
                }
            }
        }

        action->setShortcut(shortcut);
        action->setShortcutContext(Qt::WindowShortcut);
    }
}

void MainWindow::ReloadShortcuts() {
    LoadShortcutOverrides();
    ApplyShortcuts();
    statusBar()->showMessage(tr("Shortcuts reloaded"), 1500);
}

void MainWindow::OpenShortcutConfigFile() {
    EnsureShortcutConfigFile();
    const npp::platform::Status status = _processService.OpenPath(ShortcutFilePath());
    if (!status.ok()) {
        QMessageBox::warning(
            this,
            tr("Shortcuts"),
            tr("Unable to open shortcut config file:\n%1").arg(ToQString(status.message)));
    }
}

std::string MainWindow::ConfigRootPath() const {
    const auto configPath = _pathService.GetAppPath(npp::platform::PathScope::kConfig, kAppName);
    if (!configPath.ok()) {
        return std::string{};
    }
    return *configPath.value;
}

std::string MainWindow::SettingsFilePath() const {
    return ConfigRootPath() + "/editor-settings.json";
}

std::string MainWindow::ShortcutFilePath() const {
    return ConfigRootPath() + "/shortcuts-linux.json";
}

std::string MainWindow::ThemeFilePath() const {
    return ConfigRootPath() + "/theme-linux.json";
}

bool MainWindow::RestoreSession() {
    EnsureConfigRoot();
    const auto exists = _fileSystemService.Exists(SessionFilePath());
    if (!exists.ok() || !(*exists.value)) {
        return false;
    }

    const auto sessionJson = _fileSystemService.ReadTextFile(SessionFilePath());
    if (!sessionJson.ok()) {
        return false;
    }

    QJsonParseError parseError{};
    const QByteArray jsonBytes(sessionJson.value->c_str(), static_cast<int>(sessionJson.value->size()));
    const QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    const QJsonObject root = doc.object();
    const QJsonArray files = root.value(QStringLiteral("openFiles")).toArray();
    if (files.isEmpty()) {
        return false;
    }

    int opened = 0;
    for (const QJsonValue &fileValue : files) {
        if (!fileValue.isString()) {
            continue;
        }

        const std::string pathUtf8 = ToUtf8(fileValue.toString());
        if (pathUtf8.empty()) {
            continue;
        }

        const auto fileExists = _fileSystemService.Exists(pathUtf8);
        if (!fileExists.ok() || !(*fileExists.value)) {
            continue;
        }

        NewTab();
        ScintillaEditBase *editor = CurrentEditor();
        if (editor && LoadFileIntoEditor(editor, pathUtf8)) {
            ++opened;
            continue;
        }

        const int currentIndex = _tabs->currentIndex();
        if (currentIndex >= 0 && editor) {
            _tabs->removeTab(currentIndex);
            _editorStates.erase(editor);
            editor->deleteLater();
        }
    }

    if (opened <= 0) {
        return false;
    }

    const int activeIndex = root.value(QStringLiteral("activeIndex")).toInt(0);
    const int boundedIndex = std::clamp(activeIndex, 0, _tabs->count() - 1);
    _tabs->setCurrentIndex(boundedIndex);
    return true;
}

void MainWindow::SaveSession() const {
    const_cast<MainWindow *>(this)->EnsureConfigRoot();

    QJsonArray files;
    for (int index = 0; index < _tabs->count(); ++index) {
        ScintillaEditBase *editor = EditorAt(index);
        if (!editor) {
            continue;
        }
        const auto stateIt = _editorStates.find(editor);
        if (stateIt == _editorStates.end() || stateIt->second.filePathUtf8.empty()) {
            continue;
        }
        files.append(ToQString(stateIt->second.filePathUtf8));
    }

    QJsonObject root;
    root.insert(QStringLiteral("openFiles"), files);
    root.insert(QStringLiteral("activeIndex"), _tabs->currentIndex());
    const QJsonDocument doc(root);
    const QByteArray json = doc.toJson(QJsonDocument::Indented);

    npp::platform::WriteFileOptions options;
    options.atomic = true;
    options.createParentDirs = true;
    const_cast<MainWindow *>(this)->_fileSystemService.WriteTextFile(
        SessionFilePath(),
        std::string(json.constData(), static_cast<size_t>(json.size())),
        options);
}

std::string MainWindow::SessionFilePath() const {
    return ConfigRootPath() + "/session.json";
}

QString MainWindow::EncodingLabel(TextEncoding encoding) {
    switch (encoding) {
        case TextEncoding::kUtf8:
            return QStringLiteral("UTF-8");
        case TextEncoding::kUtf8Bom:
            return QStringLiteral("UTF-8 with BOM");
        case TextEncoding::kUtf16Le:
            return QStringLiteral("UTF-16 LE");
        case TextEncoding::kUtf16Be:
            return QStringLiteral("UTF-16 BE");
        case TextEncoding::kLocal8Bit:
            return QStringLiteral("Local 8-bit");
    }
    return QStringLiteral("UTF-8");
}

std::string MainWindow::ToUtf8(const QString &value) {
    return value.toUtf8().toStdString();
}

QString MainWindow::ToQString(const std::string &value) {
    return QString::fromUtf8(value.c_str(), static_cast<int>(value.size()));
}
