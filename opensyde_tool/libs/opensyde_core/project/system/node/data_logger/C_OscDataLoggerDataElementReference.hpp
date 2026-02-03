//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data logger configured data element information
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDATALOGGERDATAELEMENTREFERENCE_HPP
#define C_OSCDATALOGGERDATAELEMENTREFERENCE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include "C_OscNodeDataPoolListElementOptArrayId.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscDataLoggerDataElementReference
{
public:
   C_OscDataLoggerDataElementReference();

   void CalcHash(uint32_t & oru32_HashValue) const;

   bool q_UseCustomName;                                         ///< flag to use custom name or default name
   QString c_CustomName;                                         ///< custom name to use for logging, do not use if
                                                                 // q_UseCustomName is not set
   C_OscNodeDataPoolListElementOptArrayId c_ConfiguredElementId; ///< configured element ID

   bool GetUseCustomName(void) const { return this->q_UseCustomName; }
   const QString & GetCustomName(void) const { return this->c_CustomName; }
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
