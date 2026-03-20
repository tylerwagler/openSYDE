 //----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle project save and load with multi-format support

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
 //----------------------------------------------------------------------------------------------------------------------
 #ifndef C_OSCPROJECTFILER_NEW_HPP
 #define C_OSCPROJECTFILER_NEW_HPP

 /* -- Includes
  * ------------------------------------------------------------------------------------------------------
  */
 #include "C_OscProject.hpp"
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

 class C_OscProjectFiler_New {
 public:
     // --------------------------------------------------------------------------
     // Public Static Methods - Multi-format file operations
     // --------------------------------------------------------------------------
     static int32_t h_Save(C_OscProject &orc_Project, const QString &orc_Path,
                           const QString &orc_OpenSydeVersion);
     static int32_t h_Load(C_OscProject &orc_Project, const QString &orc_Path);

     // --------------------------------------------------------------------------
     // Format-specific loading methods
     // --------------------------------------------------------------------------
     static int32_t h_LoadBinary(C_OscProject &orc_Project, QDataStream &orc_Stream);
     static int32_t h_LoadJson(C_OscProject &orc_Project, const QJsonObject &orc_Object);
     static int32_t h_LoadXml(C_OscProject &orc_Project, C_OscXmlParserBase &orc_XmlParser);

     // --------------------------------------------------------------------------
     // Format-specific saving methods
     // --------------------------------------------------------------------------
     static int32_t h_SaveBinary(C_OscProject &orc_Project, const QString &orc_Path,
                                 const QString &orc_OpenSydeVersion, QDataStream &orc_Stream);
     static int32_t h_SaveJson(C_OscProject &orc_Project, const QString &orc_Path,
                               const QString &orc_OpenSydeVersion, QJsonObject &orc_Object);
     static int32_t h_SaveXml(C_OscProject &orc_Project, const QString &orc_Path,
                              const QString &orc_OpenSydeVersion, C_OscXmlParserBase &orc_XmlParser);

     // --------------------------------------------------------------------------
     // Legacy compatibility methods (deprecated)
     // --------------------------------------------------------------------------
     [[deprecated("Use h_Save/h_Load with auto-detection instead")]]
     static int32_t h_SaveLegacy(C_OscProject &orc_Project, const QString &orc_Path,
                                 const QString &orc_OpenSydeVersion);
     [[deprecated("Use h_Save/h_Load with auto-detection instead")]]
     static int32_t h_LoadLegacy(C_OscProject &orc_Project, const QString &orc_Path);

 private:
     static int32_t mh_SaveInternal(C_OscProject &orc_Project,
                                    const QString &orc_Path,
                                    const QString &orc_OpenSydeVersion,
                                    const bool oq_New);
 };

 /* -- Extern Global Variables
  * ---------------------------------------------------------------------------------------
  */
 } // namespace opensyde_core
 } // namespace stw

 #endif
