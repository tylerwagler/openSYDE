//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for node data
   \copyright   Copyright Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "C_OscNodeFiler.hpp"
#include "C_OscXmlParser.hpp"
#include "stwerrors.hpp"

using namespace stw::opensyde_core;
using namespace stw::errors;

int32_t C_OscNodeFiler::h_LoadNodeFile(C_OscNode & orc_Node,
                                       const QString & orc_FilePath,
                                       const bool oq_CheckNodeName) {
    Q_UNUSED(orc_Node);
    Q_UNUSED(orc_FilePath);
    Q_UNUSED(oq_CheckNodeName);
    return C_NO_ERR;
}

int32_t C_OscNodeFiler::h_SaveNodeFile(const C_OscNode & orc_Node,
                                       const QString & orc_FilePath,
                                       QStringList * const opc_CreatedFiles) {
    Q_UNUSED(orc_Node);
    Q_UNUSED(orc_FilePath);
    Q_UNUSED(opc_CreatedFiles);
    return C_NO_ERR;
}

QString C_OscNodeFiler::h_GetFileName(void) {
    return "node.xml";
}
