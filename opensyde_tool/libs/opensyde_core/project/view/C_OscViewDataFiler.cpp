//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for C_OscViewData with Qt-native serialization
   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscViewDataFiler.hpp"
#include "C_OscFilerUtil.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
using namespace stw::opensyde_core;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */

/* -- Module Global Variables
 * ---------------------------------------------------------------------------------------
 */

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscViewDataFiler::C_OscViewDataFiler() {
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_LoadFile(QList<C_OscViewData> &orc_Views,
                                        const QString &orc_Path) {
   return C_OscFilerUtil::h_LoadListFile(orc_Views, orc_Path, "views", "view", 1U);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_SaveFile(const QList<C_OscViewData> &orc_Views,
                                        const QString &orc_Path) {
   return C_OscFilerUtil::h_SaveListFile(orc_Views, orc_Path, "views", "view", 1U);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_LoadBinary(QList<C_OscViewData> &orc_Views,
                                          const QString &orc_Path) {
   return C_OscFilerUtil::h_LoadListBinary(orc_Views, orc_Path, 1U);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_SaveBinary(const QList<C_OscViewData> &orc_Views,
                                          const QString &orc_Path) {
   return C_OscFilerUtil::h_SaveListBinary(orc_Views, orc_Path, 1U);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_LoadJson(QList<C_OscViewData> &orc_Views,
                                        const QString &orc_Path) {
   return C_OscFilerUtil::h_LoadListJson(orc_Views, orc_Path);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_SaveJson(const QList<C_OscViewData> &orc_Views,
                                        const QString &orc_Path) {
   return C_OscFilerUtil::h_SaveListJson(orc_Views, orc_Path);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_LoadXml(QList<C_OscViewData> &orc_Views,
                                       const QString &orc_Path) {
   return C_OscFilerUtil::h_LoadListXml(orc_Views, orc_Path, "views", "view");
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_SaveXml(const QList<C_OscViewData> &orc_Views,
                                       const QString &orc_Path) {
   return C_OscFilerUtil::h_SaveListXml(orc_Views, orc_Path, "views", "view");
}
