//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Dialog for building UDS (ISO 14229) diagnostic requests

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CAMUDSREQUESTBUILDERDIALOG_HPP
#define C_CAMUDSREQUESTBUILDERDIALOG_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QByteArray>
#include <QVector>
#include <QGroupBox>
#include <QStackedWidget>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui
{

class C_CamUdsRequestBuilderDialog :
   public QDialog
{
   Q_OBJECT

public:
   explicit C_CamUdsRequestBuilderDialog(QWidget * const opc_Parent = nullptr);
   ~C_CamUdsRequestBuilderDialog(void) override;

   QByteArray GetGeneratedBytes(void) const;
   uint32_t   GetCanId(void) const;
   bool       GetIsExtended(void) const;
   QString    GetMessageName(void) const;

private:
   // Service selection
   QComboBox * mpc_ComboBoxService;

   // Parameter fields (shown/hidden per service)
   QGroupBox *    mpc_GroupParams;
   QStackedWidget * mpc_StackFields;

   // Page 0: sub-function only
   QWidget *    mpc_PageSubFunc;
   QSpinBox *   mpc_SpinSubFunc;
   QLabel *     mpc_LabelSubFunc;

   // Page 1: sub-function + DID
   QWidget *    mpc_PageSubFuncDid;
   QSpinBox *   mpc_SpinSubFuncDid;
   QLabel *     mpc_LabelSubFuncDid;
   QSpinBox *   mpc_SpinDid;
   QLabel *     mpc_LabelDid;

   // Page 2: DID only
   QWidget *    mpc_PageDid;
   QSpinBox *   mpc_SpinDidOnly;
   QLabel *     mpc_LabelDidOnly;

   // Page 3: DID + data
   QWidget *    mpc_PageDidData;
   QSpinBox *   mpc_SpinDidData;
   QLabel *     mpc_LabelDidData;
   QLineEdit *  mpc_EditDidData;
   QLabel *     mpc_LabelDidDataBytes;

   // Page 4: sub-function + data
   QWidget *    mpc_PageSubFuncData;
   QSpinBox *   mpc_SpinSubFuncData;
   QLabel *     mpc_LabelSubFuncData;
   QLineEdit *  mpc_EditSubFuncData;
   QLabel *     mpc_LabelSubFuncDataBytes;

   // Page 5: sub-function + RID + data
   QWidget *    mpc_PageRoutine;
   QSpinBox *   mpc_SpinRoutineSubFunc;
   QLabel *     mpc_LabelRoutineSubFunc;
   QSpinBox *   mpc_SpinRid;
   QLabel *     mpc_LabelRid;
   QLineEdit *  mpc_EditRoutineData;
   QLabel *     mpc_LabelRoutineData;

   // Page 6: no params
   QWidget *    mpc_PageNoParams;

   // Page 7: data only
   QWidget *    mpc_PageDataOnly;
   QLineEdit *  mpc_EditDataOnly;
   QLabel *     mpc_LabelDataOnly;

   // CAN ID
   QLineEdit *  mpc_EditCanId;

   // Preview
   QLabel *     mpc_LabelPreview;

   // Buttons
   QPushButton * mpc_BtnGenerate;
   QPushButton * mpc_BtnSend;
   QPushButton * mpc_BtnCancel;

   QByteArray mc_GeneratedBytes;

   void m_SetupUi(void);
   void m_UpdatePreview(void);
   void m_GenerateRequest(void);
   void m_OnServiceChanged(int32_t os32_Index);

   static QString mh_ServiceName(uint8_t ou8_Sid);
};

}
} //end of namespace

#endif
