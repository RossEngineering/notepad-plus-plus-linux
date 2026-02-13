#include "MainWindow.h"

#include <algorithm>
#include <filesystem>

#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QInputDialog>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>

#include "Scintilla.h"
#include "ScintillaEditBase.h"

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

}  // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), _diagnosticsService(_pathService, _fileSystemService) {
    BuildUi();
    LoadEditorSettings();
    NewTab();
    UpdateWindowTitle();
    UpdateCursorStatus();
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
    fileMenu->addSeparator();
    fileMenu->addAction(_actionsById.at("file.save"));
    fileMenu->addAction(_actionsById.at("file.saveAs"));
    fileMenu->addSeparator();
    fileMenu->addAction(_actionsById.at("tab.close"));
    fileMenu->addSeparator();
    fileMenu->addAction(_actionsById.at("file.quit"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(_actionsById.at("edit.find"));
    editMenu->addAction(_actionsById.at("edit.replace"));
    editMenu->addAction(_actionsById.at("edit.gotoLine"));
    editMenu->addSeparator();
    editMenu->addAction(_actionsById.at("edit.preferences"));

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(_actionsById.at("tools.shortcuts.open"));
    toolsMenu->addAction(_actionsById.at("tools.shortcuts.reload"));
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
    registerAction("file.save", tr("Save"), [this]() { SaveCurrentFile(); });
    registerAction("file.saveAs", tr("Save As..."), [this]() { SaveCurrentFileAs(); });
    registerAction("tab.close", tr("Close Tab"), [this]() { CloseTab(); });
    registerAction("file.quit", tr("Quit"), [this]() { close(); });

    registerAction("edit.find", tr("Find..."), [this]() { OnFind(); });
    registerAction("edit.replace", tr("Replace..."), [this]() { OnReplace(); });
    registerAction("edit.gotoLine", tr("Go To Line..."), [this]() { OnGoToLine(); });
    registerAction("edit.preferences", tr("Preferences..."), [this]() { OnPreferences(); });

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
    _editorStates.insert_or_assign(editor, state);

    const int index = _tabs->addTab(editor, tr("Untitled"));
    _tabs->setCurrentIndex(index);
    editor->setFocus();
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

    const QString target = QFileDialog::getSaveFileName(this, tr("Save File As"));
    if (target.isEmpty()) {
        return false;
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

    if (_tabs->count() == 0) {
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

    form->addRow(tr("Tab width:"), tabWidthSpin);
    form->addRow(QString(), wrapCheck);
    form->addRow(QString(), lineNumbersCheck);
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
    ApplyEditorSettingsToAllEditors();
    SaveEditorSettings();
    statusBar()->showMessage(tr("Preferences updated"), 1500);
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

bool MainWindow::LoadFileIntoEditor(ScintillaEditBase *editor, const std::string &pathUtf8) {
    const auto read = _fileSystemService.ReadTextFile(pathUtf8);
    if (!read.ok()) {
        QMessageBox::critical(
            this,
            tr("Open Failed"),
            tr("Unable to open file:\n%1").arg(ToQString(read.status.message)));
        return false;
    }

    SetEditorText(editor, *read.value);
    editor->send(SCI_SETSAVEPOINT);

    auto &state = _editorStates[editor];
    state.filePathUtf8 = pathUtf8;
    state.dirty = false;
    UpdateTabTitle(editor);
    return true;
}

bool MainWindow::SaveEditorToFile(ScintillaEditBase *editor, const std::string &pathUtf8) {
    npp::platform::WriteFileOptions options;
    options.atomic = true;
    options.createParentDirs = true;

    const npp::platform::Status writeStatus =
        _fileSystemService.WriteTextFile(pathUtf8, GetEditorText(editor), options);
    if (!writeStatus.ok()) {
        QMessageBox::critical(
            this,
            tr("Save Failed"),
            tr("Unable to save file:\n%1").arg(ToQString(writeStatus.message)));
        return false;
    }

    auto &state = _editorStates[editor];
    state.filePathUtf8 = pathUtf8;
    state.dirty = false;
    editor->send(SCI_SETSAVEPOINT);
    UpdateTabTitle(editor);
    return true;
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
}

void MainWindow::SaveEditorSettings() const {
    const_cast<MainWindow *>(this)->EnsureConfigRoot();
    QJsonObject settingsObject;
    settingsObject.insert(QStringLiteral("tabWidth"), _editorSettings.tabWidth);
    settingsObject.insert(QStringLiteral("wrapEnabled"), _editorSettings.wrapEnabled);
    settingsObject.insert(QStringLiteral("showLineNumbers"), _editorSettings.showLineNumbers);

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

void MainWindow::EnsureConfigRoot() {
    const std::string root = ConfigRootPath();
    if (root.empty()) {
        return;
    }
    _fileSystemService.CreateDirectories(root);
}

void MainWindow::EnsureShortcutConfigFile() {}

void MainWindow::LoadShortcutOverrides() {}

void MainWindow::ApplyShortcuts() {}

void MainWindow::ReloadShortcuts() {
    QMessageBox::information(this, tr("Shortcuts"), tr("Shortcut override loading will be enabled in Phase 4 step 4."));
}

void MainWindow::OpenShortcutConfigFile() {
    QMessageBox::information(this, tr("Shortcuts"), tr("Shortcut override file integration will be enabled in Phase 4 step 4."));
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

std::string MainWindow::ToUtf8(const QString &value) {
    return value.toUtf8().toStdString();
}

QString MainWindow::ToQString(const std::string &value) {
    return QString::fromUtf8(value.c_str(), static_cast<int>(value.size()));
}
