//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for signature

   Filer for signature

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <string>
#include <system_error>
#include "TglUtils.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscXmlParser.hpp"
#include "C_OscSupSignatureFiler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
static const uint16_t mu16_FILE_VERSION = 1U;
static const std::string mc_SIG_TYPE = "ECDSA secp256r1";
// XML node names of service update package definition
static const std::string mc_ROOT_NAME = "opensyde-secure-update-collection-signature"; // xml root node
static const std::string mc_FILE_VERSION = "file-version";                             // xml node
static const std::string mc_NODE_SIG = "signature";                                    // xml node
static const std::string mc_NODE_SIG_TYPE_ATTR = "type";                               // xml node attribute
static const std::string mc_NODE_SIG_VALUE_ATTR = "value";                             // xml node attribute

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Create signature file

   \param[in]  orc_Path       Path
   \param[in]  orc_Signature  Signature

   \return
   std::error_code

   \retval   Errc::success   File created
   \retval   Errc::rd_wr     File not created
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupSignatureFiler::h_CreateSignatureFile(const std::string & orc_Path,
                                                              const std::string & orc_Signature)
{
   std::error_code c_Result;

   // fill update package definition
   C_OscXmlParser c_XmlParser;

   //Root Node
   c_XmlParser.CreateAndSelectNodeChild(mc_ROOT_NAME);

   //File version
   tgl_assert(c_XmlParser.CreateAndSelectNodeChild(mc_FILE_VERSION) == mc_FILE_VERSION);
   c_XmlParser.SetNodeContent(std::to_string(mu16_FILE_VERSION));
   tgl_assert(c_XmlParser.SelectNodeParent() == mc_ROOT_NAME);

   //signature
   tgl_assert(c_XmlParser.CreateAndSelectNodeChild(mc_NODE_SIG) == mc_NODE_SIG);
   c_XmlParser.SetAttributeString(mc_NODE_SIG_TYPE_ATTR, mc_SIG_TYPE);
   c_XmlParser.SetAttributeString(mc_NODE_SIG_VALUE_ATTR, orc_Signature);
   tgl_assert(c_XmlParser.SelectNodeParent() == mc_ROOT_NAME);

   // save signature file
   c_Result = static_cast<Errc>(c_XmlParser.SaveToFile(orc_Path));
   if (c_Result)
   {
      c_Result = Errc::rd_wr;
   }
   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load signature file

   \param[in]      orc_Path         Path
   \param[in,out]  orc_Signature    Signature

   \return
   std::error_code

   \retval   Errc::success   File read
   \retval   Errc::rd_wr     File not read
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupSignatureFiler::h_LoadSignatureFile(const std::string & orc_Path,
                                                            std::string & orc_Signature)
{
   std::error_code c_Result;
   C_OscXmlParser c_XmlParser;

   c_Result = static_cast<Errc>(c_XmlParser.LoadFromFile(orc_Path));
   if (!c_Result)
   {
      tgl_assert(c_XmlParser.SelectRoot() == mc_ROOT_NAME);

      // file version
      tgl_assert(c_XmlParser.SelectNodeChild(mc_FILE_VERSION) == mc_FILE_VERSION);
      const std::string c_FileVersion = c_XmlParser.GetNodeContent();
      const uint32_t u32_FileVersion = static_cast<uint32_t>(std::stoi(c_FileVersion));
      tgl_assert(c_XmlParser.SelectRoot() == mc_ROOT_NAME);

      if (u32_FileVersion == mu16_FILE_VERSION)
      {
         //signature
         tgl_assert(c_XmlParser.SelectNodeChild(mc_NODE_SIG) == mc_NODE_SIG);
         const std::string c_SigType = c_XmlParser.GetAttributeString(mc_NODE_SIG_TYPE_ATTR);
         tgl_assert(c_SigType == mc_SIG_TYPE);
         orc_Signature = c_XmlParser.GetAttributeString(mc_NODE_SIG_VALUE_ATTR);

         tgl_assert(c_XmlParser.SelectRoot() == mc_ROOT_NAME);
      }
   }
   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get signature file name

   \return
   Signature file name
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscSupSignatureFiler::h_GetSignatureFileName()
{
   return "signature.syde_sucsig";
}
