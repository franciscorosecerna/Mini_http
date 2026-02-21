cmake_minimum_required(VERSION 3.16)

project(MiniHttp
    VERSION 0.2
    DESCRIPTION "Lightweight HTTP server framework in C++"
    LANGUAGES CXX
)

add_library(MiniHttp
    src/core/App.cpp
    src/core/Router.cpp
    src/http/Response.cpp
)

add_library(MiniHttp::MiniHttp ALIAS MiniHttp)

target_compile_features(MiniHttp PUBLIC cxx_std_17)

target_include_directories(MiniHttp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(MSVC)
    target_compile_options(MiniHttp PRIVATE /W4)
else()
    target_compile_options(MiniHttp PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(GNUInstallDirs)

install(TARGETS MiniHttp
    EXPORT MiniHttpTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT MiniHttpTargets
    FILE MiniHttpTargets.cmake
    NAMESPACE MiniHttp::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MiniHttp
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    cmake/MiniHttpConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MiniHttpConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MiniHttp
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/MiniHttpConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MiniHttp
)

if(WIN32)
    target_link_libraries(MiniHttp PUBLIC ws2_32)
endif()