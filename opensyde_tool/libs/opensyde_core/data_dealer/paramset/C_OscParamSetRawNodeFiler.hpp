//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       File handler for parameter set raw node data (new Qt-native implementation)

   File handler for parameter set raw node data with Qt-native serialization support (binary, JSON, XML).

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPARAMSETRAWNODEFILER_NEW_HPP
#define C_OSCPARAMSETRAWNODEFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscParamSetRawNode.hpp"
#include <QDataStream>
#include <QJsonObject>
#include <QDomDocument>
#include <QFile>

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

/// File handler for parameter set raw node data with Qt-native serialization
class C_OscParamSetRawNodeFiler_New {
public:
   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load parameter set raw node from file (auto-detect format)

      \param[in] c_FilePath Path to the file to load
      \param[out] rc_Node    Loaded node data

      \return C_NO_ERR on success, error code otherwise
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t h_LoadFile(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node);

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save parameter set raw node to file (auto-detect format)

      \param[in] c_FilePath Path to the file to save
      \param[in] rc_Node    Node data to save

      \return C_NO_ERR on success, error code otherwise
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t h_SaveFile(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node);

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load parameter set raw node from binary file

      \param[in] c_FilePath Path to the binary file to load
      \param[out] rc_Node    Loaded node data

      \return C_NO_ERR on success, error code otherwise
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t h_LoadBinary(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node);

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save parameter set raw node to binary file

      \param[in] c_FilePath Path to the binary file to save
      \param[in] rc_Node    Node data to save

      \return C_NO_ERR on success, error code otherwise
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t h_SaveBinary(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node);

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load parameter set raw node from JSON file

      \param[in] c_FilePath Path to the JSON file to load
      \param[out] rc_Node    Loaded node data

      \return C_NO_ERR on success, error code otherwise
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t h_LoadJson(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node);

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save parameter set raw node to JSON file

      \param[in] c_FilePath Path to the JSON file to save
      \param[in] rc_Node    Node data to save

      \return C_NO_ERR on success, error code otherwise
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t h_SaveJson(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node);

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load parameter set raw node from XML file

      \param[in] c_FilePath Path to the XML file to load
      \param[out] rc_Node    Loaded node data

      \return C_NO_ERR on success, error code otherwise
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t h_LoadXml(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node);

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save parameter set raw node to XML file

      \param[in] c_FilePath Path to the XML file to save
      \param[in] rc_Node    Node data to save

      \return C_NO_ERR on success, error code otherwise
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t h_SaveXml(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif