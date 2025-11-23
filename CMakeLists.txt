cmake_minimum_required(VERSION 3.16)
project(TRDPTestingTool LANGUAGES CXX)

option(TRDP_ENABLE_TESTS "Build TRDPTestingTool test targets" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure required submodules are present to avoid missing-header build failures.
set(TRDP_FTXUI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/FTXUI")
set(TRDP_TCNOPEN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/TCNopen")

if(NOT EXISTS "${TRDP_FTXUI_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR "FTXUI submodule not found. Please run: git submodule update --init --recursive")
endif()

if(NOT EXISTS "${TRDP_TCNOPEN_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR "TCNopen submodule not found. Please run: git submodule update --init --recursive")
endif()

# FTXUI configuration
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "Disable upstream examples" FORCE)
set(FTXUI_BUILD_TESTS OFF CACHE BOOL "Disable upstream tests" FORCE)
set(FTXUI_BUILD_MODULES OFF CACHE BOOL "Disable upstream C++20 modules" FORCE)
set(FTXUI_BUILD_DOCS OFF CACHE BOOL "Disable upstream docs" FORCE)
add_subdirectory(external/FTXUI)

# TRDP Light stack configuration
set(TRDP_BUILD_EXAMPLES OFF CACHE BOOL "Disable TRDP example applications" FORCE)
set(TRDP_BUILD_TSN_EXAMPLES OFF CACHE BOOL "Disable TRDP TSN demos" FORCE)
set(TRDP_BUILD_TESTS OFF CACHE BOOL "Disable TRDP test utilities" FORCE)
set(TRDP_BUILD_XML_TOOLS OFF CACHE BOOL "Disable TRDP XML helpers" FORCE)
set(TRDP_BUILD_LOCAL_TESTS OFF CACHE BOOL "Disable TRDP local tests" FORCE)
set(TRDP_BUILD_MARSHALLING_TOOLS OFF CACHE BOOL "Disable TRDP marshalling demos" FORCE)
add_subdirectory(external/TCNopen)

# tau_xml configuration hook
set(
    TAU_XML_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/external/TCNopen/trdp/src/api"
    CACHE PATH "Path to tau_xml headers"
)
set(
    TAU_XML_LIBRARIES
    tau_shared
    CACHE STRING "Linker flags or libraries for tau_xml"
)
add_library(tau_xml INTERFACE)
 target_include_directories(tau_xml INTERFACE ${TAU_XML_INCLUDE_DIR})
 target_link_libraries(tau_xml INTERFACE ${TAU_XML_LIBRARIES})

add_library(trdp_config STATIC
    src/config/xml_loader.cpp
)
target_include_directories(trdp_config PUBLIC src)
target_link_libraries(trdp_config PUBLIC tau_xml TRDP::trdp)

add_library(trdp_runtime STATIC
    src/trdp/trdp_session.cpp
    src/trdp/pd_endpoint.cpp
    src/util/logging.cpp
)
target_include_directories(trdp_runtime PUBLIC src)
target_link_libraries(trdp_runtime PUBLIC trdp_config)

add_executable(trdp_simulator
    src/main.cpp
    src/ui/screen_config_summary.cpp
)
target_include_directories(trdp_simulator PRIVATE src)

target_link_libraries(
    trdp_simulator
    PRIVATE
        trdp_config
        trdp_runtime
        tau_xml
        ftxui::component
        ftxui::dom
        ftxui::screen
)

if(TRDP_ENABLE_TESTS)
    set(BUILD_TESTING ON CACHE BOOL "Enable CTest-based targets" FORCE)
    include(CTest)
    enable_testing()

    add_executable(xml_loader_test EXCLUDE_FROM_ALL
        tests/xml_loader_test.cpp
    )
    target_include_directories(xml_loader_test PRIVATE src)
    target_link_libraries(xml_loader_test PRIVATE trdp_config tau_xml)

    add_executable(trdp_runtime_test EXCLUDE_FROM_ALL
        tests/trdp_runtime_test.cpp
    )
    target_include_directories(trdp_runtime_test PRIVATE src)
    target_link_libraries(trdp_runtime_test PRIVATE trdp_runtime trdp_config tau_xml)

    add_custom_target(trdp_tests DEPENDS xml_loader_test trdp_runtime_test)

    add_test(NAME xml_loader_test COMMAND xml_loader_test)
    add_test(NAME trdp_runtime_test COMMAND trdp_runtime_test)

    if(TARGET test)
        add_dependencies(test trdp_tests)
    endif()
endif()
