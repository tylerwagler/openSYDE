//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       UI node: stores additional UI information (implementation)

   UI node: stores additional UI information

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_PuiSdNode.hpp"
#include "C_OscHashUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::opensyde_core;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Initialize default values
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiSdNodeConnection::C_PuiSdNodeConnection()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue    Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiSdNodeConnection::CalcHash(uint32_t & oru32_HashValue) const
{
   stw::opensyde_core::hash_util::CalcHashMembers(oru32_HashValue,
                                                   this->c_ConnectionId);

   for (uint32_t u32_Counter = 0U; u32_Counter < this->c_UiNodeConnectionInteractionPoints.size(); ++u32_Counter)
   {
      //lint -e{9110} //we do not really use the bit representation; we just assume it is "stable" for this type
      stw::opensyde_core::hash_util::CalcHashMembers(oru32_HashValue,
                                                      this->c_UiNodeConnectionInteractionPoints[u32_Counter].x(),
                                                      this->c_UiNodeConnectionInteractionPoints[u32_Counter].y());
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Initialize default values
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiSdNode::C_PuiSdNode() :
   C_PuiBsBox()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue    Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiSdNode::CalcHash(uint32_t & oru32_HashValue) const
{
   stw::opensyde_core::hash_util::CalcHashMembers(oru32_HashValue,
                                                   this->c_UiBusConnections,
                                                   this->c_UiDataPools,
                                                   this->c_UiCanProtocols);

   C_PuiBsBox::CalcHash(oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Delete specified connection

   \param[in] orc_ConnectionId ID of connection to delete
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiSdNode::DeleteConnection(const C_PuiSdNodeConnectionId & orc_ConnectionId)
{
   for (uint32_t u32_Index = 0; u32_Index < this->c_UiBusConnections.size(); ++u32_Index)
   {
      const C_PuiSdNodeConnection & rc_BusConnection = this->c_UiBusConnections[u32_Index];
      if (rc_BusConnection.c_ConnectionId == orc_ConnectionId)
      {
         this->c_UiBusConnections.erase(this->c_UiBusConnections.begin() + u32_Index);
         break;
      }
   }
}
