//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscNodeDataPoolDataSet (header)

   Trivial leaf filer for the named/commented data set descriptor.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODEDATAPOOLDATASETFILER_HPP
#define C_OSCNODEDATAPOOLDATASETFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscNodeDataPoolDataSet.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscNodeDataPoolDataSetFiler
{
public:
   static QJsonObject save(const C_OscNodeDataPoolDataSet & orc_DataSet);
   static int32_t load(const QJsonObject & orc_Json, C_OscNodeDataPoolDataSet & orc_DataSet);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCNODEDATAPOOLDATASETFILER_HPP
