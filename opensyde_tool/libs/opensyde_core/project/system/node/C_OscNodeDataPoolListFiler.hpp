//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscNodeDataPoolList (header)

   Composes the ListElement and DataSet filers.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODEDATAPOOLLISTFILER_HPP
#define C_OSCNODEDATAPOOLLISTFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscNodeDataPoolList.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscNodeDataPoolListFiler
{
public:
   static QJsonObject save(const C_OscNodeDataPoolList & orc_List);
   static int32_t load(const QJsonObject & orc_Json, C_OscNodeDataPoolList & orc_List);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCNODEDATAPOOLLISTFILER_HPP
