cmake_minimum_required(VERSION 3.12)

project(mcx_lib VERSION 1.0 LANGUAGES CXX)

# Add library implementation files (.cpp) under src/. This recursive search
# includes new class files and subdirectories automatically after reconfiguration.
file(GLOB_RECURSE MCX_LIB_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(mcx_lib ${MCX_LIB_SOURCES})

# Put public headers under include/ and include them as <module/header.hpp>.
# Add dependency include directories here if the library needs external headers.
target_include_directories(mcx_lib
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Change this when adopting a newer C++ standard.
target_compile_features(mcx_lib PUBLIC cxx_std_17)

# Add or adjust project-wide compiler warning/options here.
if(MSVC)
    target_compile_options(mcx_lib PRIVATE /W4)
else()
    target_compile_options(mcx_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    # Every .cpp file under tests/ is built and registered as an independent
    # test executable. Use its relative path for a unique target/test name.
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
        target_link_libraries("${TEST_TARGET}" PRIVATE mcx_lib)
        target_include_directories("${TEST_TARGET}" PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/helper_tool"
        )

        add_test(NAME "${TEST_TARGET}" COMMAND "${TEST_TARGET}")
        set_tests_properties("${TEST_TARGET}" PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    endforeach()
endif()