//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief    Unit tests for Qt-native serialization framework
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "C_OscSerializer.hpp"
#include "precomp_headers.hpp"
#include <QTest>
#include <QTemporaryFile>
#include <QDebug>

namespace stw {
namespace opensyde_core {

class C_OscSerializerTest : public QObject {
   Q_OBJECT

private slots:
   void TestBinarySerialization();
   void TestJsonSerialization();
   void TestXmlSerialization();
   void TestFormatDetection();

private:
   void CleanupTestFiles();
};

void C_OscSerializerTest::TestBinarySerialization() {
   // Create test data
   QByteArray testData;
   testData.resize(1024);
   for (int i = 0; i < testData.size(); ++i) {
      testData[i] = static_cast<char>(i % 256);
   }

   // Create temporary file
   QTemporaryFile tempFile;
   QVERIFY(tempFile.open());
   QString tempPath = tempFile.fileName();
   tempFile.close();

   // Save binary data
   int32_t result = C_OscSerializer::SaveBinary(testData, tempPath);
   QCOMPARE(result, C_NO_ERR);

   // Load binary data
   QByteArray loadedData;
   result = C_OscSerializer::LoadBinary(loadedData, tempPath);
   QCOMPARE(result, C_NO_ERR);

   // Verify data integrity
   QCOMPARE(loadedData.size(), testData.size());
   QCOMPARE(loadedData, testData);

   // Cleanup
   QFile::remove(tempPath);
}

void C_OscSerializerTest::TestJsonSerialization() {
   // Create test JSON object
   QJsonObject testObj;
   testObj["name"] = "TestNode";
   testObj["id"] = 42;
   testObj["enabled"] = true;
   QJsonArray array;
   array.append(1);
   array.append(2);
   array.append(3);
   testObj["values"] = array;

   // Create temporary file
   QTemporaryFile tempFile;
   QVERIFY(tempFile.open());
   QString tempPath = tempFile.fileName();
   tempFile.close();

   // Save JSON
   int32_t result = C_OscSerializer::SaveJson(testObj, tempPath);
   QCOMPARE(result, C_NO_ERR);

   // Load JSON
   QJsonObject loadedObj;
   result = C_OscSerializer::LoadJson(loadedObj, tempPath);
   QCOMPARE(result, C_NO_ERR);

   // Verify data integrity
   QCOMPARE(loadedObj["name"].toString(), testObj["name"].toString());
   QCOMPARE(loadedObj["id"].toInt(), testObj["id"].toInt());
   QCOMPARE(loadedObj["enabled"].toBool(), testObj["enabled"].toBool());
   QCOMPARE(loadedObj["values"].toArray().size(), testObj["values"].toArray().size());

   // Cleanup
   QFile::remove(tempPath);
}

void C_OscSerializerTest::TestXmlSerialization() {
   // Create test XML document
   QDomDocument doc;
   QDomElement root = doc.createElement("root");
   doc.appendChild(root);

   QDomElement child1 = doc.createElement("child1");
   child1.appendChild(doc.createTextNode("value1"));
   root.appendChild(child1);

   QDomElement child2 = doc.createElement("child2");
   child2.setAttribute("attr", "value2");
   root.appendChild(child2);

   // Create temporary file
   QTemporaryFile tempFile;
   QVERIFY(tempFile.open());
   QString tempPath = tempFile.fileName();
   tempFile.close();

   // Save XML
   int32_t result = C_OscSerializer::SaveXml(doc, tempPath);
   QCOMPARE(result, C_NO_ERR);

   // Load XML
   QDomDocument loadedDoc;
   result = C_OscSerializer::LoadXml(loadedDoc, tempPath);
   QCOMPARE(result, C_NO_ERR);

   // Verify data integrity
   QDomElement loadedRoot = loadedDoc.documentElement();
   QCOMPARE(loadedRoot.tagName(), "root");
   QCOMPARE(loadedRoot.firstChildElement("child1").text(), "value1");
   QCOMPARE(loadedRoot.firstChildElement("child2").attribute("attr"), "value2");

   // Cleanup
   QFile::remove(tempPath);
}

void C_OscSerializerTest::TestFormatDetection() {
   QCOMPARE(C_OscSerializer::DetectFormat("test.bin"),
            C_OscSerializer::E_SerializationFormat::e_BINARY);
   QCOMPARE(C_OscSerializer::DetectFormat("test.BIN"),
            C_OscSerializer::E_SerializationFormat::e_BINARY);
   QCOMPARE(C_OscSerializer::DetectFormat("test.json"),
            C_OscSerializer::E_SerializationFormat::e_JSON);
   QCOMPARE(C_OscSerializer::DetectFormat("test.JSON"),
            C_OscSerializer::E_SerializationFormat::e_JSON);
   QCOMPARE(C_OscSerializer::DetectFormat("test.xml"),
            C_OscSerializer::E_SerializationFormat::e_XML);
   QCOMPARE(C_OscSerializer::DetectFormat("test.syde"),
            C_OscSerializer::E_SerializationFormat::e_XML);
   QCOMPARE(C_OscSerializer::DetectFormat("test.ods"),
            C_OscSerializer::E_SerializationFormat::e_XML);
   QCOMPARE(C_OscSerializer::DetectFormat("test.unknown"),
            C_OscSerializer::E_SerializationFormat::e_XML); // Default to XML
}

QTEST_MAIN(C_OscSerializerTest)

#include "C_OscSerializerTest.moc"
