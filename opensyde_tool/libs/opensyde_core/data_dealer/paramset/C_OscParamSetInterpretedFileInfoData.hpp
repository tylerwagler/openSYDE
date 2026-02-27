//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Storage for interpreted param set file info data (header)

   See cpp file for detailed description

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPARAMSETINTERPRETEDFILEINFODATA_HPP
#define C_OSCPARAMSETINTERPRETEDFILEINFODATA_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
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

class C_OscParamSetInterpretedFileInfoData {
public:
  C_OscParamSetInterpretedFileInfoData(void);

  void Clear(void);
  void AddInterpretedFileData(const QString &orc_DateTime,
                              const QString &orc_Creator,
                              const QString &orc_ToolName,
                              const QString &orc_ToolVersion,
                              const QString &orc_ProjectName,
                              const QString &orc_ProjectVersion,
                              const QString &orc_UserComment);

  // Date and time of creation
  QString c_DateTime;
  // User who created this file
  QString c_Creator;
  // Name of tool used for creating this param set file
  QString c_ToolName;
  // Version of tool used for creating this param set file
  QString c_ToolVersion;
  // Name of project used for creating this param set file
  QString c_ProjectName;
  // Version of project used for creating this param set file
  QString c_ProjectVersion;
  // Optional user comment for this param set file
  QString c_UserComment;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
