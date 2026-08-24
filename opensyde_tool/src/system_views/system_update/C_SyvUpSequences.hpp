//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       GUI Integration class for system update sequences (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYVSUSEQUENCES_HPP
#define C_SYVSUSEQUENCES_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QObject>
#include <QMutex>

#include "C_CanDispatcher.hpp"
#include "C_OscSuSequences.hpp"
#include "C_SyvComDriverThread.hpp"
#include "C_OscIpDispatcherPlatform.hpp"
#include "C_OscSecurityPemDatabase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SyvUpSequences :
   public QObject,
   public stw::opensyde_core::C_OscSuSequences
{
   Q_OBJECT

public:
   C_SyvUpSequences(void);
   ~C_SyvUpSequences(void) override;

   int32_t InitUpSequences(const uint32_t ou32_ViewIndex);
   int32_t ReinitDispatcher(void);
   void CloseDispatcher(void);
   int32_t SyvUpCreateTemporaryFolder(const std::string & orc_TargetPath,
                                      std::vector<C_DoFlash> & orc_ApplicationsToWrite, QString & orc_ErrorPath) const;

   int32_t StartActivateFlashloader(void);
   int32_t StartReadDeviceInformation(void);
   int32_t StartUpdateSystem(const std::vector<C_DoFlash> & orc_ApplicationsToWrite,
                             const std::vector<uint32_t> & orc_NodesOrder);
   int32_t StartResetSystem(void);

   int32_t GetResults(int32_t & ors32_Result) const;
   int32_t GetLastUpdatePosition(uint32_t & oru32_NodeIndex, uint32_t & oru32_FileIndex) const;

   void GetOsyDeviceInformation(std::vector<uint32_t> & orc_OsyNodeIndexes,
                                std::vector<C_OsyDeviceInformation> & orc_OsyDeviceInformation);
   QString GetStepName(const E_ProgressStep oe_Step) const;

   void AbortCurrentProgress(void);

   enum E_Sequence
   {
      eNOT_ACTIVE,
      eACTIVATE_FLASHLOADER,
      eREAD_DEVICE_INFORMATION,
      eUPDATE_SYSTEM,
      eRESET_SYSTEM
   };

   int32_t GetConnectStates(std::vector<stw::opensyde_core::C_OscSuSequencesNodeConnectStates> & orc_ConnectStatesNodes)
   const override;
   int32_t GetUpdateStates(std::vector<stw::opensyde_core::C_OscSuSequencesNodeUpdateStates> & orc_UpdateStatesNodes)
   const override;

   //The signals keyword is necessary for Qt signal slot functionality
   //lint -save -e1736

Q_SIGNALS:
   //lint -restore
   void SigReportProgress(const uint32_t ou32_Step, const int32_t os32_Result, const uint8_t ou8_Progress) const;

   void SigReportProgressForServer(const uint32_t ou32_Step, const int32_t os32_Result, const uint8_t ou8_Progress,
                                   const uint8_t ou8_BusIdentifier, const uint8_t ou8_NodeIdentifier) const;

   void SigReportOpenSydeFlashloaderInformationRead(void);

protected:
   bool m_ReportProgress(const E_ProgressStep oe_Step, const int32_t os32_Result, const uint8_t ou8_Progress,
                         const std::string & orc_Information) override;
   bool m_ReportProgress(const E_ProgressStep oe_Step, const int32_t os32_Result, const uint8_t ou8_Progress,
                         const stw::opensyde_core::C_OscProtocolDriverOsyNode & orc_Server,
                         const std::string & orc_Information) override;
   void m_ReportOpenSydeFlashloaderInformationRead(const C_OsyDeviceInformation & orc_Info,
                                                   const uint32_t ou32_NodeIndex) override;

private:
   //Avoid call
   C_SyvUpSequences(const C_SyvUpSequences &);
   C_SyvUpSequences & operator =(const C_SyvUpSequences &) &; //lint !e1511 //we want to hide the base func.

   static void mh_ThreadFunc(void * const opv_Instance);
   void m_ThreadFunc(void);

   static void mh_WriteLog(const C_OscSuSequences::E_ProgressStep oe_Step, const std::string & orc_Text);

   C_SyvComDriverThread * mpc_Thread;
   QMutex mc_Lock;
   stw::can::C_CanDispatcher * mpc_CanDispatcher;
   stw::opensyde_core::C_OscIpDispatcherWinSock * mpc_EthernetDispatcher;
   bool mq_AbortFlag;
   uint32_t mu32_ViewIndex;

   // Sequence execution parameter
   E_Sequence me_Sequence;
   // Input parameter for sequence
   std::vector<C_DoFlash> mc_NodesToFlash;
   std::vector<uint32_t> mc_NodesOrder;

   // Service execution result
   int32_t ms32_Result;

   //results of openSYDE device information scans:
   std::vector<uint32_t> mc_ReportOsyDeviceInformationNodeIndex;
   std::vector<C_OsyDeviceInformation> mc_ReportOsyDeviceInformation;

   // Security PEM database
   stw::opensyde_core::C_OscSecurityPemDatabase mc_PemDatabase;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
