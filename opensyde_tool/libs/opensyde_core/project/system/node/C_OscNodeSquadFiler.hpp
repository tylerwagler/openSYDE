//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       NodeSquad reader/writer
   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODESQUADFILER_HPP
#define C_OSCNODESQUADFILER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscNodeSquad.hpp"
#include "C_OscXmlParser.hpp"
#include <QList>

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

class C_OscNodeSquadFiler {
public:
  C_OscNodeSquadFiler();

  static int32_t h_LoadNodeGroups(QList<C_OscNodeSquad> &orc_NodeGroups,
                                  C_OscXmlParserBase &orc_XmlParser);
  static int32_t h_LoadNodeGroup(C_OscNodeSquad &orc_NodeGroup,
                                 C_OscXmlParserBase &orc_XmlParser);
  static void h_SaveNodeGroups(const QList<C_OscNodeSquad> &orc_NodeGroups,
                               C_OscXmlParserBase &orc_XmlParser);
  static void h_SaveNodeGroup(const C_OscNodeSquad &orc_NodeGroup,
                              C_OscXmlParserBase &orc_XmlParser);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
