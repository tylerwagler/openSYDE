//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for standalone HALC config
   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCCONFIGSTANDALONEFILER_HPP
#define C_OSCHALCCONFIGSTANDALONEFILER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscHalcConfigStandalone.hpp"
#include "C_OscXmlParser.hpp"

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

class C_OscHalcConfigStandaloneFiler {
public:
  C_OscHalcConfigStandaloneFiler(void);

  static int32_t h_LoadFileStandalone(C_OscHalcConfigStandalone &orc_IoData,
                                      const QString &orc_Path);
  static int32_t
  h_SaveFileStandalone(const C_OscHalcConfigStandalone &orc_IoData,
                       const QString &orc_Path);
  static int32_t h_LoadDataStandalone(C_OscHalcConfigStandalone &orc_IoData,
                                      C_OscXmlParserBase &orc_XmlParser);
  static int32_t
  h_SaveDataStandalone(const C_OscHalcConfigStandalone &orc_IoData,
                       C_OscXmlParserBase &orc_XmlParser);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
