//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for signature

   Filer for signature

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscSupSignatureFiler.hpp"
#include "C_OscXmlParser.hpp"
#include "stwerrors.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */
static const uint16_t mu16_FILE_VERSION = 1U;
static const QString mc_SIG_TYPE = "ECDSA secp256r1";
// XML node names of service update package definition
static const QString mc_ROOT_NAME =
    "opensyde-secure-update-collection-signature";     // xml root node
static const QString mc_FILE_VERSION = "file-version"; // xml node
static const QString mc_NODE_SIG = "signature";        // xml node
static const QString mc_NODE_SIG_TYPE_ATTR = "type";   // xml node attribute
static const QString mc_NODE_SIG_VALUE_ATTR = "value"; // xml node attribute

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
/*! \brief  Create signature file

   \param[in]  orc_Path       Path
   \param[in]  orc_Signature  Signature

   \return
   STW error codes

   \retval   C_NO_ERR   File created
   \retval   C_RD_WR    File not created
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t
C_OscSupSignatureFiler::h_CreateSignatureFile(const QString &orc_Path,
                                              const QString &orc_Signature) {
  int32_t s32_Result;

  // fill update package definition
  C_OscXmlParser c_XmlParser;

  // Root Node
  c_XmlParser.CreateAndSelectNodeChild(mc_ROOT_NAME);

  // File version
  Q_ASSERT(c_XmlParser.CreateAndSelectNodeChild(mc_FILE_VERSION) ==
           mc_FILE_VERSION);
  c_XmlParser.SetNodeContent(QString::number(mu16_FILE_VERSION));
  Q_ASSERT(c_XmlParser.SelectNodeParent() == mc_ROOT_NAME);

  // signature
  Q_ASSERT(c_XmlParser.CreateAndSelectNodeChild(mc_NODE_SIG) == mc_NODE_SIG);
  c_XmlParser.SetAttributeString(mc_NODE_SIG_TYPE_ATTR, mc_SIG_TYPE);
  c_XmlParser.SetAttributeString(mc_NODE_SIG_VALUE_ATTR, orc_Signature);
  Q_ASSERT(c_XmlParser.SelectNodeParent() == mc_ROOT_NAME);

  // save signature file
  s32_Result = c_XmlParser.SaveToFile(orc_Path);
  if (s32_Result != C_NO_ERR) {
    s32_Result = C_RD_WR;
  }
  return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load signature file

   \param[in]      orc_Path         Path
   \param[in,out]  orc_Signature    Signature

   \return
   STW error codes

   \retval   C_NO_ERR   File read
   \retval   C_RD_WR    File not read
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSupSignatureFiler::h_LoadSignatureFile(const QString &orc_Path,
                                                    QString &orc_Signature) {
  int32_t s32_Result;
  C_OscXmlParser c_XmlParser;

  s32_Result = c_XmlParser.LoadFromFile(orc_Path);
  if (s32_Result == C_NO_ERR) {
    Q_ASSERT(c_XmlParser.SelectRoot() == mc_ROOT_NAME);

    // file version
    Q_ASSERT(c_XmlParser.SelectNodeChild(mc_FILE_VERSION) == mc_FILE_VERSION);
    const QString c_FileVersion = c_XmlParser.GetNodeContent();
    const uint32_t u32_FileVersion = c_FileVersion.toUInt();
    Q_ASSERT(c_XmlParser.SelectRoot() == mc_ROOT_NAME);

    if (u32_FileVersion == mu16_FILE_VERSION) {
      // signature
      Q_ASSERT(c_XmlParser.SelectNodeChild(mc_NODE_SIG) == mc_NODE_SIG);
      const QString c_SigType =
          c_XmlParser.GetAttributeString(mc_NODE_SIG_TYPE_ATTR);
      Q_ASSERT(c_SigType == mc_SIG_TYPE);
      orc_Signature = c_XmlParser.GetAttributeString(mc_NODE_SIG_VALUE_ATTR);

      Q_ASSERT(c_XmlParser.SelectRoot() == mc_ROOT_NAME);
    }
  }
  return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get signature file name

   \return
   Signature file name
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscSupSignatureFiler::h_GetSignatureFileName() {
  return "signature.syde_sucsig";
}
