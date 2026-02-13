cmake_minimum_required(VERSION 3.20)

project(notepad_plus_plus_linux LANGUAGES CXX)

option(NPP_BUILD_SCINTILLA "Build Scintilla core static library" ON)
option(NPP_BUILD_LEXILLA "Build Lexilla static library" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

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

include(CTest)

if(BUILD_TESTING AND TARGET npp_lexilla)
    add_executable(lexilla_smoke_test tests/smoke/lexilla_smoke.cpp)
    target_include_directories(lexilla_smoke_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/lexilla/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include"
    )
    target_link_libraries(lexilla_smoke_test PRIVATE npp_lexilla)
    add_test(NAME lexilla_smoke_test COMMAND lexilla_smoke_test)
endif()

if(BUILD_TESTING AND TARGET npp_scintilla)
    add_executable(scintilla_document_smoke_test tests/smoke/scintilla_document_smoke.cpp)
    target_include_directories(scintilla_document_smoke_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/src"
    )
    target_link_libraries(scintilla_document_smoke_test PRIVATE npp_scintilla)
    add_test(NAME scintilla_document_smoke_test COMMAND scintilla_document_smoke_test)
endif()
