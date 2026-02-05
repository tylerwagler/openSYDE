//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Bus reader/writer

   Load / save bus data from / to XML file

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscLoggingHandler.hpp"
#include "C_OscSystemBusFilerV2.hpp"
#include "C_OscSystemFilerUtil.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"


/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
using namespace stw::opensyde_core;

using namespace stw::errors;

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
/*! \brief   Load bus

   Load bus data.
   pre-condition: the passed XML parser has the active node set to "bus"
   post-condition: the passed XML parser has the active node set to the same
   "bus"

   \param[out]    orc_Bus        Bus data
   \param[in,out] orc_XmlParser  XML parser

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid or incomplete (content of orc_Bus is
   undefined)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBusFilerV2::h_LoadBus(C_OscSystemBus &orc_Bus,
                                         C_OscXmlParserBase &orc_XmlParser) {
  int32_t s32_Retval = C_NO_ERR;

  if (orc_XmlParser.SelectNodeChild("core") == "core") {
    // Name
    if (orc_XmlParser.SelectNodeChild("name") == "name") {
      orc_Bus.c_Name = orc_XmlParser.GetNodeContent();
      // Return
      Q_ASSERT(orc_XmlParser.SelectNodeParent() == "core");
    } else {
      osc_write_log_error("Loading bus definition",
                          "Could not find \"core\".\"name\" node.");
      s32_Retval = C_CONFIG;
    }
    // Comment
    if (s32_Retval == C_NO_ERR) {
      if (orc_XmlParser.SelectNodeChild("comment") == "comment") {
        orc_Bus.c_Comment = orc_XmlParser.GetNodeContent();
        // Return
        Q_ASSERT(orc_XmlParser.SelectNodeParent() == "core");
      }
      // Type
      if (orc_XmlParser.SelectNodeChild("type") == "type") {
        s32_Retval = C_OscSystemFilerUtil::h_BusTypeStringToEnum(
            orc_XmlParser.GetNodeContent(), orc_Bus.e_Type);
        if (s32_Retval != C_NO_ERR) {
          osc_write_log_error("Loading bus definition",
                              "Could not find \"core\".\"type\" node.");
          s32_Retval = C_CONFIG;
        }
        // Return
        Q_ASSERT(orc_XmlParser.SelectNodeParent() == "core");
      } else {
        osc_write_log_error("Loading bus definition",
                            "Could not find \"core\".\"type\" node.");
        s32_Retval = C_CONFIG;
      }
    }
    // Bitrate
    if (s32_Retval == C_NO_ERR) {
      if (orc_XmlParser.SelectNodeChild("bitrate") == "bitrate") {
        try {
          orc_Bus.u64_BitRate = orc_XmlParser.GetAttributeSint64("number");
        } catch (...) {
          osc_write_log_error("Loading bus definition",
                              "Invalid value for \"bitrate\".\"number\".");
          orc_Bus.u64_BitRate = 0ULL;
          s32_Retval = C_CONFIG;
        }
        // Return
        Q_ASSERT(orc_XmlParser.SelectNodeParent() == "core");
      } else {
        osc_write_log_error("Loading bus definition",
                            "Could not find \"core\".\"bitrate\" node.");
        s32_Retval = C_CONFIG;
      }
    }
    // Bus id
    if (s32_Retval == C_NO_ERR) {
      if (orc_XmlParser.SelectNodeChild("bus-id") == "bus-id") {
        try {
          orc_Bus.u8_BusId =
              static_cast<uint8_t>(orc_XmlParser.GetAttributeUint32("number"));
        } catch (...) {
          osc_write_log_error("Loading bus definition",
                              "Invalid value for \"bus-id\".\"number\".");
          orc_Bus.u8_BusId = 0;
          s32_Retval = C_CONFIG;
        }
        // Return
        Q_ASSERT(orc_XmlParser.SelectNodeParent() == "core");
      } else {
        osc_write_log_error("Loading bus definition",
                            "Could not find \"core\".\"bus-id\" node.");
        s32_Retval = C_CONFIG;
      }
    }
    // Rx delta time
    if (s32_Retval == C_NO_ERR) {
      if (orc_XmlParser.SelectNodeChild("rx-delta-time") == "rx-delta-time") {
        try {
          orc_Bus.u16_RxTimeoutOffsetMs =
              static_cast<uint16_t>(orc_XmlParser.GetAttributeUint32("number"));
        } catch (...) {
          osc_write_log_error(
              "Loading bus definition",
              "Invalid value for \"rx-delta-time\".\"number\".");
          orc_Bus.u16_RxTimeoutOffsetMs = 0U;
          s32_Retval = C_CONFIG;
        }
        // Return
        Q_ASSERT(orc_XmlParser.SelectNodeParent() == "core");
      } else {
        osc_write_log_error("Loading bus definition",
                            "Could not find \"core\".\"rx-delta-time\" node.");
        s32_Retval = C_CONFIG;
      }
    }
    // Return
    Q_ASSERT(orc_XmlParser.SelectNodeParent() == "bus");
  } else {
    osc_write_log_error("Loading bus definition",
                        "Could not find \"core\" node.");
    s32_Retval = C_CONFIG;
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save bus

   Save bus data to XML file
   pre-condition: the passed XML parser has the active node set to "bus"
   post-condition: the passed XML parser has the active node set to the same
   "bus"

   \param[in]     orc_Bus        Bus data to store
   \param[in,out] orc_XmlParser  XML with bus active
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSystemBusFilerV2::h_SaveBus(const C_OscSystemBus &orc_Bus,
                                      C_OscXmlParserBase &orc_XmlParser) {

  Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("core") == "core");
  // Name
  Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("name") == "name");
  orc_XmlParser.SetNodeContent(orc_Bus.c_Name);
  // Return
  Q_ASSERT(orc_XmlParser.SelectNodeParent() == "core");
  // Comment
  orc_XmlParser.CreateNodeChild("comment", orc_Bus.c_Comment);
  // Type
  Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("type") == "type");
  orc_XmlParser.SetNodeContent(
      C_OscSystemFilerUtil::h_BusTypeEnumToString(orc_Bus.e_Type));
  // Return
  Q_ASSERT(orc_XmlParser.SelectNodeParent() == "core");
  // Bitrate
  Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("bitrate") == "bitrate");
  orc_XmlParser.SetAttributeString("number",
                                   QString::number(orc_Bus.u64_BitRate));
  // Return
  Q_ASSERT(orc_XmlParser.SelectNodeParent() == "core");
  // Bus id
  Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("bus-id") == "bus-id");
  orc_XmlParser.SetAttributeUint32("number", orc_Bus.u8_BusId);
  // Return
  Q_ASSERT(orc_XmlParser.SelectNodeParent() == "core");
  // Bitrate
  Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("rx-delta-time") ==
           "rx-delta-time");
  orc_XmlParser.SetAttributeString(
      "number", QString::number(orc_Bus.u16_RxTimeoutOffsetMs));
  // Return
  Q_ASSERT(orc_XmlParser.SelectNodeParent() == "core");
  // Return
  Q_ASSERT(orc_XmlParser.SelectNodeParent() == "bus");
}
