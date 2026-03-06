//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for C_OscViewData with Qt-native serialization
   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCVIEWDATAFILER_HPP
#define C_OSCVIEWDATAFILER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscViewData.hpp"
#include "stwtypes.hpp"

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

class C_OscViewDataFiler {
public:
   static int32_t h_LoadFile(QList<C_OscViewData> &orc_Views,
                             const QString &orc_Path);
   static int32_t h_SaveFile(const QList<C_OscViewData> &orc_Views,
                             const QString &orc_Path);

   static int32_t h_LoadBinary(QList<C_OscViewData> &orc_Views,
                               const QString &orc_Path);
   static int32_t h_SaveBinary(const QList<C_OscViewData> &orc_Views,
                               const QString &orc_Path);

   static int32_t h_LoadJson(QList<C_OscViewData> &orc_Views,
                             const QString &orc_Path);
   static int32_t h_SaveJson(const QList<C_OscViewData> &orc_Views,
                             const QString &orc_Path);

   static int32_t h_LoadXml(QList<C_OscViewData> &orc_Views,
                            const QString &orc_Path);
   static int32_t h_SaveXml(const QList<C_OscViewData> &orc_Views,
                            const QString &orc_Path);

private:
   C_OscViewDataFiler();
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
