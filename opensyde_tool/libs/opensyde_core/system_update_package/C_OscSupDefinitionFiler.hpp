//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       System update package definition filer
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSUPDEFINITIONFILER_HPP
#define C_OSCSUPDEFINITIONFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <map>

#include <QList>
#include <QString>
#include <QStringList>
#include "C_OscXmlParser.hpp"
#include "C_OscSuSequences.hpp"
#include "C_OscSupDefinition.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscSupDefinitionFiler
{
public:
   static int32_t h_CreateUpdatePackageDefFile(const QString & orc_Path,
                                               const C_OscSupDefinition & orc_SupDefContent,
                                               const QStringList & orc_Files);
   static int32_t h_LoadUpdatePackageDefFile(const QString & orc_TargetUnzipPath, const bool oq_IsZip,
                                             const QString & orc_PackagePath,
                                             uint32_t & oru32_FileVersion, QString & orc_FilePackagePath,
                                             uint32_t & oru32_ActiveBusIndex, QByteArray & orc_ActiveNodes,
                                             QList<uint32_t> & orc_UpdatePosition,
                                             QStringList & orc_PackageFiles);

   static const QString hc_PACKAGE_UPDATE_DEF;

private:
   static void mh_SaveNodes(C_OscXmlParserBase & orc_XmlParser, const QList<C_OscSupNodeDefinition> & orc_Nodes,
                            const QStringList & orc_Files);
   static void mh_LoadNodes(C_OscXmlParserBase & orc_XmlParser, QByteArray & orc_ActiveNodes,
                            QList<uint32_t> & orc_UpdatePosition,
                            QStringList & orc_PackageFiles);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
