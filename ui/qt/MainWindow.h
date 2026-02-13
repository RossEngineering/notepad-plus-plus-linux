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
class QCloseEvent;
class QLabel;
class QTabWidget;
class ScintillaEditBase;

class MainWindow final : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow() override = default;

protected:
	void closeEvent(QCloseEvent *event) override;

private:
	enum class TextEncoding {
		kUtf8 = 0,
		kUtf8Bom,
		kUtf16Le,
		kUtf16Be,
		kLocal8Bit,
	};

	struct EditorState {
		std::string filePathUtf8;
		bool dirty = false;
		TextEncoding encoding = TextEncoding::kUtf8;
		int eolMode = 2;
		std::string lexerName;
	};

	struct EditorSettings {
		int tabWidth = 4;
		bool wrapEnabled = false;
		bool showLineNumbers = true;
	};

	struct ThemeSettings {
		int background = 0xFFFFFF;
		int foreground = 0x1A1A1A;
		int lineNumberBackground = 0xF2F2F2;
		int lineNumberForeground = 0x616161;
		int caretLineBackground = 0xF7FAFF;
		int selectionBackground = 0xCFE8FF;
		int selectionForeground = 0x000000;
		int comment = 0x6A9955;
		int keyword = 0x005FB8;
		int number = 0x9A3E9D;
		int stringColor = 0xB34100;
		int operatorColor = 0x1A1A1A;
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
	void ReloadCurrentFile();
	bool SaveCurrentFile();
	bool SaveCurrentFileAs();
	bool CloseTabAt(int index);
	void CloseTab();
	void UpdateTabTitle(ScintillaEditBase *editor);
	void UpdateWindowTitle();
	void UpdateCursorStatus();

	void OnFind();
	void OnReplace();
	void OnFindInFiles();
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
	void SetEditorEolMode(ScintillaEditBase *editor, int eolMode);
	int DetectDominantEolMode(const std::string &textUtf8) const;

	bool LoadFileIntoEditor(ScintillaEditBase *editor, const std::string &pathUtf8);
	bool SaveEditorToFile(ScintillaEditBase *editor, const std::string &pathUtf8);
	bool DecodeTextForEditor(const std::string &bytes, TextEncoding *encoding, std::string *utf8Text) const;
	std::string EncodeForWrite(
		const std::string &utf8Text,
		TextEncoding encoding,
		int eolMode) const;
	std::string NormalizeEol(const std::string &textUtf8, int eolMode) const;

	void LoadEditorSettings();
	void SaveEditorSettings() const;
	void ApplyEditorSettingsToAllEditors();
	void ApplyEditorSettings(ScintillaEditBase *editor);
	void ApplyTheme(ScintillaEditBase *editor);
	void ApplyLexerForPath(ScintillaEditBase *editor, const std::string &pathUtf8);
	void ApplyLexerStyles(ScintillaEditBase *editor, const std::string &lexerName);

	void EnsureConfigRoot();
	void EnsureThemeFile();
	void LoadTheme();
	void EnsureShortcutConfigFile();
	void LoadShortcutOverrides();
	void ApplyShortcuts();
	void ReloadShortcuts();
	void OpenShortcutConfigFile();
	bool RestoreSession();
	void SaveSession() const;

	std::string ConfigRootPath() const;
	std::string SettingsFilePath() const;
	std::string ShortcutFilePath() const;
	std::string ThemeFilePath() const;
	std::string SessionFilePath() const;
	static QString EncodingLabel(TextEncoding encoding);
	static std::string ToUtf8(const QString &value);
	static QString ToQString(const std::string &value);

private:
	static constexpr const char *kAppName = "notepad-plus-plus-linux";

	QTabWidget *_tabs = nullptr;
	QLabel *_cursorStatusLabel = nullptr;

	std::map<ScintillaEditBase *, EditorState> _editorStates;
	std::map<std::string, QAction *> _actionsById;
	EditorSettings _editorSettings;
	ThemeSettings _themeSettings;
	QJsonObject _shortcutOverrides;
	std::string _lastFindUtf8;
	bool _closingApplication = false;

	npp::platform::LinuxPathService _pathService;
	npp::platform::LinuxFileSystemService _fileSystemService;
	npp::platform::LinuxProcessService _processService;
	npp::platform::LinuxDiagnosticsService _diagnosticsService;
};
