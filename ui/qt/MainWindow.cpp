#include "MainWindow.h"

#include <array>
#include <filesystem>

#include <QAction>
#include <QFileDialog>
#include <QJsonDocument>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTabWidget>

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
    QMessageBox::information(this, tr("Find"), tr("Find dialog will be enabled in Phase 4 step 3."));
}

void MainWindow::OnReplace() {
    QMessageBox::information(this, tr("Replace"), tr("Replace dialog will be enabled in Phase 4 step 3."));
}

void MainWindow::OnGoToLine() {
    QMessageBox::information(this, tr("Go To Line"), tr("Go To Line dialog will be enabled in Phase 4 step 3."));
}

void MainWindow::OnPreferences() {
    QMessageBox::information(this, tr("Preferences"), tr("Preferences dialog will be enabled in Phase 4 step 3."));
}

bool MainWindow::FindNextInEditor(ScintillaEditBase *, const std::string &, bool) {
    return false;
}

int MainWindow::ReplaceAllInEditor(
    ScintillaEditBase *,
    const std::string &,
    const std::string &,
    bool) {
    return 0;
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

void MainWindow::LoadEditorSettings() {}

void MainWindow::SaveEditorSettings() const {}

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

void MainWindow::EnsureConfigRoot() {}

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
