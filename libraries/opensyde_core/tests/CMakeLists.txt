# openSYDE Core Unit Tests
#
# Built only when OPENSYDE_CORE_BUILD_TESTS=ON. Links against the opensyde_core
# static library and GoogleTest. Each test suite is a separate executable so
# failures are isolated and parallel ctest -j works efficiently.

# Load the GoogleTest CMake module (provides gtest_discover_tests).
# Required explicitly for CMake < 3.18; harmless for newer versions.
include(GoogleTest)

# The security suites exercise OpenSSL-backed code paths.
find_package(OpenSSL QUIET)

# Individual test executables --- one per source file ------------------------

function(add_osc_test TEST_NAME TEST_SOURCE)
   add_executable(${TEST_NAME} ${TEST_SOURCE})
   target_link_libraries(${TEST_NAME}
      PRIVATE
         opensyde_core
         GTest::gtest
         GTest::gtest_main
   )
   # opensyde_core deliberately does not link OpenSSL — applications provide it
   # (see the note at the top of the core CMakeLists). Test executables are the
   # application here, so they must link it themselves for the security suites.
   if(TARGET OpenSSL::Crypto)
      target_link_libraries(${TEST_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
   endif()
   # Tests are code too, and were the one place in the tree building without -Werror.
   # They sat at a single warning -- a discarded [[nodiscard]] return -- which is
   # exactly the class the attribute exists to surface. Fixed, and held at zero here.
   if(NOT MSVC)
      target_compile_options(${TEST_NAME} PRIVATE -Wall -Wextra -Werror)
   endif()

   # Do not propagate the core library's precompiled headers to test targets
   target_precompile_headers(${TEST_NAME} REUSE_FROM opensyde_core)
   # Let GTest list tests at runtime so add_test works without enumerating
   # every TEST() macro.
   # DISCOVERY_TIMEOUT defaults to 5 seconds. On macOS the first launch of a freshly
   # linked binary goes through Gatekeeper, and with a dozen test executables being
   # discovered in parallel that was enough to time out several -- transiently, and
   # only on the first build. The binaries themselves list their tests in ~40 ms.
   gtest_discover_tests(${TEST_NAME} DISCOVERY_TIMEOUT 60)
   # Label for filtering (e.g. ctest -L quick)
   set_property(TARGET ${TEST_NAME} PROPERTY LABELS "opensyde_core")
endfunction()

# --- Test suites --------------------------------------------------------------

add_osc_test(test_opensyde_core_checksums      test_checksums.cpp)
add_osc_test(test_opensyde_core_cstdint        test_cstdint.cpp)
add_osc_test(test_opensyde_core_endian         test_endian.cpp)
add_osc_test(test_opensyde_core_build_info     test_build_info.cpp)
add_osc_test(test_opensyde_core_stwerrors      test_stwerrors.cpp)
add_osc_test(test_opensyde_core_error_category test_osc_error_category.cpp)
add_osc_test(test_opensyde_core_xml_parser     test_xml_parser.cpp)
add_osc_test(test_opensyde_core_dynamic_array  test_dynamic_array.cpp)
add_osc_test(test_opensyde_core_scl_string     test_scl_string.cpp)
add_osc_test(test_opensyde_core_logging        test_logging.cpp)
add_osc_test(test_opensyde_core_hex_parsing    test_hex_string_parsing.cpp)
add_osc_test(test_opensyde_core_aes_file       test_security_aes_file.cpp)
add_osc_test(test_opensyde_core_hex_file       test_hex_file.cpp)
add_osc_test(test_opensyde_core_serial_number  test_protocol_serial_number.cpp)
add_osc_test(test_opensyde_core_app_info        test_application_info_block.cpp)
add_osc_test(test_opensyde_core_tp_base        test_protocol_driver_base.cpp)
add_osc_test(test_opensyde_core_dp_content     test_node_datapool_content.cpp)
add_osc_test(test_opensyde_core_dp_util        test_datapool_content_util.cpp)
add_osc_test(test_opensyde_core_project_meta   test_project_metadata.cpp)
add_osc_test(test_opensyde_core_view_security test_view_security_options.cpp)
add_osc_test(test_opensyde_core_tgl_file      test_tgl_file.cpp)
add_osc_test(test_opensyde_core_tgl_time      test_tgl_time.cpp)
add_osc_test(test_opensyde_core_device_manager test_device_manager.cpp)
add_osc_test(test_opensyde_core_datapool_type_string test_datapool_type_string.cpp)
add_osc_test(test_opensyde_core_dl_trigger     test_data_logger_trigger_parser.cpp)
add_osc_test(test_opensyde_core_md5           test_md5.cpp)
add_osc_test(test_opensyde_core_canopen_eds  test_canopen_eds.cpp)
add_osc_test(test_opensyde_core_crc_xml      test_checksummed_xml.cpp)
add_osc_test(test_opensyde_core_niceify     test_osc_utils_niceify.cpp)
add_osc_test(test_opensyde_core_filer_rt    test_filer_roundtrip.cpp)
add_osc_test(test_opensyde_core_xco_create  test_xco_create.cpp)
add_osc_test(test_opensyde_core_sup_package test_sup_package.cpp)
add_osc_test(test_opensyde_core_tsp_filer   test_tsp_filer.cpp)
add_osc_test(test_opensyde_core_halc_magician test_halc_magician.cpp)
add_osc_test(test_opensyde_core_code_export test_code_export.cpp)
add_osc_test(test_opensyde_core_can_protocol test_can_protocol.cpp)
add_osc_test(test_opensyde_core_security_sig test_security_signatures.cpp)
add_osc_test(test_opensyde_core_x_packages  test_x_packages.cpp)
add_osc_test(test_opensyde_core_routing     test_routing_calculation.cpp)
add_osc_test(test_opensyde_core_com_logger  test_com_message_logger.cpp)
add_osc_test(test_opensyde_core_data_dealer test_data_dealer.cpp)
add_osc_test(test_opensyde_core_osy_driver  test_protocol_driver_osy.cpp)
add_osc_test(test_opensyde_core_virtual_ecu test_su_sequences_virtual_ecu.cpp)
add_osc_test(test_opensyde_core_virtual_can test_can_transport_virtual_ecu.cpp)
add_osc_test(test_opensyde_core_xml_locale  test_xml_float_locale.cpp)
add_osc_test(test_opensyde_core_scl_ini_file test_scl_ini_file.cpp)
add_osc_test(test_opensyde_core_str_prefix   test_string_prefix_parsing.cpp)
