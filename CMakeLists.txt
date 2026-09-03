cmake_minimum_required(VERSION 3.20)

project(codex-menubar-kde VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

include(CTest)

find_package(ECM 6.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Qml)
find_package(Plasma 6.0 REQUIRED)

plasma_install_package(package io.github.sangimed.codexmenubarkde)

add_library(codexmenubarkdeplugin SHARED
    src/CodexBackend.cpp
    src/CodexBackend.h
    src/CodexPlugin.cpp
    src/RateLimitParser.cpp
    src/RateLimitParser.h
)

target_link_libraries(codexmenubarkdeplugin
    PRIVATE
        Qt6::Core
        Qt6::Qml
)

target_compile_definitions(codexmenubarkdeplugin
    PRIVATE CODEX_MENUBAR_KDE_VERSION="${PROJECT_VERSION}"
)

install(
    TARGETS codexmenubarkdeplugin
    DESTINATION ${KDE_INSTALL_QMLDIR}/io/github/sangimed/codexmenubarkde
)

install(
    FILES src/qmldir
    DESTINATION ${KDE_INSTALL_QMLDIR}/io/github/sangimed/codexmenubarkde
)

if(BUILD_TESTING)
    find_package(Qt6 6.6 REQUIRED COMPONENTS Test)

    add_executable(rate-limit-parser-test
        tests/RateLimitParserTest.cpp
        src/RateLimitParser.cpp
        src/RateLimitParser.h
    )

    target_include_directories(rate-limit-parser-test PRIVATE src)
    target_link_libraries(rate-limit-parser-test PRIVATE Qt6::Core Qt6::Test)

    add_test(NAME rate-limit-parser COMMAND rate-limit-parser-test)
endif()
