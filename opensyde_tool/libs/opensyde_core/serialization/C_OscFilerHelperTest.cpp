//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief    Unit tests for C_OscFilerHelper template
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "C_OscFilerHelper.hpp"
#include "precomp_headers.hpp"
#include <QTest>
#include <QTemporaryFile>
#include <QDebug>

namespace stw {
namespace opensyde_core {

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Test data class for C_OscFilerHelper tests
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OscFilerHelperTestData {
public:
   C_OscFilerHelperTestData() : m_Id(0), m_qEnabled(false) {}

   // Binary serialization
   void ToQDataStream(QDataStream& out) const {
      out << m_Id;
      out << m_cName;
      out << m_qEnabled;
      out << static_cast<qint32>(m_Values.size());
      for (const auto& val : m_Values) {
         out << val;
      }
   }

   void FromQDataStream(QDataStream& in) {
      qint32 count = 0;
      in >> m_Id >> m_cName >> m_qEnabled >> count;
      m_Values.clear();
      for (int32_t i = 0; i < count; ++i) {
         QString val;
         in >> val;
         m_Values.append(val);
      }
   }

   // JSON serialization
   QJsonObject ToJsonObject() const {
      QJsonObject obj;
      obj["id"] = m_Id;
      obj["name"] = m_cName;
      obj["enabled"] = m_qEnabled;
      QJsonArray arr;
      for (const auto& v : m_Values) arr.append(v);
      obj["values"] = arr;
      return obj;
   }

   void FromJsonObject(const QJsonObject& json) {
      m_Id = json["id"].toInt();
      m_cName = json["name"].toString();
      m_qEnabled = json["enabled"].toBool();
      m_Values.clear();
      if (json.contains("values") && json["values"].isArray()) {
         for (const auto& v : json["values"].toArray()) {
            m_Values.append(v.toString());
         }
      }
   }

   // XML serialization
   void ToQDomDocument(QDomDocument& doc) const {
      QDomElement root = doc.createElement("test-data");
      root.setAttribute("id", m_Id);
      root.setAttribute("name", m_cName);
      root.setAttribute("enabled", m_qEnabled ? "true" : "false");
      QDomElement values = doc.createElement("values");
      for (const auto& v : m_Values) {
         QDomElement val = doc.createElement("value");
         val.appendChild(doc.createTextNode(v));
         values.appendChild(val);
      }
      root.appendChild(values);
      doc.appendChild(root);
   }

   void FromQDomDocument(const QDomDocument& doc) {
      QDomElement root = doc.documentElement();
      m_Id = root.attribute("id").toInt();
      m_cName = root.attribute("name");
      m_qEnabled = (root.attribute("enabled") == "true");
      m_Values.clear();
      QDomElement values = root.firstChildElement("values");
      QDomElement val = values.firstChildElement("value");
      while (!val.isNull()) {
         m_Values.append(val.text());
         val = val.nextSiblingElement("value");
      }
   }

   int32_t m_Id;
   QString m_cName;
   bool m_qEnabled;
   QList<QString> m_Values;
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Test suite for C_OscFilerHelper
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OscFilerHelperTest : public QObject {
   Q_OBJECT

private slots:
   void TestBinarySerialization();
   void TestJsonSerialization();
   void TestXmlSerialization();
   void TestInMemoryBinary();
   void TestInMemoryJson();

private:
   void VerifyTestData(const C_OscFilerHelperTestData& orc_Data);
};

void C_OscFilerHelperTest::VerifyTestData(const C_OscFilerHelperTestData& orc_Data) {
   QCOMPARE(orc_Data.m_Id, 42);
   QCOMPARE(orc_Data.m_cName, QString("TestName"));
   QCOMPARE(orc_Data.m_qEnabled, true);
   QCOMPARE(orc_Data.m_Values.size(), 3);
   QCOMPARE(orc_Data.m_Values[0], QString("Value1"));
   QCOMPARE(orc_Data.m_Values[1], QString("Value2"));
   QCOMPARE(orc_Data.m_Values[2], QString("Value3"));
}

void C_OscFilerHelperTest::TestBinarySerialization() {
   // Create test data
   C_OscFilerHelperTestData original;
   original.m_Id = 42;
   original.m_cName = "TestName";
   original.m_qEnabled = true;
   original.m_Values.append("Value1");
   original.m_Values.append("Value2");
   original.m_Values.append("Value3");

   // Create temporary file
   QTemporaryFile tempFile;
   QVERIFY(tempFile.open());
   QString tempPath = tempFile.fileName();
   tempFile.close();

   // Save binary
   int32_t result = C_OscFilerHelper<C_OscFilerHelperTestData>::SaveBinary(original, tempPath);
   QCOMPARE(result, C_NO_ERR);

   // Load binary
   C_OscFilerHelperTestData loaded;
   result = C_OscFilerHelper<C_OscFilerHelperTestData>::LoadBinary(loaded, tempPath);
   QCOMPARE(result, C_NO_ERR);

   // Verify
   VerifyTestData(loaded);

   // Cleanup
   QFile::remove(tempPath);
}

void C_OscFilerHelperTest::TestJsonSerialization() {
   // Create test data
   C_OscFilerHelperTestData original;
   original.m_Id = 42;
   original.m_cName = "TestName";
   original.m_qEnabled = true;
   original.m_Values.append("Value1");
   original.m_Values.append("Value2");
   original.m_Values.append("Value3");

   // Create temporary file
   QTemporaryFile tempFile;
   QVERIFY(tempFile.open());
   QString tempPath = tempFile.fileName();
   tempFile.close();

   // Save JSON
   int32_t result = C_OscFilerHelper<C_OscFilerHelperTestData>::SaveJson(original, tempPath);
   QCOMPARE(result, C_NO_ERR);

   // Load JSON
   C_OscFilerHelperTestData loaded;
   result = C_OscFilerHelper<C_OscFilerHelperTestData>::LoadJson(loaded, tempPath);
   QCOMPARE(result, C_NO_ERR);

   // Verify
   VerifyTestData(loaded);

   // Cleanup
   QFile::remove(tempPath);
}

void C_OscFilerHelperTest::TestXmlSerialization() {
   // Create test data
   C_OscFilerHelperTestData original;
   original.m_Id = 42;
   original.m_cName = "TestName";
   original.m_qEnabled = true;
   original.m_Values.append("Value1");
   original.m_Values.append("Value2");
   original.m_Values.append("Value3");

   // Create temporary file
   QTemporaryFile tempFile;
   QVERIFY(tempFile.open());
   QString tempPath = tempFile.fileName();
   tempFile.close();

   // Save XML
   int32_t result = C_OscFilerHelper<C_OscFilerHelperTestData>::SaveXml(original, tempPath);
   QCOMPARE(result, C_NO_ERR);

   // Load XML
   C_OscFilerHelperTestData loaded;
   result = C_OscFilerHelper<C_OscFilerHelperTestData>::LoadXml(loaded, tempPath);
   QCOMPARE(result, C_NO_ERR);

   // Verify
   VerifyTestData(loaded);

   // Cleanup
   QFile::remove(tempPath);
}

void C_OscFilerHelperTest::TestInMemoryBinary() {
   // Create test data
   C_OscFilerHelperTestData original;
   original.m_Id = 42;
   original.m_cName = "TestName";
   original.m_qEnabled = true;
   original.m_Values.append("Value1");
   original.m_Values.append("Value2");
   original.m_Values.append("Value3");

   // Save to memory
   QByteArray data = C_OscFilerHelper<C_OscFilerHelperTestData>::SaveToMemoryBinary(original);
   QVERIFY(!data.isEmpty());

   // Load from memory
   C_OscFilerHelperTestData loaded;
   int32_t result = C_OscFilerHelper<C_OscFilerHelperTestData>::LoadFromMemoryBinary(loaded, data);
   QCOMPARE(result, C_NO_ERR);

   // Verify
   VerifyTestData(loaded);
}

void C_OscFilerHelperTest::TestInMemoryJson() {
   // Create test data
   C_OscFilerHelperTestData original;
   original.m_Id = 42;
   original.m_cName = "TestName";
   original.m_qEnabled = true;
   original.m_Values.append("Value1");
   original.m_Values.append("Value2");
   original.m_Values.append("Value3");

   // Save to memory (JSON)
   QString jsonStr = C_OscFilerHelper<C_OscFilerHelperTestData>::SaveToMemoryJson(original);
   QVERIFY(!jsonStr.isEmpty());
   QVERIFY(jsonStr.contains("TestName"));

   // Load from memory
   C_OscFilerHelperTestData loaded;
   int32_t result = C_OscFilerHelper<C_OscFilerHelperTestData>::LoadFromMemoryJson(loaded, jsonStr);
   QCOMPARE(result, C_NO_ERR);

   // Verify
   VerifyTestData(loaded);
}

QTEST_MAIN(C_OscFilerHelperTest)

#include "C_OscFilerHelperTest.moc"
