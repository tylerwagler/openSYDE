//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       View reader/writer (Multi-Format)

   Load / save view data from / to binary, JSON, or XML files using the
   Qt-native serialization framework.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCVIEWFILER_NEW_HPP
#define C_OSCVIEWFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscViewData.hpp"
#include "C_OscFilerHelper.hpp"
#include "C_OscNode.hpp"
#include "stwtypes.hpp"
#include <QList>
#include <QString>

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

class C_OscViewFiler_New {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadViewFile(C_OscViewData &orc_View, const QString &orc_FilePath,
                                 const QList<C_OscNode> &orc_OscNodes);
   static int32_t h_SaveViewFile(const C_OscViewData &orc_View, const QString &orc_FilePath,
                                 const QList<C_OscNode> &orc_OscNodes);

   // --------------------------------------------------------------------------
   // Binary Format (Fastest, compact)
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(C_OscViewData &orc_View, const QString &orc_Path,
                               const QList<C_OscNode> &orc_OscNodes);
   static int32_t h_SaveBinary(const C_OscViewData &orc_View, const QString &orc_Path,
                               const QList<C_OscNode> &orc_OscNodes);
   static int32_t h_LoadFromMemoryBinary(C_OscViewData &orc_View, const QByteArray &orc_Data);
   static QByteArray h_SaveToMemoryBinary(const C_OscViewData &orc_View);

   // --------------------------------------------------------------------------
   // JSON Format (Human-readable, debugging)
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(C_OscViewData &orc_View, const QString &orc_Path,
                             const QList<C_OscNode> &orc_OscNodes);
   static int32_t h_SaveJson(const C_OscViewData &orc_View, const QString &orc_Path,
                             const QList<C_OscNode> &orc_OscNodes);
   static int32_t h_LoadFromMemoryJson(C_OscViewData &orc_View, const QJsonObject &orc_Object);
   static QJsonObject h_SaveToMemoryJson(const C_OscViewData &orc_View);

   // --------------------------------------------------------------------------
   // XML Format (Legacy compatibility)
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(C_OscViewData &orc_View, const QString &orc_Path,
                            const QList<C_OscNode> &orc_OscNodes);
   static int32_t h_SaveXml(const C_OscViewData &orc_View, const QString &orc_Path,
                            const QList<C_OscNode> &orc_OscNodes);
   static int32_t h_LoadFromMemoryXml(C_OscViewData &orc_View, const QDomElement &orc_Element,
                                      const QList<C_OscNode> &orc_OscNodes);
   static QDomElement h_SaveToMemoryXml(const C_OscViewData &orc_View, QDomDocument &orc_Doc);

   // --------------------------------------------------------------------------
   // Legacy Compatibility (deprecated)
   // --------------------------------------------------------------------------
   [[deprecated("Use h_LoadViewFile/h_SaveViewFile with format detection")]]
   static int32_t h_LoadViewFileOsc(C_OscViewData &orc_View, const QString &orc_FilePath,
                                    const QList<C_OscNode> &orc_OscNodes);
   [[deprecated("Use h_LoadViewFile/h_SaveViewFile with format detection")]]
   static void h_SaveViewFileOsc(const C_OscViewData &orc_View, const QString &orc_FilePath,
                                 const QList<C_OscNode> &orc_OscNodes);

private:
   static int32_t mh_DetectAndLoad(C_OscViewData &orc_View, const QString &orc_Path,
                                   const QList<C_OscNode> &orc_OscNodes);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
