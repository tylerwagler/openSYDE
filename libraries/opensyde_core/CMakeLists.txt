# This file lists all openSYDE Core files and include paths.
# It can be included from the CMakeLists.txt file of an application.
# Some optional parts of the core can be skipped by setting options in the application CMakeLists.txt file before
#  including this file in order to keep build times low.
#  List of the groups (all but "COMMON_SOURCES" are skippable):
#     COMMON_SOURCES             basic infrastructure (also requires a target layer, e.g. for Windows or a
#                                                      corresponding implementation for e.g. Linux)
#     ZIPPING                    zip/unzip functionality
#     IMPORTS                    imports: EDS, DCF
#     CODE_GENERATION            source code generation
#     PROJECT_HANDLING           project handling (including all aspects of System Definition)
#     PARAM_SET                  parameter set handling (reading and writing syde_psi files)
#     PROTOCOL_DRIVERS_BASIC     communication protocol drivers basic update
#     PROTOCOL_DRIVERS_SYSTEM    communication protocol drivers system update
#     PROTOCOL_DRIVERS_MONITOR   communication protocol drivers monitor (openSYDE CAN Monitor basics)
#     PROTOCOL_DRIVERS_COMMON    communication protocol drivers common
#                                (openSYDE, STW Flashloader, Data Dealer, Dispatcher interface)
#     PROTOCOL_LOGGING           protocol logging
#     X_CONFIG_GENERATION        X-App configuration generation functionality
#     X_CERTIFICATES_GENERATION  X-App certificate generation functionality
#     SYSTEM_PACKAGE_HANDLING    system package base functionality
#     WINDOWS_DRIVERS            windows target drivers: CAN and Ethernet dispatchers
#     WINDOWS_TARGET             windows target glue layer
#     SECURITY                   security features like authentication, secure update, secure communication
#                                 (note: OpenSSL also needed to be provided by application)
#
# This CMakeLists.txt defined the library "opensyde_core" to be linked from the application.
# As the openSYDE core needs the "precomp_headers.hpp" that is placed with the application, the path
#  to that file needs to be set before this CMakeLists.txt is dragged in.
#
# Simplified example usage:
# <example>
# include_directories(
#    ../src
# )
# set(OPENSYDE_CORE_SKIP_ZIPPING 1)  #we do not want the zipping stuff
# add_subdirectory(../libs/opensyde_core ${CMAKE_BINARY_DIR}/opensyde_core)
# add_executable(my_application mymain.cpp)
# target_link_libraries(my_application
#   opensyde_core
# )
# </example>

# minimum 3.16 for precompiled headers
cmake_minimum_required(VERSION 3.16)
project("opensyde_core")

# --- libcan dependency -----------------------------------------------------------------------------------------------
# opensyde_core's CAN abstraction (C_CanDispatcher) is now backed by libcan from the
# can-libraries project, vendored as a git submodule at libraries/can-libraries. The
# C_OscLibcanBackendAdapter shim wraps can::ICanBackend instances behind the existing
# C_CanDispatcher interface, so all four backends (SocketCAN, PCANBasic, Kvaser, Vector XL)
# are reachable from any opensyde_core consumer. can-libraries also vendors the Vector
# file-format libraries (DBC/BLF/ASC) that openSYDE's CAN monitor / tool consume.
#
# Resolution order: prefer an installed CanLibraries package; fall back to add_subdirectory()
# of the submodule source tree so a fresh checkout builds without an install step. Override
# OPENSYDE_CANLIBS_SOURCE_DIR if the layout differs.
if(NOT TARGET CanLibraries::can)
   find_package(CanLibraries QUIET)
   if(NOT CanLibraries_FOUND)
      set(OPENSYDE_CANLIBS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../can-libraries"
          CACHE PATH "Path to the can-libraries submodule source tree (only used when CanLibraries isn't installed)")
      if(NOT EXISTS "${OPENSYDE_CANLIBS_SOURCE_DIR}/CMakeLists.txt")
         message(FATAL_ERROR
            "opensyde_core: CanLibraries (libcan) not found. "
            "Install it system-wide or set OPENSYDE_CANLIBS_SOURCE_DIR to the can-libraries "
            "source tree (currently '${OPENSYDE_CANLIBS_SOURCE_DIR}').")
      endif()
      message(STATUS "opensyde_core: building CanLibraries from ${OPENSYDE_CANLIBS_SOURCE_DIR}")
      # Keep CanLibraries from polluting our install / test registration. Force static linkage:
      # libcan would otherwise inherit BUILD_SHARED_LIBS=ON and turn opensyde_core into a .so,
      # which then can't resolve OpenSSL symbols at link time the way the static build does.
      # Enable the Vector_DBC + Vector_BLF targets: openSYDE's CAN monitor links CanLibraries::blf
      # and CanLibraries::dbc, and the tool links CanLibraries::dbc, replacing the formerly
      # in-tree vector_blf / dbc_driver_library copies. ASC stays off (unused by openSYDE).
      set(CANLIB_INSTALL OFF CACHE BOOL "" FORCE)
      set(_OPENSYDE_BUILD_TESTING_SAVED ${BUILD_TESTING})
      set(_OPENSYDE_BUILD_SHARED_LIBS_SAVED ${BUILD_SHARED_LIBS})
      set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
      set(BUILD_SHARED_LIBS OFF)
      set(CANLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
      set(CANLIB_USE_VECTOR_DBC ON CACHE BOOL "" FORCE)
      set(CANLIB_USE_VECTOR_BLF ON CACHE BOOL "" FORCE)
      set(CANLIB_USE_VECTOR_ASC OFF CACHE BOOL "" FORCE)
      add_subdirectory(${OPENSYDE_CANLIBS_SOURCE_DIR} can-libraries-build)
      set(BUILD_TESTING ${_OPENSYDE_BUILD_TESTING_SAVED} CACHE BOOL "" FORCE)
      set(BUILD_SHARED_LIBS ${_OPENSYDE_BUILD_SHARED_LIBS_SAVED})
   endif()
endif()

# --- Dependency checks ------------------------------------------------------------------------------------------------
# Some packages depend on others. To make life simpler for applications we drag in required other packages
#  even if "_SKIP_"ed.
if(NOT OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON)
   #We also need the security package (e.g. for routines for traffic encryption)
   if (OPENSYDE_CORE_SKIP_SECURITY)
      set(OPENSYDE_CORE_SKIP_SECURITY 0)
      message("OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON not set: forcing OPENSYDE_CORE_SKIP_SECURITY to 0")
   endif()
endif()

if(NOT OPENSYDE_CORE_SKIP_SECURITY)
   #We also need the zipping package
   if (OPENSYDE_CORE_SKIP_ZIPPING)
      set(OPENSYDE_CORE_SKIP_ZIPPING 0)
      message("OPENSYDE_CORE_SKIP_SECURITY not set: forcing OPENSYDE_CORE_SKIP_ZIPPING to 0")
   endif()
endif()

if (NOT OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM)
   #We also need the zipping package (for handling .syde_sup packages)
   if (OPENSYDE_CORE_SKIP_ZIPPING)
      set(OPENSYDE_CORE_SKIP_ZIPPING 0)
      message("OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM not set: forcing OPENSYDE_CORE_SKIP_ZIPPING to 0")
   endif()
endif()

# --- Project source files ---------------------------------------------------------------------------------------------
#basic infrastructure (also requires Windows target layer, or a corresponding implementation for e.g. Linux)
set(OPENSYDE_CORE_COMMON_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/precompiled_headers/precomp_headers.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/util/C_OscUtilBinaryHash.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/util/C_OscUtilChecksummedIniFile.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/util/C_OscUtils.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/stw_compid/stwcompid.c
   ${CMAKE_CURRENT_SOURCE_DIR}/logging/C_OscLoggingHandler.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/md5/C_Md5Checksum.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/scl/C_SclChecksums.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/scl/C_SclDateTime.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/scl/C_SclIniFile.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/scl/C_SclString.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/scl/C_SclStringList.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xml_parser/C_OscXmlParser.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xml_parser/C_OscXmlParserLog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xml_parser/C_OscChecksummedXml.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xml_parser/tinyxml2/tinyxml2.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/util/C_OscUtilBinaryHash.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/util/C_OscUtilChecksummedIniFile.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/util/C_OscUtils.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/stw_compid/stwcompid.h
   ${CMAKE_CURRENT_SOURCE_DIR}/logging/C_OscLoggingHandler.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/md5/C_Md5Checksum.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/scl/C_SclChecksums.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/scl/C_SclDateTime.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/scl/C_SclIniFile.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/scl/C_SclString.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/scl/C_SclStringList.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/scl/C_SclDynamicArray.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/stwerrors/stwerrors.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/stwerrors/stwerrors.h
   ${CMAKE_CURRENT_SOURCE_DIR}/stwtypes/stwtypes.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/stwtypes/stwtypes.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xml_parser/C_OscXmlParser.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xml_parser/C_OscXmlParserLog.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xml_parser/C_OscChecksummedXml.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xml_parser/tinyxml2/tinyxml2.h
   ${CMAKE_CURRENT_SOURCE_DIR}/conf_file_handler/C_OscConfFileHandler.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/conf_file_handler/C_OscConfFileHandler.hpp
)

set(OPENSYDE_CORE_COMMON_INCLUDE_DIRECTORIES
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/conf_file_handler
   ${CMAKE_CURRENT_SOURCE_DIR}/stw_compid
   ${CMAKE_CURRENT_SOURCE_DIR}/logging
   ${CMAKE_CURRENT_SOURCE_DIR}/md5
   ${CMAKE_CURRENT_SOURCE_DIR}/precompiled_headers
   ${CMAKE_CURRENT_SOURCE_DIR}/scl
   ${CMAKE_CURRENT_SOURCE_DIR}/stwerrors
   ${CMAKE_CURRENT_SOURCE_DIR}/stwtypes
   ${CMAKE_CURRENT_SOURCE_DIR}/util
   ${CMAKE_CURRENT_SOURCE_DIR}/xml_parser
   ${CMAKE_CURRENT_SOURCE_DIR}/xml_parser/tinyxml2
)

# zip/unzip functionality
if(OPENSYDE_CORE_SKIP_ZIPPING)
   message("OPENSYDE_CORE_SKIP_ZIPPING detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_ZIPPING not detected ... dragging in package")

   set(OPENSYDE_CORE_ZIPPING_SOURCES

      ${CMAKE_CURRENT_SOURCE_DIR}/zip/C_OscZipData.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/zip/C_OscZipFile.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/miniz/miniz.c

      ${CMAKE_CURRENT_SOURCE_DIR}/zip/C_OscZipData.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/zip/C_OscZipFile.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/miniz/miniz.h
   )

   set(OPENSYDE_CORE_ZIPPING_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/miniz
      ${CMAKE_CURRENT_SOURCE_DIR}/zip
   )
endif()

# imports
if(OPENSYDE_CORE_SKIP_IMPORTS)
   message("OPENSYDE_CORE_SKIP_IMPORTS detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_IMPORTS not detected ... dragging in package")

   set(OPENSYDE_CORE_IMPORTS_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/imports/C_OscImportEdsDcf.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/imports/C_OscImportEdsDcf.hpp
   )

   set(OPENSYDE_CORE_IMPORTS_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/imports
   )

endif()

# source code generation
if(OPENSYDE_CORE_SKIP_CODE_GENERATION)
   message("OPENSYDE_CORE_SKIP_CODE_GENERATION detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_CODE_GENERATION not detected ... dragging in package")

   set(OPENSYDE_CORE_CODE_GENERATION_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportCommunicationStack.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportCanOpenConfig.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportCanOpenInit.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportDataPool.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportHalc.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportNode.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportOsyInit.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportUti.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportParamSet.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportCommunicationStack.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportCanOpenConfig.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportCanOpenInit.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportDataPool.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportHalc.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportNode.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportOsyInit.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportUti.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation/C_OscExportParamSet.hpp
   )

   set(OPENSYDE_CORE_CODE_GENERATION_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/code_generation
   )
endif()


# project handling (Including all aspects of System Definition)
if(OPENSYDE_CORE_SKIP_PROJECT_HANDLING)
   message("OPENSYDE_CORE_SKIP_PROJECT_HANDLING detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_PROJECT_HANDLING not detected ... dragging in package")

   set(OPENSYDE_CORE_PROJECT_HANDLING_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/C_OscHalcConfig.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/C_OscHalcConfigChannel.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/C_OscHalcConfigDomain.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/C_OscHalcConfigFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/C_OscHalcConfigParameter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/C_OscHalcConfigParameterStruct.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/C_OscHalcConfigUtil.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/standalone/C_OscHalcConfigStandalone.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/standalone/C_OscHalcConfigStandaloneChannel.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/standalone/C_OscHalcConfigStandaloneDomain.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/standalone/C_OscHalcConfigStandaloneFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/base/C_OscHalcDefBase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDef.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefChannelAvailability.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefChannelDef.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefChannelUseCase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefChannelValues.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefContent.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefContentBitmaskItem.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefDomain.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefElement.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefStruct.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefStructFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/magician/C_OscHalcMagicianDatapoolListHandler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/magician/C_OscHalcMagicianGenerator.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/magician/C_OscHalcMagicianUtil.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/C_OscProject.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/C_OscProjectFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscDeviceGroup.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscDeviceManager.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscSystemBus.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscSystemBusFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscSystemDefinition.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscSystemDefinitionFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscSystemNameMaxCharLimitChangeReportItem.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscSystemFilerUtil.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/device_definition/C_OscDeviceDefinition.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/device_definition/C_OscDeviceDefinitionFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/device_definition/C_OscSupportedCanInterfaceFeatures.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/device_definition/C_OscSubDeviceDefinition.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/target_support_package/C_OscTargetSupportPackage.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/target_support_package/C_OscTargetSupportPackageFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNode.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeSquad.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeSquadFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeApplication.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeCodeExportSettings.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeComInterfaceSettings.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPool.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolContent.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolContentUtil.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolDataSet.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolFiler.cpp
     
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolList.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolListElement.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolListElementId.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolListElementOptArrayId.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolListElementOptArrayOptValidId.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolListId.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeOpenSydeServerSettings.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeProperties.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeCommFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolId.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanMessage.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanMessageUniqueId.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanMessageContainer.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanMessageIdentificationIndices.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanProtocol.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanSignal.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanUtil.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanInterfaceId.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/can_open/C_OscCanOpenManagerMappableSignal.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/can_open/C_OscCanOpenManagerDeviceInfo.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/can_open/C_OscCanOpenManagerInfo.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/can_open/C_OscCanOpenManagerFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/data_logger/C_OscDataLoggerJob.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/data_logger/C_OscDataLoggerJobFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/data_logger/C_OscDataLoggerJobProperties.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/data_logger/C_OscDataLoggerDataElementReference.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/data_logger/C_OscDataLoggerJobAdditionalTriggerProperties.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/data_logger/C_OscDataLoggerJobAdditionalTriggerExpertMode.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/xapp/C_OscXappProperties.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/xapp/C_OscXappPropertiesFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/view/C_OscViewPc.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/view/C_OscViewData.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/view/C_OscViewNodeUpdate.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/view/C_OscViewNodeUpdateParamInfo.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/view/C_OscViewFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/imports/C_OscEdsDcfImportMessageGroup.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/imports/C_OscCanOpenObjectDictionary.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/imports/C_OscCanOpenEdsInfoBlock.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/imports/C_OscCanOpenEdsDeviceInfoBlock.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/imports/C_OscCanOpenEdsFileInfoBlock.cpp

      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/C_OscHalcConfig.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/C_OscHalcConfigChannel.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/C_OscHalcConfigDomain.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/C_OscHalcConfigFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/C_OscHalcConfigParameter.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/C_OscHalcConfigParameterStruct.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/C_OscHalcConfigUtil.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/standalone/C_OscHalcConfigStandalone.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/standalone/C_OscHalcConfigStandaloneChannel.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/standalone/C_OscHalcConfigStandaloneDomain.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/standalone/C_OscHalcConfigStandaloneFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/base/C_OscHalcDefBase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDef.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefChannelAvailability.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefChannelDef.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefChannelUseCase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefChannelValues.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefContent.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefContentBitmaskItem.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefDomain.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefElement.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefStruct.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/C_OscHalcDefStructFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/magician/C_OscHalcMagicianDatapoolListHandler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/magician/C_OscHalcMagicianGenerator.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/magician/C_OscHalcMagicianUtil.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/C_OscProject.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/C_OscProjectFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscDeviceGroup.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscDeviceManager.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscSystemBus.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscSystemBusFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscSystemDefinition.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscSystemDefinitionFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscSystemNameMaxCharLimitChangeReportItem.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/C_OscSystemFilerUtil.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/device_definition/C_OscDeviceDefinition.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/device_definition/C_OscDeviceDefinitionFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/device_definition/C_OscSupportedCanInterfaceFeatures.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/device_definition/C_OscSubDeviceDefinition.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/target_support_package/C_OscTargetSupportPackage.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/target_support_package/C_OscTargetSupportPackageFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNode.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeSquad.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeSquadFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeApplication.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeCodeExportSettings.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolContentUtil.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolId.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeComInterfaceSettings.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeCommFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPool.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolContent.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolDataSet.hpp
       ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolFiler.hpp
      
       ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolList.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolListElement.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolListElementId.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolListElementOptArrayId.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolListElementOptArrayOptValidId.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeDataPoolListId.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeOpenSydeServerSettings.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/C_OscNodeProperties.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanMessage.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanMessageUniqueId.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanMessageContainer.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanMessageIdentificationIndices.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanProtocol.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanSignal.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanUtil.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/C_OscCanInterfaceId.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/can_open/C_OscCanOpenManagerMappableSignal.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/can_open/C_OscCanOpenManagerDeviceInfo.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/can_open/C_OscCanOpenManagerInfo.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/can_open/C_OscCanOpenManagerFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/data_logger/C_OscDataLoggerJob.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/data_logger/C_OscDataLoggerJobFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/data_logger/C_OscDataLoggerJobProperties.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/data_logger/C_OscDataLoggerDataElementReference.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/data_logger/C_OscDataLoggerJobAdditionalTriggerProperties.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/data_logger/C_OscDataLoggerJobAdditionalTriggerExpertMode.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/xapp/C_OscXappProperties.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/xapp/C_OscXappPropertiesFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/view/C_OscViewData.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/view/C_OscViewPc.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/view/C_OscViewNodeUpdate.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/view/C_OscViewNodeUpdateParamInfo.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/view/C_OscViewFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/imports/C_OscEdsDcfImportMessageGroup.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/imports/C_OscCanOpenObjectDictionary.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/imports/C_OscCanOpenEdsInfoBlock.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/imports/C_OscCanOpenEdsDeviceInfoBlock.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/imports/C_OscCanOpenEdsFileInfoBlock.hpp
   )

   set(OPENSYDE_CORE_PROJECT_HANDLING_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/imports
      ${CMAKE_CURRENT_SOURCE_DIR}/halc
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/configuration/standalone
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/definition/base
      ${CMAKE_CURRENT_SOURCE_DIR}/halc/magician
      ${CMAKE_CURRENT_SOURCE_DIR}/project
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/device_definition
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/can/can_open
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/data_logger
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/node/xapp
      ${CMAKE_CURRENT_SOURCE_DIR}/project/system/target_support_package
      ${CMAKE_CURRENT_SOURCE_DIR}/project/view
   )
endif()

# param set handling (reading and writing syde_psi files)
if(OPENSYDE_CORE_SKIP_PARAM_SET)
   message("OPENSYDE_CORE_SKIP_PARAM_SET detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_PARAM_SET not detected ... dragging in package")

   set(OPENSYDE_CORE_PARAM_SET_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetDataPoolInfo.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetFilerBase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetHandler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetInterpretedData.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetInterpretedDataPool.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetInterpretedElement.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetInterpretedFileInfoData.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetInterpretedList.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetInterpretedNode.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetInterpretedNodeFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetRawEntry.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetRawNode.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetRawNodeFiler.cpp

      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetDataPoolInfo.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetFilerBase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetHandler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetInterpretedData.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetInterpretedDataPool.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetInterpretedElement.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetInterpretedFileInfoData.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetInterpretedList.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetInterpretedNode.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetInterpretedNodeFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetRawEntry.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetRawNode.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset/C_OscParamSetRawNodeFiler.hpp
   )

   set(OPENSYDE_CORE_PARAM_SET_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/paramset
   )
endif()

#communication protocol drivers basic update
if(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_BASIC)
   message("OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_BASIC detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_BASIC not detected ... dragging in package")

   set(OPENSYDE_CORE_PROTOCOL_DRIVERS_BASIC_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/basic_update/C_OscBuSequences.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/device_config/C_OscDcBasicSequences.cpp

      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/basic_update/C_OscBuSequences.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/device_config/C_OscDcBasicSequences.hpp
   )

   set(OPENSYDE_CORE_PROTOCOL_DRIVERS_BASIC_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/basic_update
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/device_config
   )
endif()

#communication protocol drivers common (openSYDE, STW Flashloader, KEFEX, Data Dealer, Dispatcher interface)
if(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON)
   message("OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON not detected ... dragging in package")

   set(OPENSYDE_CORE_PROTOCOL_DRIVERS_COMMON_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscProtocolDriverOsyTpCan.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/hex_file/C_HexFile.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/can_dispatcher/dispatcher/C_CanBase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/can_dispatcher/dispatcher/C_CanDispatcher.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/can_dispatcher/adapter/C_OscCanAdapterConfig.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/can_dispatcher/adapter/C_OscCanAdapterFactory.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/can_dispatcher/adapter/C_OscLibcanBackendAdapter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscProtocolDriverOsy.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscProtocolSerialNumber.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscProtocolDriverOsyTpBase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscProtocolSecuritySubLayer.cpp

      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscHexFile.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscApplicationInfoBlock.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscCanDispatcherOsyRouter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscDiagProtocolBase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscDiagProtocolOsy.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscProtocolDriverOsyTpIp.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscUpdateUtil.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComFlashloaderInformation.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/device_config/C_OscDcDeviceInformation.cpp

      ${CMAKE_CURRENT_SOURCE_DIR}/can_dispatcher/dispatcher/stw_can.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscProtocolDriverOsyTpCan.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/hex_file/C_HexFile.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ip_dispatcher/dispatcher/C_OscIpDispatcher.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/can_dispatcher/dispatcher/C_CanBase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/can_dispatcher/dispatcher/C_CanDispatcher.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/can_dispatcher/adapter/C_OscCanAdapterConfig.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/can_dispatcher/adapter/C_OscCanAdapterFactory.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/can_dispatcher/adapter/C_OscLibcanBackendAdapter.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscProtocolDriverOsy.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscProtocolSerialNumber.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscProtocolDriverOsyTpBase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscProtocolSecuritySubLayer.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscHexFile.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscApplicationInfoBlock.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscCanDispatcherOsyRouter.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscDiagProtocolBase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscDiagProtocolOsy.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscProtocolDriverOsyTpIp.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/C_OscUpdateUtil.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComFlashloaderInformation.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/device_config/C_OscDcDeviceInformation.hpp
   )

   set(OPENSYDE_CORE_PROTOCOL_DRIVERS_COMMON_INCLUDE_DIRECTORIES
       ${CMAKE_CURRENT_SOURCE_DIR}/can_dispatcher
       ${CMAKE_CURRENT_SOURCE_DIR}/can_dispatcher/dispatcher
       ${CMAKE_CURRENT_SOURCE_DIR}/can_dispatcher/adapter
       ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers
       ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication
       ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/device_config
        ${CMAKE_CURRENT_SOURCE_DIR}/hex_file
        ${CMAKE_CURRENT_SOURCE_DIR}/ip_dispatcher/dispatcher
    )
endif()

#communication protocol drivers system update
if(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM)
   message("OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM not detected ... dragging in package")

   set(OPENSYDE_CORE_PROTOCOL_DRIVERS_SYSTEM_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/system_update/C_OscSuSequences.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/system_update/C_OscSuSequencesNodeStates.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupServiceUpdatePackageCreate.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupServiceUpdatePackageLoad.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupServiceUpdatePackageBase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupSignatureFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupDefinition.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupDefinitionFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupNodeDefinition.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupNodeDefinitionFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/C_OscDataDealer.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/C_OscDataDealerNvm.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/C_OscDataDealerNvmSafe.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComDriverBase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComDriverFlash.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComDriverProtocol.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComSequencesBase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComAutoSupport.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/routing/C_OscRoutingCalculation.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/routing/C_OscRoutingRoute.cpp

      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/system_update/C_OscSuSequences.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/system_update/C_OscSuSequencesNodeStates.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupServiceUpdatePackageCreate.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupServiceUpdatePackageLoad.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupServiceUpdatePackageBase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupSignatureFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupDefinition.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupDefinitionFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupNodeDefinition.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package/C_OscSupNodeDefinitionFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/C_OscDataDealer.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/C_OscDataDealerNvm.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer/C_OscDataDealerNvmSafe.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComDriverBase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComDriverFlash.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComDriverProtocol.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComSequencesBase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComAutoSupport.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/routing/C_OscRoutingCalculation.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/routing/C_OscRoutingRoute.hpp
   )

   set(OPENSYDE_CORE_PROTOCOL_DRIVERS_SYSTEM_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/system_update
      ${CMAKE_CURRENT_SOURCE_DIR}/data_dealer
      ${CMAKE_CURRENT_SOURCE_DIR}/system_update_package
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/routing
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/system_update
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols
   )
endif()

#communication protocol drivers monitor (openSYDE CAN Monitor basics)
if(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR)
   message("OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR not detected ... dragging in package")

   set(OPENSYDE_CORE_PROTOCOL_DRIVERS_MONITOR_SOURCES
     ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComDriverBase.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComAutoSupport.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComDriverBase.hpp
     ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComAutoSupport.hpp
   )

   set(OPENSYDE_CORE_PROTOCOL_DRIVERS_MONITOR_INCLUDE_DIRECTORIES
       ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication
   )
endif()

# protocol logging
if(OPENSYDE_CORE_SKIP_PROTOCOL_LOGGING)
   message("OPENSYDE_CORE_SKIP_PROTOCOL_LOGGING detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_PROTOCOL_LOGGING not detected ... dragging in package")

   set (OPENSYDE_CORE_PROTOCOL_LOGGING_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocol.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocolBase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocolCanOpen.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocolGd.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocolJ1939.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocolL2.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocolOpenSyde.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocolUds.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComLoggerProtocolOpenSyde.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComLoggerProtocols.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComMessageLogger.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComMessageLoggerData.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComMessageLoggerFileAsc.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComMessageLoggerFileBase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComMessageLoggerOsySysDefConfig.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocol.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocolBase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocolCanOpen.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocolGd.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocolJ1939.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocolL2.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocolOpenSyde.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols/C_CanMonProtocolUds.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComLoggerProtocolOpenSyde.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComLoggerProtocols.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComMessageLogger.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComMessageLoggerData.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComMessageLoggerFileAsc.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComMessageLoggerFileBase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/protocol_drivers/communication/C_OscComMessageLoggerOsySysDefConfig.hpp
   )
   set(OPENSYDE_CORE_PROTOCOL_LOGGING_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/cmon_protocols
   )
endif()

# optional: system package base functionality
if(OPENSYDE_CORE_SKIP_SYSTEM_PACKAGE_HANDLING)
   message("OPENSYDE_CORE_SKIP_SYSTEM_PACKAGE_HANDLING detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_SYSTEM_PACKAGE_HANDLING not detected ... dragging in package")

   set(OPENSYDE_CORE_SYSTEM_PACKAGE_HANDLING_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/system_package_handling/C_OscSpaServicePackageCreateUtil.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_package_handling/C_OscSpaServicePackageLoadUtil.cpp

      ${CMAKE_CURRENT_SOURCE_DIR}/system_package_handling/C_OscSpaServicePackageCreateUtil.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/system_package_handling/C_OscSpaServicePackageLoadUtil.hpp
   )

   set(OPENSYDE_CORE_SYSTEM_PACKAGE_HANDLING_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/system_package_handling
   )
endif()

# optional: X-App certificate generation functionality
if(OPENSYDE_CORE_SKIP_X_CERTIFICATES_GENERATION)
   message("OPENSYDE_CORE_SKIP_X_CERTIFICATES_GENERATION detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_X_CERTIFICATES_GENERATION not detected ... dragging in package")

   set(OPENSYDE_CORE_X_CERTIFICATES_GENERATION_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_certificates_package_generation/C_OscXceManifest.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_certificates_package_generation/C_OscXceManifestFiler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_certificates_package_generation/C_OscXceUpdatePackageParameters.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_certificates_package_generation/C_OscXceBase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_certificates_package_generation/C_OscXceCreate.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_certificates_package_generation/C_OscXceLoad.cpp

      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_certificates_package_generation/C_OscXceManifest.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_certificates_package_generation/C_OscXceManifestFiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_certificates_package_generation/C_OscXceUpdatePackageParameters.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_certificates_package_generation/C_OscXceBase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_certificates_package_generation/C_OscXceCreate.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_certificates_package_generation/C_OscXceLoad.hpp
   )

   set(OPENSYDE_CORE_X_CERTIFICATES_GENERATION_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_certificates_package_generation
   )
endif()

# optional: x-config generation functionality
if(OPENSYDE_CORE_SKIP_X_CONFIG_GENERATION)
   message("OPENSYDE_CORE_SKIP_X_CONFIG_GENERATION detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_X_CONFIG_GENERATION not detected ... dragging in package")

   set(OPENSYDE_CORE_X_CONFIG_GENERATION_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_config_generation/C_OscXcoBase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_config_generation/C_OscXcoCreate.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_config_generation/C_OscXcoLoad.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_config_generation/C_OscXcoManifest.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_config_generation/C_OscXcoManifestFiler.cpp

      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_config_generation/C_OscXcoBase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_config_generation/C_OscXcoCreate.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_config_generation/C_OscXcoLoad.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_config_generation/C_OscXcoManifest.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_config_generation/C_OscXcoManifestFiler.hpp
   )

   set(OPENSYDE_CORE_X_CONFIG_GENERATION_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/exports/x_config_generation
   )
endif()

# windows IP dispatcher (CAN backend moved to libcan; this only ships the WinSock IP support now)
if(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS)
   message("OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS not detected ... dragging in package")

   set (OPENSYDE_CORE_WINDOWS_DRIVERS_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/ip_dispatcher/target_windows_win_sock/C_OscIpDispatcherWinSock.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ip_dispatcher/target_windows_win_sock/C_OscIpDispatcherWinSock.hpp
   )

   set(OPENSYDE_CORE_WINDOWS_DRIVERS_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/ip_dispatcher/target_windows_win_sock
   )
endif()

# windows target glue layer
if(OPENSYDE_CORE_SKIP_WINDOWS_TARGET)
   message("OPENSYDE_CORE_SKIP_WINDOWS_TARGET detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_WINDOWS_TARGET not detected ... dragging in package")

   set (OPENSYDE_CORE_WINDOWS_TARGET_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/windows/TglFile.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/windows/TglTasks.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/windows/TglTime.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/windows/TglUtils.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/windows/TglFile.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/windows/TglTasks.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/windows/TglTime.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/windows/TglUtils.hpp
   )

   set(OPENSYDE_CORE_WINDOWS_TARGET_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/windows
   )
endif()

# linux IP dispatcher (CAN backend moved to libcan; this only ships the Linux sockets IP support now)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT OPENSYDE_CORE_SKIP_LINUX_DRIVERS)
   message("Linux detected ... dragging in Linux drivers")

   set(OPENSYDE_CORE_LINUX_DRIVERS_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/ip_dispatcher/target_linux_sock/C_OscIpDispatcherLinuxSock.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ip_dispatcher/target_linux_sock/C_OscIpDispatcherLinuxSock.hpp
   )

   set(OPENSYDE_CORE_LINUX_DRIVERS_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/ip_dispatcher
      ${CMAKE_CURRENT_SOURCE_DIR}/ip_dispatcher/target_linux_sock
   )
elseif(OPENSYDE_CORE_SKIP_LINUX_DRIVERS)
   message("OPENSYDE_CORE_SKIP_LINUX_DRIVERS detected ... skipping package")
endif()

# linux target glue layer
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   message("Linux detected ... dragging in Linux target layer")

   set(OPENSYDE_CORE_LINUX_TARGET_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/linux/TglFile.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/linux/TglTasks.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/linux/TglTime.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/linux/TglUtils.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/linux/TglFile.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/linux/TglTasks.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/linux/TglTime.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/linux/TglUtils.hpp
   )

   set(OPENSYDE_CORE_LINUX_TARGET_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/tgl/linux
   )
endif()

# security features like authentication, secure update, secure communication
if(OPENSYDE_CORE_SKIP_SECURITY)
   message("OPENSYDE_CORE_SKIP_SECURITY detected ... skipping package")
else()
   message("OPENSYDE_CORE_SKIP_SECURITY not detected ... dragging in package")

   set (OPENSYDE_CORE_SECURITY_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityPem.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityPemBase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityPemDatabase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityPemKeyInfo.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityPemSecUpdate.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityAesCbc.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityAesFile.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityEcdsa.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityEcdhAes.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityRsa.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/aes/AES.cpp

      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityPem.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityPemBase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityPemDatabase.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityPemKeyInfo.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityPemSecUpdate.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityAesCbc.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityAesFile.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityEcdsa.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityEcdhAes.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/C_OscSecurityRsa.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/security/aes/AES.h
   )

   set(OPENSYDE_CORE_SECURITY_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/security
      ${CMAKE_CURRENT_SOURCE_DIR}/security/aes
   )
endif()

# all sources
set(OPENSYDE_CORE_SOURCES
   ${OPENSYDE_CORE_COMMON_SOURCES}
   ${OPENSYDE_CORE_ZIPPING_SOURCES}
   ${OPENSYDE_CORE_IMPORTS_SOURCES}
   ${OPENSYDE_CORE_CODE_GENERATION_SOURCES}
   ${OPENSYDE_CORE_PROJECT_HANDLING_SOURCES}
   ${OPENSYDE_CORE_PARAM_SET_SOURCES}
   ${OPENSYDE_CORE_PROTOCOL_DRIVERS_BASIC_SOURCES}
   ${OPENSYDE_CORE_SYSTEM_PACKAGE_HANDLING_SOURCES}
   ${OPENSYDE_CORE_X_CERTIFICATES_GENERATION_SOURCES}
   ${OPENSYDE_CORE_X_CONFIG_GENERATION_SOURCES}
   ${OPENSYDE_CORE_PROTOCOL_DRIVERS_SYSTEM_SOURCES}
   ${OPENSYDE_CORE_PROTOCOL_DRIVERS_COMMON_SOURCES}
   ${OPENSYDE_CORE_PROTOCOL_DRIVERS_MONITOR_SOURCES}
   ${OPENSYDE_CORE_WINDOWS_DRIVERS_SOURCES}
   ${OPENSYDE_CORE_WINDOWS_TARGET_SOURCES}
   ${OPENSYDE_CORE_LINUX_DRIVERS_SOURCES}
   ${OPENSYDE_CORE_LINUX_TARGET_SOURCES}
   ${OPENSYDE_CORE_PROTOCOL_LOGGING_SOURCES}
   ${OPENSYDE_CORE_SECURITY_SOURCES}
)

# all include directories
set(OPENSYDE_CORE_INCLUDE_DIRECTORIES
   ${OPENSYDE_CORE_COMMON_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_ZIPPING_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_IMPORTS_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_CODE_GENERATION_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_PROJECT_HANDLING_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_PARAM_SET_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_PROTOCOL_DRIVERS_BASIC_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_SYSTEM_PACKAGE_HANDLING_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_X_CERTIFICATES_GENERATION_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_X_CONFIG_GENERATION_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_PROTOCOL_DRIVERS_SYSTEM_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_PROTOCOL_DRIVERS_COMMON_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_PROTOCOL_DRIVERS_MONITOR_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_WINDOWS_DRIVERS_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_WINDOWS_TARGET_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_LINUX_DRIVERS_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_LINUX_TARGET_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_PROTOCOL_LOGGING_INCLUDE_DIRECTORIES}
   ${OPENSYDE_CORE_SECURITY_INCLUDE_DIRECTORIES}
)

#deactivate warnings for miniz due to external component; but only for GCC as the switch is compiler specific
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   set_source_files_properties(
     ${CMAKE_CURRENT_SOURCE_DIR}/miniz/miniz.c
     PROPERTIES
     COMPILE_FLAGS "-w"
   )
endif()

# define library target including all sources
add_library(opensyde_core ${OPENSYDE_CORE_SOURCES})

# Enable large file support on Linux (required by miniz for 64-bit file I/O)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   target_compile_definitions(opensyde_core PRIVATE _LARGEFILE64_SOURCE)
endif()

# add all include directories to library target and make public
target_include_directories(opensyde_core PUBLIC ${OPENSYDE_CORE_INCLUDE_DIRECTORIES})

target_precompile_headers(opensyde_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/precompiled_headers/precomp_headers.hpp)

# Skip precompiled headers for C source files (the PCH only has C++ content)
file(GLOB_RECURSE OPENSYDE_CORE_C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
set_source_files_properties(${OPENSYDE_CORE_C_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

# Link libcan PUBLIC so consumers of opensyde_core can also use the can:: API directly if they
# want (CAN Monitor's future bus-load/error-counter display, for instance).
if(NOT OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON)
   target_link_libraries(opensyde_core PUBLIC CanLibraries::can)
endif()
