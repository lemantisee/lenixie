project(Tests)

add_executable(${PROJECT_NAME}
    main.cpp
    test_DataPacket.cpp
    test_SString.cpp
)

add_subdirectory(Catch2)

target_link_libraries( ${PROJECT_NAME} PRIVATE
    Logger
    Json
    SString
    Catch2::Catch2
)

# include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})

set(CATCH_INSTALL_DOCS OFF)
# set(CATCH_INSTALL_EXTRAS ON)
set(CATCH_DEVELOPMENT_BUILD OFF)
set(CATCH_ENABLE_REPRODUCIBLE_BUILD OFF)

set(CATCH_BUILD_TESTING OFF)
set(CATCH_BUILD_EXAMPLES OFF)
set(CATCH_BUILD_EXTRA_TESTS OFF)
set(CATCH_BUILD_FUZZERS OFF)
set(CATCH_ENABLE_COVERAGE OFF)
set(CATCH_ENABLE_WERROR OFF)
set(CATCH_BUILD_SURROGATES OFF)
set(CATCH_ENABLE_CONFIGURE_TESTS OFF)
set(CATCH_ENABLE_CMAKE_HELPER_TESTS OFF)