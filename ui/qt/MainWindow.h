#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <QJsonObject>
#include <QMainWindow>
#include <QString>

#include "LinuxDiagnosticsService.h"
#include "LinuxExtensionService.h"
#include "LinuxFileSystemService.h"
#include "LinuxLspClientService.h"
#include "LinuxPathService.h"
#include "LinuxProcessService.h"

class QAction;
class QCloseEvent;
class QEvent;
class QLabel;
class QTabWidget;
class QTimer;
class ScintillaEditBase;

class MainWindow final : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow() override = default;

protected:
	void closeEvent(QCloseEvent *event) override;
	bool eventFilter(QObject *watched, QEvent *event) override;

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
		bool lexerManualLock = false;
		double lexerConfidence = 0.0;
		std::string lexerReason;
	};

	struct EditorSettings {
		int tabWidth = 4;
		bool wrapEnabled = false;
		bool showLineNumbers = true;
		bool autoDetectLanguage = true;
		bool autoCloseHtmlTags = true;
		bool autoCloseDelimiters = true;
		bool autoSaveOnFocusLost = false;
		bool autoSaveOnInterval = false;
		bool autoSaveBeforeRun = false;
		int autoSaveIntervalSeconds = 30;
		bool extensionGuardrailsEnabled = true;
		int extensionStartupBudgetMs = 1200;
		int extensionPerExtensionBudgetMs = 250;
		int crashRecoveryAutosaveSeconds = 15;
		std::string skinId = "builtin.light";
	};

	struct ThemeSettings {
		int windowBackground = 0xF3F3F3;
		int windowForeground = 0x1A1A1A;
		int menuBackground = 0xFFFFFF;
		int menuForeground = 0x1A1A1A;
		int statusBackground = 0xEFEFEF;
		int statusForeground = 0x1A1A1A;
		int accent = 0x005FB8;
		int dialogBackground = 0xFFFFFF;
		int dialogForeground = 0x1A1A1A;
		int dialogButtonBackground = 0xE9EEF6;
		int dialogButtonForeground = 0x1A1A1A;
		int dialogBorder = 0xB5C3D6;
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
	void OnFormatDocument();
	void OnPreferences();
	void OnRunCommand();
	void OnCommandPalette();
	void OnMultiCursorAddCaretAbove();
	void OnMultiCursorAddCaretBelow();
	void OnMultiCursorAddNextMatch();
	void OnMultiCursorSelectAllMatches();
	void OnOpenHelpDocs();
	void OnOpenHelpWiki();
	void OnReportBug();
	void OnRequestFeature();
	void OnAboutDialog();
	void OnInstallExtensionFromDirectory();
	void OnManageExtensions();
	void OnAutoDetectLanguage();
	void OnToggleLexerLock();
	void OnLspStartSession();
	void OnLspStopSession();
	void OnLspShowHover();
	void OnLspGoToDefinition();
	void OnLspShowDiagnostics();
	void OnSetSkin(const std::string &skinId);
	void SetCurrentEditorManualLexer(const std::string &lexerName);
	void UpdateLanguageActionState();
	void UpdateSkinActionState();

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
	void MaybeAutoCloseHtmlTag(ScintillaEditBase *editor, int ch);
	void MaybeAutoCloseDelimiterPair(ScintillaEditBase *editor, int ch);

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
	void ApplyChromeTheme();
	void ApplyLexerForPath(ScintillaEditBase *editor, const std::string &pathUtf8);
	void ApplyLexerByName(ScintillaEditBase *editor, const std::string &lexerName);
	void AutoDetectAndApplyLexer(
		ScintillaEditBase *editor,
		const std::string &pathUtf8,
		const std::string &contentUtf8,
		const char *trigger,
		bool force = false);
	void ApplyLexerStyles(ScintillaEditBase *editor, const std::string &lexerName);

	void EnsureConfigRoot();
	void EnsureBuiltInSkins();
	void EnsureThemeFile();
	void LoadTheme();
	void EnsureShortcutConfigFile();
	void LoadShortcutOverrides();
	void ApplyShortcuts();
	void ReloadShortcuts();
	void OpenShortcutConfigFile();
	void OpenExternalLink(const QString &url, const QString &label);
	void InitializeLspServers();
	void StopLspSessionForEditor(ScintillaEditBase *editor);
	void InitializeExtensionsWithGuardrails();
	void StartCrashRecoveryTimer();
	void StartAutoSaveTimer();
	bool AutoSaveEditorIfNeeded(ScintillaEditBase *editor, std::string *savedPathUtf8 = nullptr);
	int AutoSaveDirtyEditors(std::vector<std::string> *savedPathsUtf8 = nullptr);
	void SaveCrashRecoveryJournal() const;
	bool RestoreCrashRecoveryJournal();
	void ClearCrashRecoveryJournal() const;
	bool RestoreSession();
	void SaveSession() const;

	std::string ConfigRootPath() const;
	std::string SkinDirectoryPath() const;
	std::string SkinFilePathForId(const std::string &skinId) const;
	std::string SettingsFilePath() const;
	std::string ShortcutFilePath() const;
	std::string ThemeFilePath() const;
	std::string SessionFilePath() const;
	std::string CrashRecoveryJournalPath() const;
	static QString EncodingLabel(TextEncoding encoding);
	static std::string ToUtf8(const QString &value);
	static QString ToQString(const std::string &value);

private:
	static constexpr const char *kAppName = "notepad-plus-plus-linux";

	QTabWidget *_tabs = nullptr;
	QLabel *_cursorStatusLabel = nullptr;

	std::map<ScintillaEditBase *, EditorState> _editorStates;
	std::map<ScintillaEditBase *, std::uint64_t> _lspSessionByEditor;
	std::map<std::string, QAction *> _actionsById;
	EditorSettings _editorSettings;
	ThemeSettings _themeSettings;
	QJsonObject _shortcutOverrides;
	std::string _lastFindUtf8;
	std::string _lastRunCommandUtf8;
	std::string _lastRunWorkingDirUtf8;
	bool _closingApplication = false;
	bool _suppressAutoCloseHandler = false;
	QTimer *_crashRecoveryTimer = nullptr;
	QTimer *_autoSaveTimer = nullptr;

	npp::platform::LinuxPathService _pathService;
	npp::platform::LinuxFileSystemService _fileSystemService;
	npp::platform::LinuxProcessService _processService;
	npp::platform::LinuxDiagnosticsService _diagnosticsService;
	npp::platform::LinuxExtensionService _extensionService;
	npp::platform::LinuxLspClientService _lspService;
};
