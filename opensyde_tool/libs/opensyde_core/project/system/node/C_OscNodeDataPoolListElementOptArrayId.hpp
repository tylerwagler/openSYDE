//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Store all indices to identify a data element stored in a node including an optional array
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODEDATAPOOLLISTELEMENTOPTARRAYID_HPP
#define C_OSCNODEDATAPOOLLISTELEMENTOPTARRAYID_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscNodeDataPoolListElementId.hpp"
#include <QString>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscNodeDataPoolListElementOptArrayId :
   public C_OscNodeDataPoolListElementId
{
public:
   C_OscNodeDataPoolListElementOptArrayId();
   C_OscNodeDataPoolListElementOptArrayId(const uint32_t ou32_NodeIndex, const uint32_t ou32_DataPoolIndex,
                                          const uint32_t ou32_ListIndex, const uint32_t ou32_ElementIndex,
                                          const bool oq_UseArrayElementIndex = false,
                                          const uint32_t ou32_ArrayElementIndex = 0UL);
   C_OscNodeDataPoolListElementOptArrayId(const C_OscNodeDataPoolListElementId & orc_Base,
                                          const bool oq_UseArrayElementIndex = false,
                                          const uint32_t ou32_ArrayElementIndex = 0UL);

   virtual bool operator < (const C_OscNodeDataPoolId & orc_Cmp) const;
   virtual bool operator == (const C_OscNodeDataPoolId & orc_Cmp) const;

   virtual void CalcHash(uint32_t & oru32_HashValue) const;

   QString GetHalChannelName(void) const;
   void SetHalChannelName(const QString & orc_Value);

   uint32_t GetArrayElementIndex(void) const;
   uint32_t GetArrayElementIndexOrZero(void) const;

   bool GetUseArrayElementIndex(void) const;

private:
   bool mq_UseArrayElementIndex;
   uint32_t mu32_ArrayElementIndex;
   QString mc_HalChannelName;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
