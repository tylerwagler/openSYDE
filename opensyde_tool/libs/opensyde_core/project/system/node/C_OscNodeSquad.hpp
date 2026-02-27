//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       The squad of nodes. In case of multiple CPUs it has all indexes
   of the sub nodes and has its base name

   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODESQUAD_HPP
#define C_OSCNODESQUAD_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "stwtypes.hpp"
#include <QList>
#include <QString>
#include <vector>

#include "C_OscNode.hpp"

/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_core {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_OscNodeSquad {
public:
  C_OscNodeSquad(void);
  virtual ~C_OscNodeSquad(void);

  static QString h_CombineNames(const QString &orc_MainDeviceName,
                                const QString &orc_SubDeviceName);
  int32_t SetBaseName(QList<C_OscNode> &orc_Nodes,
                      const QString &orc_NodeBaseName);
  static bool h_CheckIsMultiDevice(
      const uint32_t ou32_NodeIndex,
      const QList<stw::opensyde_core::C_OscNodeSquad> &orc_AvailableGroups,
      uint32_t *const opu32_GroupIndex = NULL);

  static const QString
      hc_SEPARATOR;   ///< Default separator for GUI and device type checks
  QString c_BaseName; ///< Base name for all sub nodes
  QList<uint32_t>
      c_SubNodeIndexes; ///< Indexes of all containing sub nodes in the squad
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
