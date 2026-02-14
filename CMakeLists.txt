cmake_minimum_required(VERSION 3.20)

project(notepad_plus_plus_linux LANGUAGES CXX)

option(NPP_BUILD_SCINTILLA "Build Scintilla core static library" ON)
option(NPP_BUILD_LEXILLA "Build Lexilla static library" ON)
option(NPP_BUILD_PLATFORM_LINUX "Build Linux platform service implementations" ON)
option(NPP_BUILD_QT_SHELL "Build Qt-based Linux shell prototype" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(GNUInstallDirs)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(NPP_APP_VERSION "dev")
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}" describe --tags --always --dirty
        OUTPUT_VARIABLE NPP_APP_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(NOT NPP_APP_VERSION)
        set(NPP_APP_VERSION "dev")
    endif()
endif()

add_library(npp_language_detection STATIC
    ui/qt/LanguageDetection.cpp
)
add_library(npp::language_detection ALIAS npp_language_detection)
target_include_directories(npp_language_detection
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/ui/qt"
)

if(NPP_BUILD_SCINTILLA)
    set(SCINTILLA_CORE_SOURCES
        scintilla/src/AutoComplete.cxx
        scintilla/src/CallTip.cxx
        scintilla/src/CaseConvert.cxx
        scintilla/src/CaseFolder.cxx
        scintilla/src/CellBuffer.cxx
        scintilla/src/ChangeHistory.cxx
        scintilla/src/CharacterCategoryMap.cxx
        scintilla/src/CharacterType.cxx
        scintilla/src/CharClassify.cxx
        scintilla/src/ContractionState.cxx
        scintilla/src/DBCS.cxx
        scintilla/src/Decoration.cxx
        scintilla/src/Document.cxx
        scintilla/src/EditModel.cxx
        scintilla/src/Editor.cxx
        scintilla/src/EditView.cxx
        scintilla/src/Geometry.cxx
        scintilla/src/Indicator.cxx
        scintilla/src/KeyMap.cxx
        scintilla/src/LineMarker.cxx
        scintilla/src/MarginView.cxx
        scintilla/src/PerLine.cxx
        scintilla/src/PositionCache.cxx
        scintilla/src/RESearch.cxx
        scintilla/src/RunStyles.cxx
        scintilla/src/ScintillaBase.cxx
        scintilla/src/Selection.cxx
        scintilla/src/Style.cxx
        scintilla/src/UndoHistory.cxx
        scintilla/src/UniConversion.cxx
        scintilla/src/UniqueString.cxx
        scintilla/src/ViewStyle.cxx
        scintilla/src/XPM.cxx
    )

    add_library(npp_scintilla STATIC ${SCINTILLA_CORE_SOURCES})
    add_library(npp::scintilla ALIAS npp_scintilla)

    target_include_directories(npp_scintilla
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include"
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/src"
    )
endif()

if(NPP_BUILD_LEXILLA)
    file(GLOB LEXILLA_LEXLIB_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/lexilla/lexlib/*.cxx"
    )
    file(GLOB LEXILLA_LEXER_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/lexilla/lexers/Lex*.cxx"
    )

    add_library(npp_lexilla STATIC
        lexilla/src/Lexilla.cxx
        ${LEXILLA_LEXLIB_SOURCES}
        ${LEXILLA_LEXER_SOURCES}
    )
    add_library(npp::lexilla ALIAS npp_lexilla)

    target_include_directories(npp_lexilla
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/lexilla/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include"
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/lexilla/lexlib"
            "${CMAKE_CURRENT_SOURCE_DIR}/lexilla/src"
    )
endif()

if(UNIX AND NOT WIN32 AND NPP_BUILD_PLATFORM_LINUX)
    add_library(npp_platform_linux STATIC
        platform/linux/LinuxClipboardService.cpp
        platform/linux/LinuxExtensionService.cpp
        platform/linux/LinuxDiagnosticsService.cpp
        platform/linux/LinuxFileSystemService.cpp
        platform/linux/LinuxLspClientService.cpp
        platform/linux/LinuxPathService.cpp
        platform/linux/LinuxProcessService.cpp
        platform/linux/VscodeLanguageCompatibility.cpp
    )
    add_library(npp::platform_linux ALIAS npp_platform_linux)

    target_include_directories(npp_platform_linux
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/platform/include"
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/platform/linux"
            "${CMAKE_CURRENT_SOURCE_DIR}/PowerEditor/src/json"
    )
endif()

if(UNIX AND NOT WIN32 AND NPP_BUILD_QT_SHELL)
    find_package(QT NAMES Qt6 Qt5 QUIET COMPONENTS Core Gui Widgets)
    if(QT_FOUND)
        find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Core Gui Widgets)
    endif()

    if(QT_FOUND
       AND TARGET Qt${QT_VERSION_MAJOR}::Core
       AND TARGET Qt${QT_VERSION_MAJOR}::Gui
       AND TARGET Qt${QT_VERSION_MAJOR}::Widgets
       AND TARGET npp_scintilla
       AND TARGET npp_platform_linux)
        if(QT_VERSION_MAJOR EQUAL 6)
            find_package(Qt6 QUIET COMPONENTS Core5Compat)
        endif()

        add_library(npp_scintilla_qt STATIC
            scintilla/qt/ScintillaEditBase/PlatQt.cpp
            scintilla/qt/ScintillaEditBase/ScintillaQt.cpp
            scintilla/qt/ScintillaEditBase/ScintillaQt.h
            scintilla/qt/ScintillaEditBase/ScintillaEditBase.cpp
            scintilla/qt/ScintillaEditBase/ScintillaEditBase.h
        )
        add_library(npp::scintilla_qt ALIAS npp_scintilla_qt)

        set_target_properties(npp_scintilla_qt PROPERTIES
            AUTOMOC ON
            AUTOUIC OFF
            AUTORCC OFF
        )

        target_compile_definitions(npp_scintilla_qt
            PRIVATE
                SCINTILLA_QT=1
                MAKING_LIBRARY=1
        )

        target_include_directories(npp_scintilla_qt
            PUBLIC
                "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include"
                "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/qt/ScintillaEditBase"
            PRIVATE
                "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/src"
        )

        target_link_libraries(npp_scintilla_qt
            PUBLIC
                npp_scintilla
                Qt${QT_VERSION_MAJOR}::Core
                Qt${QT_VERSION_MAJOR}::Gui
                Qt${QT_VERSION_MAJOR}::Widgets
        )

        if(TARGET Qt6::Core5Compat)
            target_link_libraries(npp_scintilla_qt PRIVATE Qt6::Core5Compat)
        endif()

        add_executable(npp_linux_shell
            ui/qt/main.cpp
            ui/qt/LanguageAwareFormatter.cpp
            ui/qt/LanguageAwareFormatter.h
            ui/qt/MainWindow.cpp
            ui/qt/MainWindow.h
            ui/qt/LspBaselineFeatures.cpp
            ui/qt/LspBaselineFeatures.h
        )
        add_executable(npp::linux_shell ALIAS npp_linux_shell)

        set_target_properties(npp_linux_shell PROPERTIES
            AUTOMOC ON
            AUTOUIC OFF
            AUTORCC OFF
            OUTPUT_NAME "notepad-plus-plus-linux"
        )

        target_include_directories(npp_linux_shell
            PRIVATE
                "${CMAKE_CURRENT_SOURCE_DIR}/lexilla/include"
                "${CMAKE_CURRENT_SOURCE_DIR}/platform/include"
                "${CMAKE_CURRENT_SOURCE_DIR}/platform/linux"
                "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include"
                "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/src"
                "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/qt/ScintillaEditBase"
                "${CMAKE_CURRENT_SOURCE_DIR}/ui/qt"
        )

        target_link_libraries(npp_linux_shell
            PRIVATE
                npp::language_detection
                npp::platform_linux
                npp::scintilla_qt
                Qt${QT_VERSION_MAJOR}::Core
                Qt${QT_VERSION_MAJOR}::Gui
                Qt${QT_VERSION_MAJOR}::Widgets
        )
        target_compile_definitions(npp_linux_shell PRIVATE NPP_APP_VERSION="${NPP_APP_VERSION}")

        if(TARGET npp_lexilla)
            target_compile_definitions(npp_linux_shell PRIVATE NPP_HAVE_LEXILLA=1)
            target_link_libraries(npp_linux_shell PRIVATE npp::lexilla)
        endif()

        install(TARGETS npp_linux_shell
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        )

        install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/notepad-plus-plus-linux.desktop"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
        )

        install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/icons/hicolor/scalable/apps/notepad-plus-plus-linux.svg"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps"
        )

        install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/mime/notepad-plus-plus-linux.xml"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/mime/packages"
        )

        install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/skins/light.json"
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/skins/dark.json"
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/skins/high-contrast.json"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/notepad-plus-plus-linux/skins"
        )

        install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/default-update-channel"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/notepad-plus-plus-linux"
        )

        install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/extensions/index.json"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/notepad-plus-plus-linux/extensions"
        )
    else()
        message(STATUS "Skipping npp_linux_shell: Qt Core/Gui/Widgets or prerequisite targets not available")
    endif()
endif()

include(CTest)

if(BUILD_TESTING AND TARGET npp_language_detection)
    add_executable(language_detection_regression_test tests/regression/language_detection_regression_test.cpp)
    target_link_libraries(language_detection_regression_test PRIVATE npp::language_detection)
    target_compile_definitions(language_detection_regression_test PRIVATE
        NPP_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )
    add_test(NAME language_detection_regression_test COMMAND language_detection_regression_test)
endif()

if(BUILD_TESTING)
    add_executable(skin_accessibility_regression_test tests/regression/skin_accessibility_regression_test.cpp)
    target_compile_definitions(skin_accessibility_regression_test PRIVATE
        NPP_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )
    add_test(NAME skin_accessibility_regression_test COMMAND skin_accessibility_regression_test)

    add_executable(lexer_style_config_regression_test tests/regression/lexer_style_config_regression_test.cpp)
    target_include_directories(lexer_style_config_regression_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ui/qt"
        "${CMAKE_CURRENT_SOURCE_DIR}/lexilla/include"
    )
    add_test(NAME lexer_style_config_regression_test COMMAND lexer_style_config_regression_test)

    add_executable(
        lsp_baseline_ux_regression_test
        tests/regression/lsp_baseline_ux_regression_test.cpp
        ui/qt/LspBaselineFeatures.cpp)
    target_include_directories(lsp_baseline_ux_regression_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ui/qt"
    )
    add_test(NAME lsp_baseline_ux_regression_test COMMAND lsp_baseline_ux_regression_test)

    add_executable(
        language_aware_formatting_regression_test
        tests/regression/language_aware_formatting_regression_test.cpp
        ui/qt/LanguageAwareFormatter.cpp)
    target_include_directories(language_aware_formatting_regression_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ui/qt"
    )
    add_test(
        NAME language_aware_formatting_regression_test
        COMMAND language_aware_formatting_regression_test)
endif()

if(BUILD_TESTING AND TARGET npp_lexilla)
    add_executable(lexilla_smoke_test tests/smoke/lexilla_smoke.cpp)
    target_include_directories(lexilla_smoke_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/lexilla/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include"
    )
    target_link_libraries(lexilla_smoke_test PRIVATE npp_lexilla)
    add_test(NAME lexilla_smoke_test COMMAND lexilla_smoke_test)
endif()

if(BUILD_TESTING AND TARGET npp_lexilla AND TARGET npp_scintilla)
    add_executable(syntax_highlighting_smoke_test tests/smoke/syntax_highlighting_smoke_test.cpp)
    target_sources(syntax_highlighting_smoke_test PRIVATE
        tests/support/scintilla_debug_stubs.cpp
    )
    target_include_directories(syntax_highlighting_smoke_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/lexilla/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/src"
    )
    target_link_libraries(syntax_highlighting_smoke_test PRIVATE npp_lexilla npp_scintilla)
    add_test(NAME syntax_highlighting_smoke_test COMMAND syntax_highlighting_smoke_test)

    add_executable(startup_typing_benchmark tests/perf/startup_typing_benchmark.cpp)
    target_sources(startup_typing_benchmark PRIVATE
        tests/support/scintilla_debug_stubs.cpp
    )
    target_include_directories(startup_typing_benchmark PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/lexilla/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/src"
    )
    target_link_libraries(startup_typing_benchmark PRIVATE npp_lexilla npp_scintilla)
endif()

if(BUILD_TESTING AND TARGET npp_scintilla)
    add_executable(scintilla_document_smoke_test tests/smoke/scintilla_document_smoke.cpp)
    target_include_directories(scintilla_document_smoke_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/src"
    )
    target_link_libraries(scintilla_document_smoke_test PRIVATE npp_scintilla)
    add_test(NAME scintilla_document_smoke_test COMMAND scintilla_document_smoke_test)

    add_executable(core_text_undo_redo_test tests/unit/core_text_undo_redo_test.cpp)
    target_sources(core_text_undo_redo_test PRIVATE
        tests/support/scintilla_debug_stubs.cpp
    )
    target_include_directories(core_text_undo_redo_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/src"
    )
    target_link_libraries(core_text_undo_redo_test PRIVATE npp_scintilla)
    add_test(NAME core_text_undo_redo_test COMMAND core_text_undo_redo_test)

    add_executable(encoding_large_file_regression_test tests/regression/encoding_large_file_regression_test.cpp)
    target_sources(encoding_large_file_regression_test PRIVATE
        tests/support/scintilla_debug_stubs.cpp
    )
    target_include_directories(encoding_large_file_regression_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/src"
    )
    target_link_libraries(encoding_large_file_regression_test PRIVATE npp_scintilla)
    add_test(NAME encoding_large_file_regression_test COMMAND encoding_large_file_regression_test)
endif()

if(BUILD_TESTING AND TARGET npp_platform_linux)
    add_executable(platform_linux_services_smoke_test tests/platform/linux_services_smoke.cpp)
    target_include_directories(platform_linux_services_smoke_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/linux"
    )
    target_link_libraries(platform_linux_services_smoke_test PRIVATE npp_platform_linux)
    add_test(NAME platform_linux_services_smoke_test COMMAND platform_linux_services_smoke_test)

    add_executable(extension_lifecycle_smoke_test tests/platform/extension_lifecycle_smoke_test.cpp)
    target_include_directories(extension_lifecycle_smoke_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/linux"
    )
    target_link_libraries(extension_lifecycle_smoke_test PRIVATE npp_platform_linux)
    add_test(NAME extension_lifecycle_smoke_test COMMAND extension_lifecycle_smoke_test)

    add_executable(
        crash_recovery_persistence_regression_test
        tests/regression/crash_recovery_persistence_regression_test.cpp)
    target_include_directories(crash_recovery_persistence_regression_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/linux"
    )
    target_link_libraries(crash_recovery_persistence_regression_test PRIVATE npp_platform_linux)
    add_test(
        NAME crash_recovery_persistence_regression_test
        COMMAND crash_recovery_persistence_regression_test)

    add_executable(lsp_client_foundation_smoke_test tests/platform/lsp_client_foundation_smoke_test.cpp)
    target_include_directories(lsp_client_foundation_smoke_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/linux"
    )
    target_link_libraries(lsp_client_foundation_smoke_test PRIVATE npp_platform_linux)
    add_test(NAME lsp_client_foundation_smoke_test COMMAND lsp_client_foundation_smoke_test)
endif()

if(BUILD_TESTING AND TARGET npp_platform_linux)
    add_executable(
        vscode_language_asset_compatibility_test
        tests/regression/vscode_language_asset_compatibility_test.cpp)
    target_include_directories(vscode_language_asset_compatibility_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/linux"
    )
    target_compile_definitions(vscode_language_asset_compatibility_test PRIVATE
        NPP_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )
    target_link_libraries(vscode_language_asset_compatibility_test PRIVATE npp_platform_linux)
    add_test(NAME vscode_language_asset_compatibility_test COMMAND vscode_language_asset_compatibility_test)
endif()
