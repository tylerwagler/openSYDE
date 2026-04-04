//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node update package definition filer
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscSupNodeDefinitionFiler.hpp"
#include "C_OscXmlParser.hpp"
#include "C_OscUtils.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"
#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDomDocument>
#include <QDomElement>

using namespace stw::opensyde_core;
using namespace stw::errors;

const uint8_t C_OscSupNodeDefinitionFiler::hu8_ACTIVE_NODE = 1U;

// Stub implementation - TODO: Complete implementation
int32_t C_OscSupNodeDefinitionFiler::h_SaveNodes(const QStringList &orc_Files,
                                                 const QList<C_OscSupNodeDefinition> &orc_Nodes) {
    Q_UNUSED(orc_Files);
    Q_UNUSED(orc_Nodes);
    return C_NO_ERR;
}

int32_t C_OscSupNodeDefinitionFiler::h_LoadNodes(const QStringList &orc_Files,
                                                 const QStringList &orc_NodeFoldersAbs,
                                                 const QByteArray &orc_ActiveNodes,
                                                 QList<C_OscSuSequences::C_DoFlash> &orc_ApplicationsToWrite,
                                                 QMap<uint32_t, uint32_t> &orc_UpdateOrderByNodes,
                                                 const QList<uint32_t> &orc_UpdatePosition,
                                                 QStringList &orc_Signatures) {
    Q_UNUSED(orc_Files);
    Q_UNUSED(orc_NodeFoldersAbs);
    Q_UNUSED(orc_ActiveNodes);
    Q_UNUSED(orc_ApplicationsToWrite);
    Q_UNUSED(orc_UpdateOrderByNodes);
    Q_UNUSED(orc_UpdatePosition);
    Q_UNUSED(orc_Signatures);
    return C_NO_ERR;
}
