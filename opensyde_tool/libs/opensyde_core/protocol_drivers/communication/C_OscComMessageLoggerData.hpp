//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for logging CAN messages (header)

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCOMMESSAGELOGGERDATA_HPP
#define C_OSCCOMMESSAGELOGGERDATA_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <algorithm>
#include <QList>

#include <QString>
#include "stwtypes.hpp"
#include "stw_can.hpp"
#include "C_OscCanSignal.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscComMessageLoggerDataSignal
{
public:
   C_OscComMessageLoggerDataSignal(void);

   bool operator <(const C_OscComMessageLoggerDataSignal & orc_Cmp) const;

   QString c_Name;
   QString c_Value;
   QString c_Unit;
   QString c_RawValueDec;
   QString c_RawValueHex;
   QString c_Comment;
   bool q_DlcError;

   stw::opensyde_core::C_OscCanSignal c_OscSignal;
};

class C_OscComMessageLoggerData
{
public:
   C_OscComMessageLoggerData(void);
   virtual ~C_OscComMessageLoggerData(void);

   void SortSignals(void);
   static QString h_GetTimestampAsString(const uint64_t ou64_TimeStamp);

   uint64_t u64_TimeStampRelative;
   uint64_t u64_TimeStampAbsoluteStart;
   uint64_t u64_TimeStampAbsoluteTimeOfDay;
   QString c_TimeStampRelative;
   QString c_TimeStampAbsoluteStart;
   QString c_TimeStampAbsoluteTimeOfDay;
   QString c_CanIdDec;
   QString c_CanIdHex;
   QString c_CanDlc;
   bool q_CanDlcError;
   QString c_CanDataDec;
   QString c_CanDataHex;
   QString c_Name;
   QString c_ProtocolTextDec;
   QString c_ProtocolTextHex;
   QString c_Counter;
   QString c_Status;

   QList<C_OscComMessageLoggerDataSignal> c_Signals;

   stw::can::T_STWCAN_Msg_RX c_CanMsg;
   bool q_IsTx;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
