//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Dialog to edit the additional trigger condition
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDNDEDALLOGJOBADDITIONALTRIGGERDIALOG_HPP
#define C_SDNDEDALLOGJOBADDITIONALTRIGGERDIALOG_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OgePopUpContentBase.hpp"
#include "C_OscDataLoggerDataElementReference.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_SdNdeDalLogJobAdditionalTriggerDialog;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdNdeDalLogJobAdditionalTriggerDialog :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_SdNdeDalLogJobAdditionalTriggerDialog(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
                                                    const QString & orc_LogJobName);
   ~C_SdNdeDalLogJobAdditionalTriggerDialog(void) override;

   void InitStaticNames(void) const;
   void SetNodeDataLoggerJob(const uint32_t ou32_NodeIndex, const uint32_t ou32_DataLoggerJobIndex);


protected:
   void m_OnEnterAccept(void) override;

private:
   void m_CancelClicked(void);
   void m_OkClicked(void);
   void m_AddDataElementClicked(void);
   void m_ValidateTriggerCondition(void);
   void m_GetAdditionalTriggerCondition(void);
   bool m_SaveTriggerCondition(void);

   Ui::C_SdNdeDalLogJobAdditionalTriggerDialog * mpc_Ui;
   QString mc_LogJobName;
   QString mc_TriggerCondition;

   uint32_t mu32_NodeIndex;
   uint32_t mu32_DataLoggerJobIndex;
   bool mq_TriggerConditionValid;

   //Avoid call
   C_SdNdeDalLogJobAdditionalTriggerDialog(const C_SdNdeDalLogJobAdditionalTriggerDialog &);
   C_SdNdeDalLogJobAdditionalTriggerDialog & operator =(const C_SdNdeDalLogJobAdditionalTriggerDialog &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
