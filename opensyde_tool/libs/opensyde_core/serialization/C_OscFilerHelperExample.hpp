//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief    Example implementation showing how to migrate a Filer class to use Qt-native serialization
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/**
 * \example C_OscXappPropertiesExample.hpp
 *
 * This example demonstrates how to migrate an existing Filer class to use the
 * new Qt-native serialization framework (C_OscFilerHelper).
 *
 * \section example_overview Overview
 *
 * Traditional approach (XML only):
 * - Separate Filer class with manual XML parsing
 * - ~200-800 lines of repetitive code
 * - Hard to maintain across multiple formats
 *
 * New approach (Qt-native):
 * - Implement serialization methods in the data class itself
 * - Use C_OscFilerHelper template for file I/O
 * - Support multiple formats (binary, JSON, XML) with minimal code
 *
 * \section example_implementation Implementation Steps
 *
 * 1. Add serialization methods to your data class
 * 2. Use C_OscFilerHelper for file operations
 * 3. Remove old Filer class (optional, can keep for backward compatibility)
 *
 * \section example_code Example Code
 */

#ifndef C_OSCXAPPPROPERTIESEXAMPLE_HPP
#define C_OSCXAPPPROPERTIESEXAMPLE_HPP

#include "serialization/C_OscFilerHelper.hpp"
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QDataStream>
#include <QDomDocument>

namespace stw {
namespace opensyde_core {

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Example class showing how to implement serialization methods
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OscXappPropertiesExample {
public:
   C_OscXappPropertiesExample() : m_Id(0), m_qEnabled(false) {}

   // ========================================================================
   // 1. Binary Serialization Methods (QDataStream)
   // ========================================================================

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Serialize object to QDataStream

      \param[out] orc_Out   Output stream

      \note This method must be implemented for binary serialization
   */
   //----------------------------------------------------------------------------------------------------------------------
   void ToQDataStream(QDataStream& orc_Out) const {
      orc_Out << m_Id;
      orc_Out << m_cName;
      orc_Out << m_qEnabled;
      orc_Out << static_cast<qint32>(m_Values.size());
      for (const auto& val : m_Values) {
         orc_Out << val;
      }
   }

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Deserialize object from QDataStream

      \param[in,out] orc_In   Input stream

      \note This method must be implemented for binary deserialization
   */
   //----------------------------------------------------------------------------------------------------------------------
   void FromQDataStream(QDataStream& orc_In) {
      qint32 s32_ValueCount = 0;
      orc_In >> m_Id;
      orc_In >> m_cName;
      orc_In >> m_qEnabled;
      orc_In >> s32_ValueCount;

      m_Values.clear();
      for (int32_t i = 0; i < s32_ValueCount; ++i) {
         QString c_Value;
         orc_In >> c_Value;
         m_Values.append(c_Value);
      }
   }

   // ========================================================================
   // 2. JSON Serialization Methods (QJsonObject)
   // ========================================================================

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Serialize object to QJsonObject

      \return JSON representation of the object

      \note This method must be implemented for JSON serialization
   */
   //----------------------------------------------------------------------------------------------------------------------
   QJsonObject ToJsonObject() const {
      QJsonObject obj;
      obj["id"] = m_Id;
      obj["name"] = m_cName;
      obj["enabled"] = m_qEnabled;

      QJsonArray valuesArray;
      for (const auto& val : m_Values) {
         valuesArray.append(val);
      }
      obj["values"] = valuesArray;

      return obj;
   }

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Deserialize object from QJsonObject

      \param[in] orc_Json   JSON object to deserialize

      \note This method must be implemented for JSON deserialization
   */
   //----------------------------------------------------------------------------------------------------------------------
   void FromJsonObject(const QJsonObject& orc_Json) {
      m_Id = orc_Json["id"].toInt();
      m_cName = orc_Json["name"].toString();
      m_qEnabled = orc_Json["enabled"].toBool();

      m_Values.clear();
      if (orc_Json.contains("values") && orc_Json["values"].isArray()) {
         QJsonArray valuesArray = orc_Json["values"].toArray();
         for (const auto& val : valuesArray) {
            m_Values.append(val.toString());
         }
      }
   }

   // ========================================================================
   // 3. XML Serialization Methods (QDomDocument)
   // ========================================================================

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Serialize object to QDomDocument

      \param[out] orc_Doc   XML document to populate

      \note This method must be implemented for XML serialization
   */
   //----------------------------------------------------------------------------------------------------------------------
   void ToQDomDocument(QDomDocument& orc_Doc) const {
      QDomElement root = orc_Doc.createElement("xapp-properties");
      root.setAttribute("id", QString::number(m_Id));
      root.setAttribute("name", m_cName);
      root.setAttribute("enabled", m_qEnabled ? "true" : "false");

      QDomElement valuesElement = orc_Doc.createElement("values");
      for (const auto& val : m_Values) {
         QDomElement valueElement = orc_Doc.createElement("value");
         valueElement.appendChild(orc_Doc.createTextNode(val));
         valuesElement.appendChild(valueElement);
      }
      root.appendChild(valuesElement);

      orc_Doc.appendChild(root);
   }

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Deserialize object from QDomDocument

      \param[in] orc_Doc   XML document to parse

      \note This method must be implemented for XML deserialization
   */
   //----------------------------------------------------------------------------------------------------------------------
   void FromQDomDocument(const QDomDocument& orc_Doc) {
      QDomElement root = orc_Doc.documentElement();

      if (root.hasAttribute("id")) {
         m_Id = root.attribute("id").toInt();
      }
      if (root.hasAttribute("name")) {
         m_cName = root.attribute("name");
      }
      if (root.hasAttribute("enabled")) {
         m_qEnabled = (root.attribute("enabled") == "true");
      }

      m_Values.clear();
      QDomElement valuesElement = root.firstChildElement("values");
      QDomElement valueElement = valuesElement.firstChildElement("value");
      while (!valueElement.isNull()) {
         m_Values.append(valueElement.text());
         valueElement = valueElement.nextSiblingElement("value");
      }
   }

   // ========================================================================
   // 4. Public Data Members (for demonstration)
   // ========================================================================

   int32_t m_Id;                    ///< Property ID
   QString m_cName;                 ///< Property name
   bool m_qEnabled;                 ///< Enabled flag
   QList<QString> m_Values;         ///< Value list
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Example usage of C_OscFilerHelper with C_OscXappPropertiesExample

   This demonstrates how simple it is to save/load objects using the new framework.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OscXappPropertiesExampleUsage {
public:
   static void DemonstrateUsage() {
      // Create an example object
      C_OscXappPropertiesExample example;
      example.m_Id = 42;
      example.m_cName = "ExampleProperty";
      example.m_qEnabled = true;
      example.m_Values.append("Value1");
      example.m_Values.append("Value2");

      // =====================================================================
      // Binary Serialization (Fast, compact)
      // =====================================================================
      QString binaryPath = "example.bin";
      int32_t result = C_OscFilerHelper<C_OscXappPropertiesExample>::SaveBinary(example, binaryPath);
      if (result == C_NO_ERR) {
         // Successfully saved to binary
      }

      // Load from binary
      C_OscXappPropertiesExample loadedExample;
      result = C_OscFilerHelper<C_OscXappPropertiesExample>::LoadBinary(loadedExample, binaryPath);
      if (result == C_NO_ERR) {
         // Successfully loaded from binary
      }

      // =====================================================================
      // JSON Serialization (Human-readable)
      // =====================================================================
      QString jsonPath = "example.json";
      result = C_OscFilerHelper<C_OscXappPropertiesExample>::SaveJson(example, jsonPath);
      if (result == C_NO_ERR) {
         // Successfully saved to JSON
      }

      // Load from JSON
      result = C_OscFilerHelper<C_OscXappPropertiesExample>::LoadJson(loadedExample, jsonPath);
      if (result == C_NO_ERR) {
         // Successfully loaded from JSON
      }

      // =====================================================================
      // XML Serialization (Legacy compatibility)
      // =====================================================================
      QString xmlPath = "example.xml";
      result = C_OscFilerHelper<C_OscXappPropertiesExample>::SaveXml(example, xmlPath);
      if (result == C_NO_ERR) {
         // Successfully saved to XML
      }

      // Load from XML
      result = C_OscFilerHelper<C_OscXappPropertiesExample>::LoadXml(loadedExample, xmlPath);
      if (result == C_NO_ERR) {
         // Successfully loaded from XML
      }

      // =====================================================================
      // In-Memory Serialization (for caching, network transfer)
      // =====================================================================

      // Binary to memory
      QByteArray binaryData = C_OscFilerHelper<C_OscXappPropertiesExample>::SaveToMemoryBinary(example);
      C_OscXappPropertiesExample memoryExample;
      C_OscFilerHelper<C_OscXappPropertiesExample>::LoadFromMemoryBinary(memoryExample, binaryData);

      // JSON to memory
      QString jsonString = C_OscFilerHelper<C_OscXappPropertiesExample>::SaveToMemoryJson(example);
      C_OscFilerHelper<C_OscXappPropertiesExample>::LoadFromMemoryJson(memoryExample, jsonString);
   }
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCXAPPPROPERTIESEXAMPLE_HPP

/**
 * \section example_benefits Benefits of This Approach

 * 1. **Code Reduction**: From ~200 lines of Filer code to ~50 lines of serialization methods
 * 2. **Multiple Formats**: Support binary, JSON, and XML with the same code
 * 3. **Type Safety**: Compile-time checking via templates
 * 4. **Maintainability**: Serialization logic is in the data class, not scattered across filers
 * 5. **Performance**: Binary format is 5-10x faster than XML
 *
 * \section example_migration Migration Guide

 * To migrate an existing Filer class:
 *
 * 1. **Identify the data class** (e.g., C_OscXappProperties)
 * 2. **Add serialization methods** to the data class:
 *    - ToQDataStream() / FromQDataStream()
 *    - ToJsonObject() / FromJsonObject()
 *    - ToQDomDocument() / FromQDomDocument()
 * 3. **Replace Filer calls** with C_OscFilerHelper calls
 * 4. **Test thoroughly** to ensure data integrity
 * 5. **Optionally remove** old Filer class (keep for backward compatibility if needed)
 *
 * \section example_next_steps Next Steps

 * - Apply this pattern to other Filer classes
 * - Create code generation tool for repetitive serialization methods
 * - Benchmark performance vs. old XML-only approach
 * - Document migration progress
 */
