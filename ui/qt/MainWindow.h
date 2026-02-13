#pragma once

#include <map>
#include <string>
#include <vector>

#include <QJsonObject>
#include <QMainWindow>
#include <QString>

#include "LinuxDiagnosticsService.h"
#include "LinuxFileSystemService.h"
#include "LinuxPathService.h"
#include "LinuxProcessService.h"

class QAction;
class QLabel;
class QTabWidget;
class ScintillaEditBase;

class MainWindow final : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow() override = default;

private:
	struct EditorState {
		std::string filePathUtf8;
		bool dirty = false;
	};

	struct EditorSettings {
		int tabWidth = 4;
		bool wrapEnabled = false;
		bool showLineNumbers = true;
	};

	void BuildUi();
	void BuildMenus();
	void BuildStatusBar();
	void BuildActions();
	void BuildTabs();

	ScintillaEditBase *CreateEditor();
	ScintillaEditBase *CurrentEditor() const;
	ScintillaEditBase *EditorAt(int index) const;

	void NewTab();
	void OpenFile();
	bool SaveCurrentFile();
	bool SaveCurrentFileAs();
	bool CloseTabAt(int index);
	void CloseTab();
	void UpdateTabTitle(ScintillaEditBase *editor);
	void UpdateWindowTitle();
	void UpdateCursorStatus();

	void OnFind();
	void OnReplace();
	void OnGoToLine();
	void OnPreferences();

	bool FindNextInEditor(ScintillaEditBase *editor, const std::string &needleUtf8, bool matchCase);
	int ReplaceAllInEditor(
		ScintillaEditBase *editor,
		const std::string &needleUtf8,
		const std::string &replacementUtf8,
		bool matchCase);

	std::string GetEditorText(ScintillaEditBase *editor) const;
	std::string GetSelectedText(ScintillaEditBase *editor) const;
	void SetEditorText(ScintillaEditBase *editor, const std::string &textUtf8);

	bool LoadFileIntoEditor(ScintillaEditBase *editor, const std::string &pathUtf8);
	bool SaveEditorToFile(ScintillaEditBase *editor, const std::string &pathUtf8);

	void LoadEditorSettings();
	void SaveEditorSettings() const;
	void ApplyEditorSettingsToAllEditors();
	void ApplyEditorSettings(ScintillaEditBase *editor);

	void EnsureConfigRoot();
	void EnsureShortcutConfigFile();
	void LoadShortcutOverrides();
	void ApplyShortcuts();
	void ReloadShortcuts();
	void OpenShortcutConfigFile();

	std::string ConfigRootPath() const;
	std::string SettingsFilePath() const;
	std::string ShortcutFilePath() const;
	static std::string ToUtf8(const QString &value);
	static QString ToQString(const std::string &value);

private:
	static constexpr const char *kAppName = "notepad-plus-plus-linux";

	QTabWidget *_tabs = nullptr;
	QLabel *_cursorStatusLabel = nullptr;

	std::map<ScintillaEditBase *, EditorState> _editorStates;
	std::map<std::string, QAction *> _actionsById;
	EditorSettings _editorSettings;
	QJsonObject _shortcutOverrides;
	std::string _lastFindUtf8;

	npp::platform::LinuxPathService _pathService;
	npp::platform::LinuxFileSystemService _fileSystemService;
	npp::platform::LinuxProcessService _processService;
	npp::platform::LinuxDiagnosticsService _diagnosticsService;
};
