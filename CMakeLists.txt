cmake_minimum_required(VERSION 3.16)
project(TRDPTestingTool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
set(TAU_XML_INCLUDE_DIR "" CACHE PATH "Path to tau_xml headers")
set(TAU_XML_LIBRARIES "" CACHE STRING "Linker flags or libraries for tau_xml")
add_library(tau_xml INTERFACE)
target_include_directories(tau_xml INTERFACE ${TAU_XML_INCLUDE_DIR})
target_link_libraries(tau_xml INTERFACE ${TAU_XML_LIBRARIES})

add_executable(trdp_simulator src/main.cpp)
target_link_libraries(
    trdp_simulator
    PRIVATE
        tau_xml
        TRDP::trdp
        ftxui::component
        ftxui::dom
        ftxui::screen
)
