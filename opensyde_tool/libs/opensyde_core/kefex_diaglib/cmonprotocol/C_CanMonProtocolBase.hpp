//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Translate L2 CAN message to textual interpretation

   \copyright   Copyright 2002 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CCMONPROTOCOLBASEHPP
#define CCMONPROTOCOLBASEHPP

#include "stwtypes.hpp"
#include "C_CanMonProtocolTarget.hpp"
#include <QString>
#include <QSettings>

//----------------------------------------------------------------------------------------------------------------------

namespace stw
{
namespace cmon_protocol
{
//----------------------------------------------------------------------------------------------------------------------

//Base class for all protocol interpretations
//Provides basic functions to help converting a CAN message structure into text.
//Also provides an abstract interface for protocol-specific conversions.
class C_CanMonProtocolBase
{
protected:
   bool mq_Decimal;

   //general:
   QString m_GetValueDecHex(const uint32_t ou32_Value) const;
   QString m_GetByteAsStringFormat(const uint8_t ou8_Value) const;
   QString m_GetWordAsStringFormat(const uint16_t ou16_Value) const;
   static uint32_t mh_BytesToDwordLowHigh(const uint8_t oau8_Bytes[4]);
   static uint16_t mh_BytesToWordLowHigh(const uint8_t oau8_Bytes[2]);
   static uint16_t mh_BytesToWordHighLow(const uint8_t oau8_Bytes[2]);
   static uint32_t mh_BytesToDwordHighLow(const uint8_t oau8_Bytes[4]);
   static QString mh_SerialNumberToString(const uint8_t * const opu8_SerialNumber);

public:
   C_CanMonProtocolBase(void);
   virtual ~C_CanMonProtocolBase(void);

   //-----------------------------------------------------------------------------
   /*!
      \brief    Return CAN message in string interpretation

      Actual conversion routine to be overloaded by inheriting class

      \param[in]       orc_Msg    CAN message to convert to string

      \return
      CAN message in string representation
   */
   //-----------------------------------------------------------------------------
   virtual QString MessageToString(const stw::can::T_STWCAN_Msg_RX & orc_Msg) const = 0;

   //-----------------------------------------------------------------------------
   /*!
      \brief    Return string representation of protocol name

      To be overloaded by inheriting class.

      \return
      string representation of protocol name
   */
   //-----------------------------------------------------------------------------
   virtual QString GetProtocolName(void) const = 0;

   virtual void SetDecimal(const bool oq_Decimal);

   //overload if we have something to save (use protocol abbreviation as prefix for directives !)
   virtual int32_t SaveParamsToIni(QSettings & orc_IniFile, const QString & orc_Section);
   virtual int32_t LoadParamsFromIni(QSettings & orc_IniFile, const QString & orc_Section);
};

//----------------------------------------------------------------------------------------------------------------------
//Common base for KEFEX-style variable-based protocols
///Adds functionality to link in an opened KEFEX project in order to decode variable indexes from CAN messages.
class C_CanMonProtocolKefexIva :
   public C_CanMonProtocolBase
{
private:
   C_CanMonProtocolKefexIva(const C_CanMonProtocolKefexIva & orc_Source); //not implemented -> prevent
   // copying
   C_CanMonProtocolKefexIva & operator = (const C_CanMonProtocolKefexIva & orc_Source); //not implemented -> prevent
   // assignment

protected:
   uint16_t mu16_KfxListOffset;
#ifdef CMONPROTOCOL_ALLOW_RAMVIEW_PROJECT_MAPPING
   const stw::diag_lib::C_KFXVariableLists * mpc_KfxLists;
#endif

   //used by KFX and IVA:
   QString m_KfxIndexToString(const uint16_t ou16_Index, bool oq_IsKefexVarIndex = true) const;
   QString m_KfxTextAndIndexToString(const char_t * const opcn_Text, const uint16_t ou16_Index) const;

public:
   C_CanMonProtocolKefexIva(void);
#ifdef CMONPROTOCOL_ALLOW_RAMVIEW_PROJECT_MAPPING
   void SetVariableInfo(const stw::diag_lib::C_KFXVariableLists * const opc_Lists, const uint16_t ou16_ListOffset);
#endif
};

//----------------------------------------------------------------------------------------------------------------------
}
}

#endif
