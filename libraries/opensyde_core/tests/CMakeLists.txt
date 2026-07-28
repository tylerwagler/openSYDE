# openSYDE Core Unit Tests
#
# Built only when OPENSYDE_CORE_BUILD_TESTS=ON. Links against the opensyde_core
# static library and GoogleTest. Each test suite is a separate executable so
# failures are isolated and parallel ctest -j works efficiently.

# Load the GoogleTest CMake module (provides gtest_discover_tests).
# Required explicitly for CMake < 3.18; harmless for newer versions.
include(GoogleTest)

# Individual test executables --- one per source file ------------------------

function(add_osc_test TEST_NAME TEST_SOURCE)
   add_executable(${TEST_NAME} ${TEST_SOURCE})
   target_link_libraries(${TEST_NAME}
      PRIVATE
         opensyde_core
         GTest::gtest
         GTest::gtest_main
   )
   # Do not propagate the core library's precompiled headers to test targets
   target_precompile_headers(${TEST_NAME} REUSE_FROM opensyde_core)
   # Let GTest list tests at runtime so add_test works without enumerating
   # every TEST() macro.
   gtest_discover_tests(${TEST_NAME})
   # Label for filtering (e.g. ctest -L quick)
   set_property(TARGET ${TEST_NAME} PROPERTY LABELS "opensyde_core")
endfunction()

# --- Test suites --------------------------------------------------------------

add_osc_test(test_opensyde_core_checksums      test_checksums.cpp)
add_osc_test(test_opensyde_core_stwtypes       test_stwtypes.cpp)
add_osc_test(test_opensyde_core_stwerrors      test_stwerrors.cpp)
add_osc_test(test_opensyde_core_xml_parser     test_xml_parser.cpp)
add_osc_test(test_opensyde_core_dynamic_array  test_dynamic_array.cpp)
add_osc_test(test_opensyde_core_scl_string     test_scl_string.cpp)
add_osc_test(test_opensyde_core_logging        test_logging.cpp)
