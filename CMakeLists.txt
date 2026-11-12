cmake_minimum_required(VERSION 3.12)
project(mcx_lib VERSION 1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# PJSIP, installed with 'sudo make install' (provides libpjproject.pc)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PJPROJECT REQUIRED IMPORTED_TARGET libpjproject)

file(GLOB_RECURSE MCX_LIB_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(mcx_lib SHARED ${MCX_LIB_SOURCES})
target_include_directories(mcx_lib
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(mcx_lib PUBLIC cxx_std_17)
target_compile_definitions(mcx_lib PRIVATE MCX_LIB_BUILDING_LIBRARY)
set_target_properties(mcx_lib PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
target_link_libraries(mcx_lib PRIVATE PkgConfig::PJPROJECT)

if(MSVC)
    set(MCX_WARNING_OPTIONS /W4)
else()
    set(MCX_WARNING_OPTIONS -Wall -Wextra -Wpedantic)
endif()
target_compile_options(mcx_lib PRIVATE ${MCX_WARNING_OPTIONS})

set(MCX_LIB_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/mcx_lib")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/mcx_libConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/mcx_libConfig.cmake"
    INSTALL_DESTINATION "${MCX_LIB_INSTALL_CMAKEDIR}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/mcx_libConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(TARGETS mcx_lib
    EXPORT mcx_libTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(DIRECTORY include/api/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/api"
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT mcx_libTargets
    FILE mcx_libTargets.cmake
    NAMESPACE mcx::
    DESTINATION "${MCX_LIB_INSTALL_CMAKEDIR}"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mcx_libConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/mcx_libConfigVersion.cmake"
    DESTINATION "${MCX_LIB_INSTALL_CMAKEDIR}"
)

add_executable(mcx_lib_mock apps/mock_app/mcxlibapp.cpp)
target_link_libraries(mcx_lib_mock PRIVATE mcx_lib)

include(CTest)
if(BUILD_TESTING)
    add_library(mcx_lib_test_support STATIC ${MCX_LIB_SOURCES})
    target_include_directories(mcx_lib_test_support PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(mcx_lib_test_support PUBLIC cxx_std_17)
    target_compile_definitions(mcx_lib_test_support PUBLIC MCX_LIB_STATIC)
    target_compile_options(mcx_lib_test_support PRIVATE ${MCX_WARNING_OPTIONS})
    target_link_libraries(mcx_lib_test_support PUBLIC PkgConfig::PJPROJECT)

    file(GLOB_RECURSE MCX_LIB_TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    )

    foreach(TEST_SOURCE IN LISTS MCX_LIB_TEST_SOURCES)
        file(RELATIVE_PATH TEST_NAME "${CMAKE_CURRENT_SOURCE_DIR}/tests" "${TEST_SOURCE}")
        string(REGEX REPLACE "\\.cpp$" "" TEST_NAME "${TEST_NAME}")
        string(REPLACE "/" "_" TEST_NAME "${TEST_NAME}")
        string(REPLACE "\\" "_" TEST_NAME "${TEST_NAME}")
        set(TEST_TARGET "mcx_test_${TEST_NAME}")
        add_executable("${TEST_TARGET}" "${TEST_SOURCE}")
        target_link_libraries("${TEST_TARGET}" PRIVATE mcx_lib_test_support)
        add_test(NAME "${TEST_TARGET}" COMMAND "${TEST_TARGET}")
        set_tests_properties("${TEST_TARGET}" PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    endforeach()
endif()
