//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       System view data element (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCVIEWDATA_HPP
#define C_OSCVIEWDATA_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>

#include "C_OscViewPc.hpp"
#include "C_OscViewNodeUpdate.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscViewData
{
public:
   enum E_DeviceConfigurationMode
   {
      eDCM_ALL_CONNECTED_INTERFACES,
      eDCM_ONLY_USED_INTERFACES
   };

   C_OscViewData(void);
   virtual ~C_OscViewData(void);

   virtual void CalcHash(uint32_t & oru32_HashValue) const;

   const C_OscViewPc & GetOscPcData(void) const;
   void SetOscPcData(const C_OscViewPc & orc_Value);
   bool GetNodeActive(const uint32_t ou32_NodeIndex) const;
   const std::vector<uint8_t> & GetNodeActiveFlags(void) const;
   void SetNodeActiveFlags(const std::vector<uint8_t> & orc_Value);
   const std::vector<C_OscViewNodeUpdate> & GetAllNodeUpdateInformation(void) const;
   void SetNodeUpdateInformation(const std::vector<C_OscViewNodeUpdate> & orc_NodeUpdateInformation);
   const C_OscViewNodeUpdate * GetNodeUpdateInformation(const uint32_t ou32_NodeIndex) const;
   std::error_code SetNodeUpdateInformation(const uint32_t ou32_NodeIndex,
                                            const C_OscViewNodeUpdate & orc_NodeUpdateInformation);
   std::error_code SetNodeUpdateInformationPath(const uint32_t ou32_NodeIndex, const uint32_t ou32_Index,
                                                const std::string & orc_Value,
                                                const C_OscViewNodeUpdate::E_GenericFileType oe_Type);
   std::error_code SetNodeUpdateInformationParamInfo(const uint32_t ou32_NodeIndex, const uint32_t ou32_Index,
                                                     const C_OscViewNodeUpdateParamInfo & orc_Value);
   std::error_code SetNodeUpdateInformationPemFilePath(const uint32_t ou32_NodeIndex, const std::string & orc_Value);
   std::error_code SetNodeUpdateInformationSkipUpdateOfPath(const uint32_t ou32_NodeIndex, const uint32_t ou32_Index,
                                                            const bool oq_SkipFile,
                                                            const C_OscViewNodeUpdate::E_GenericFileType oe_Type);
   std::error_code SetNodeUpdateInformationSkipUpdateOfParamInfo(const uint32_t ou32_NodeIndex,
                                                                 const uint32_t ou32_Index, const bool oq_SkipFile);
   std::error_code SetNodeUpdateInformationSkipUpdateOfPemFile(const uint32_t ou32_NodeIndex, const bool oq_SkipFile);
   std::error_code SetNodeUpdateInformationStates(const uint32_t ou32_NodeIndex,
                                                  const C_OscViewNodeUpdate::E_StateSecureAuthentication oe_StateAuthentication,
                                                  const C_OscViewNodeUpdate::E_StateDebugger oe_StateDebugger,
                                                  const C_OscViewNodeUpdate::E_StateTrafficEncryption oe_StateTrafficEncryption);
   std::error_code SetNodeUpdateInformationParamInfoContent(const uint32_t ou32_NodeIndex, const uint32_t ou32_Index,
                                                            const std::string & orc_FilePath,
                                                            const uint32_t ou32_LastKnownCrc);
   std::error_code AddNodeUpdateInformationPath(const uint32_t ou32_NodeIndex, const std::string & orc_Value,
                                                const C_OscViewNodeUpdate::E_GenericFileType oe_Type);
   std::error_code AddNodeUpdateInformationParamInfo(const uint32_t ou32_NodeIndex,
                                                     const C_OscViewNodeUpdateParamInfo & orc_Value);
   const std::string & GetName(void) const;
   void SetName(const std::string & orc_Value);
   void SetPcConnected(const bool oq_Connected, const uint32_t ou32_BusIndex);

   //Sync to system definition
   void OnSyncBusAdded(const uint32_t ou32_Index);
   void OnSyncBusDeleted(const uint32_t ou32_Index);

   //Delete
   std::error_code RemoveNodeUpdateInformationPath(const uint32_t ou32_NodeIndex, const uint32_t ou32_Index,
                                                   const C_OscViewNodeUpdate::E_GenericFileType oe_Type);
   std::error_code RemoveNodeUpdateInformationParamInfo(const uint32_t ou32_NodeIndex, const uint32_t ou32_Index);
   std::error_code RemoveNodeUpdateInformationPemFilePath(const uint32_t ou32_NodeIndex);
   std::error_code ClearNodeUpdateInformationAsAppropriate(const uint32_t ou32_NodeIndex,
                                                           const C_OscViewNodeUpdate::E_GenericFileType oe_Type);
   std::error_code ClearNodeUpdateInformationParamPaths(const uint32_t ou32_NodeIndex);

protected:
   //Protected access for tests and inheritance
   std::vector<uint8_t> mc_NodeActiveFlags; ///< Vector of usage flags.
   ///< Equal to system definition nodes count.
   ///< True: Node used in system view
   ///< False: Node not used in system view
   std::vector<stw::opensyde_core::C_OscViewNodeUpdate> mc_NodeUpdateInformation;
   ///< Vector of node update information.
   ///< Equal to system definition nodes count.

private:
   std::string mc_Name;             ///< System view name
   stw::opensyde_core::C_OscViewPc mc_PcData; ///< Data for PC element
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
