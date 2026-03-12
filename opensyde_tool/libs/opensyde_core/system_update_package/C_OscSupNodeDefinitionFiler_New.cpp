 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save to binary format

    \param[in]  orc_File    File path
    \param[in]  orc_Node    Node definition

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupNodeDefinitionFiler_New::h_SaveBinary(
     const QString &orc_File,
     const C_OscSupNodeDefinition &orc_Node) {
     QFile c_File(orc_File);
     if (!c_File.open(QIODevice::WriteOnly)) {
         return C_RD_WR;
     }

     QDataStream c_Stream(&c_File);
     c_Stream.setVersion(QDataStream::Qt_5_12);

     // Write file version
     c_Stream << static_cast<uint16_t>(mu16_FILE_VERSION);

     // Write application files
     quint32 u32_Count = static_cast<quint32>(orc_Node.c_ApplicationFileNames.size());
     c_Stream << u32_Count;
     for (const QString &rc_File : orc_Node.c_ApplicationFileNames) {
         c_Stream << rc_File;
     }

     // Write NVM files
     u32_Count = static_cast<quint32>(orc_Node.c_NvmFileNames.size());
     c_Stream << u32_Count;
     for (const QString &rc_File : orc_Node.c_NvmFileNames) {
         c_Stream << rc_File;
     }

     // Write PEM file and config
     c_Stream << orc_Node.c_PemFile;
     if (!orc_Node.c_PemFile.isEmpty()) {
         c_Stream << orc_Node.q_SendSecurityEnabledState;
         c_Stream << orc_Node.q_SecurityEnabled;
         c_Stream << orc_Node.q_SendDebuggerEnabledState;
         c_Stream << orc_Node.q_DebuggerEnabled;
     }

     c_File.close();
     return (c_Stream.status() == QDataStream::Ok) ? C_NO_ERR : C_RD_WR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save to JSON format

    \param[in]  orc_File    File path
    \param[in]  orc_Node    Node definition

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupNodeDefinitionFiler_New::h_SaveJson(
     const QString &orc_File,
     const C_OscSupNodeDefinition &orc_Node) {
     QJsonObject c_Object;

     // File version
     c_Object["file-version"] = static_cast<double>(mu16_FILE_VERSION);

     // Application files
     QJsonArray c_AppFiles;
     for (const QString &rc_File : orc_Node.c_ApplicationFileNames) {
         c_AppFiles.append(rc_File);
     }
     c_Object["files"] = c_AppFiles;

     // NVM files
     QJsonArray c_NvmFiles;
     for (const QString &rc_File : orc_Node.c_NvmFileNames) {
         c_NvmFiles.append(rc_File);
     }
     c_Object["param-files"] = c_NvmFiles;

     // PEM config
     if (!orc_Node.c_PemFile.isEmpty()) {
         QJsonObject c_PemObj;
         c_PemObj["pem-file"] = orc_Node.c_PemFile;
         c_PemObj["security-send"] = orc_Node.q_SendSecurityEnabledState;
         c_PemObj["security-enabled"] = orc_Node.q_SecurityEnabled;
         c_PemObj["debugger-send"] = orc_Node.q_SendDebuggerEnabledState;
         c_PemObj["debugger-enabled"] = orc_Node.q_DebuggerEnabled;
         c_Object["pem-file-config"] = c_PemObj;
     }

     // Write to file
     QJsonDocument c_Doc(c_Object);
     QFile c_File(orc_File);
     if (!c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
         return C_RD_WR;
     }
     c_File.write(c_Doc.toJson(QJsonDocument::Indented));
     c_File.close();

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save to XML format

    \param[in]  orc_File    File path
    \param[in]  orc_Node    Node definition

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupNodeDefinitionFiler_New::h_SaveXml(
     const QString &orc_File,
     const C_OscSupNodeDefinition &orc_Node) {
     C_OscXmlParser c_XmlParser;

     // Root Node
     c_XmlParser.CreateAndSelectNodeChild(mc_ROOT_NAME);

     // File version
     c_XmlParser.CreateAndSelectNodeChild(mc_FILE_VERSION);
     c_XmlParser.SetNodeContent(QString::number(mu16_FILE_VERSION));
     c_XmlParser.SelectNodeParent();

     // Save files
     mh_SaveFiles(orc_Node.c_ApplicationFileNames, c_XmlParser, mc_FILES, mc_FILE);
     mh_SaveFiles(orc_Node.c_NvmFileNames, c_XmlParser, mc_PARAM_FILES, mc_PARAM_FILE);
     mh_SavePemConfig(orc_Node, c_XmlParser);
     mh_SaveSignatureFile(orc_Node, c_XmlParser);

     // Save to file
     const int32_t s32_Result = c_XmlParser.SaveToFile(orc_File);
     return (s32_Result == C_NO_ERR) ? C_NO_ERR : C_RD_WR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save node (helper)

    \param[in]  orc_File    File path
    \param[in]  orc_Node    Node definition

    \return
    C_NO_ERR   No error
    C_RD_WR    File(s) could not be created
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupNodeDefinitionFiler_New::mh_SaveNode(
     const QString &orc_File,
     const C_OscSupNodeDefinition &orc_Node) {
     const QString c_Extension = QFileInfo(orc_File).suffix().toLower();

     if (c_Extension == "bin") {
         return h_SaveBinary(orc_File, orc_Node);
     } else if (c_Extension == "json") {
         return h_SaveJson(orc_File, orc_Node);
     } else {
         return h_SaveXml(orc_File, orc_Node);
     }
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load files section from XML

    \param[in,out]  orc_Files              Files vector to append to
    \param[in]      ou32_NodeCounter       Current node index
    \param[in]      ou32_UpdatePos         Current update position
    \param[in,out]  orc_PositionMap        Map for node indices and update positions
    \param[in]      orc_NodeFolderAbs      Path where all the files will be unzipped to
    \param[in,out]  orc_XmlParser          XMLParser
    \param[in]      orc_BaseNodeName       XML node name to use on base level
    \param[in]      orc_ElementNodeName    XML node name to use on item level
 */
 //----------------------------------------------------------------------------------------------------------------------
 void C_OscSupNodeDefinitionFiler_New::mh_LoadFilesSection(
     QStringList &orc_Files,
     const uint32_t ou32_NodeCounter,
     const uint32_t ou32_UpdatePos,
     QMap<uint32_t, uint32_t> &orc_PositionMap,
     const QString &orc_NodeFolderAbs,
     C_OscXmlParserBase &orc_XmlParser,
     const QString &orc_BaseNodeName,
     const QString &orc_ElementNodeName) {
     if (orc_XmlParser.SelectNodeChild(orc_BaseNodeName) == orc_BaseNodeName) {
         orc_PositionMap.insert(ou32_NodeCounter, ou32_UpdatePos);

         Q_ASSERT(orc_XmlParser.SelectNodeChild(orc_ElementNodeName) == orc_ElementNodeName);

         QString c_SelectedNode;
         do {
             const QString c_XmlAttr = orc_XmlParser.GetAttributeString(mc_FILE_NAME_ATTR);
             const QString c_FilePath =
                 stw::opensyde_core::C_OscUtils::h_IncludeTrailingDelimiter(orc_NodeFolderAbs) + c_XmlAttr;
             orc_Files.push_back(c_FilePath);
             c_SelectedNode = orc_XmlParser.SelectNodeNext(orc_ElementNodeName);
         } while (c_SelectedNode == orc_ElementNodeName);

         Q_ASSERT(orc_XmlParser.SelectNodeParent() == orc_BaseNodeName);
         Q_ASSERT(orc_XmlParser.SelectNodeParent() == mc_ROOT_NAME);
     }
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load PEM configuration section from XML

    \param[in,out]  orc_DoFlash         Node configuration with PEM configuration
    \param[in]      ou32_NodeCounter    Current node index
    \param[in]      ou32_UpdatePos      Current update position
    \param[in,out]  orc_PositionMap     Map for node indices and update positions
    \param[in]      orc_NodeFolderAbs   Path where all the files will be unzipped to
    \param[in,out]  orc_XmlParser       XMLParser
 */
 //----------------------------------------------------------------------------------------------------------------------
 void C_OscSupNodeDefinitionFiler_New::mh_LoadPemConfigSection(
     C_OscSuSequences::C_DoFlash &orc_DoFlash,
     const uint32_t ou32_NodeCounter,
     const uint32_t ou32_UpdatePos,
     QMap<uint32_t, uint32_t> &orc_PositionMap,
     const QString &orc_NodeFolderAbs,
     C_OscXmlParserBase &orc_XmlParser) {
     if (orc_XmlParser.SelectNodeChild(mc_PEM_FILE_CONFIG) == mc_PEM_FILE_CONFIG) {
         Q_ASSERT(orc_XmlParser.SelectNodeChild(mc_PEM_FILE) == mc_PEM_FILE);

         const QString c_XmlAttr = orc_XmlParser.GetAttributeString(mc_FILE_NAME_ATTR);
         if (c_XmlAttr != "") {
             const QString c_FilePath =
                 stw::opensyde_core::C_OscUtils::h_IncludeTrailingDelimiter(orc_NodeFolderAbs) + c_XmlAttr;
             orc_DoFlash.c_PemFile = c_FilePath;
             orc_PositionMap.insert(ou32_NodeCounter, ou32_UpdatePos);
         }

         Q_ASSERT(orc_XmlParser.SelectNodeParent() == mc_PEM_FILE_CONFIG);

         if (orc_DoFlash.c_PemFile != "") {
             orc_DoFlash.q_SendSecurityEnabledState = orc_XmlParser.GetAttributeBool(
                 mc_PEM_FILE_CONFIG_SEC_SEND_ATTR, false);
             orc_DoFlash.q_SecurityEnabled = orc_XmlParser.GetAttributeBool(
                 mc_PEM_FILE_CONFIG_SEC_ENAB_ATTR, false);
             orc_DoFlash.q_SendDebuggerEnabledState = orc_XmlParser.GetAttributeBool(
                 mc_PEM_FILE_CONFIG_DEB_SEND_ATTR, false);
             orc_DoFlash.q_DebuggerEnabled = orc_XmlParser.GetAttributeBool(
                 mc_PEM_FILE_CONFIG_DEB_ENAB_ATTR, false);
         }

         Q_ASSERT(orc_XmlParser.SelectNodeParent() == mc_ROOT_NAME);
     }
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save files in XML

    \param[in]      orc_Files              Files to save
    \param[in,out]  orc_XmlParser          XMLParser
    \param[in]      orc_BaseNodeName       XML node name to use on base level
    \param[in]      orc_ElementNodeName    XML node name to use on item level
 */
 //----------------------------------------------------------------------------------------------------------------------
 void C_OscSupNodeDefinitionFiler_New::mh_SaveFiles(
     const QStringList &orc_Files,
     C_OscXmlParserBase &orc_XmlParser,
     const QString &orc_BaseNodeName,
     const QString &orc_ElementNodeName) {
     if (orc_Files.size() > 0) {
         Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild(orc_BaseNodeName) == orc_BaseNodeName);

         for (uint32_t u32_PosFile = 0; u32_PosFile < static_cast<uint32_t>(orc_Files.size()); ++u32_PosFile) {
             Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild(orc_ElementNodeName) == orc_ElementNodeName);
             orc_XmlParser.SetAttributeString(mc_FILE_NAME_ATTR, orc_Files[u32_PosFile]);
             Q_ASSERT(orc_XmlParser.SelectNodeParent() == orc_BaseNodeName);
         }

         Q_ASSERT(orc_XmlParser.SelectNodeParent() == mc_ROOT_NAME);
     }
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save PEM file configuration in XML

    \param[in]      orc_CurrentNode  Current node configuration with PEM file configuration
    \param[in,out]  orc_XmlParser    XMLParser
 */
 //----------------------------------------------------------------------------------------------------------------------
 void C_OscSupNodeDefinitionFiler_New::mh_SavePemConfig(
     const C_OscSupNodeDefinition &orc_CurrentNode,
     C_OscXmlParserBase &orc_XmlParser) {
     if (orc_CurrentNode.c_PemFile != "") {
         Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild(mc_PEM_FILE_CONFIG) == mc_PEM_FILE_CONFIG);
         orc_XmlParser.SetAttributeBool(mc_PEM_FILE_CONFIG_SEC_SEND_ATTR,
                                        orc_CurrentNode.q_SendSecurityEnabledState);
         orc_XmlParser.SetAttributeBool(mc_PEM_FILE_CONFIG_SEC_ENAB_ATTR,
                                        orc_CurrentNode.q_SecurityEnabled);
         orc_XmlParser.SetAttributeBool(mc_PEM_FILE_CONFIG_DEB_SEND_ATTR,
                                        orc_CurrentNode.q_SendDebuggerEnabledState);
         orc_XmlParser.SetAttributeBool(mc_PEM_FILE_CONFIG_DEB_ENAB_ATTR,
                                        orc_CurrentNode.q_DebuggerEnabled);

         Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild(mc_PEM_FILE) == mc_PEM_FILE);
         orc_XmlParser.SetAttributeString(mc_FILE_NAME_ATTR, orc_CurrentNode.c_PemFile);
         Q_ASSERT(orc_XmlParser.SelectNodeParent() == mc_PEM_FILE_CONFIG);
         Q_ASSERT(orc_XmlParser.SelectNodeParent() == mc_ROOT_NAME);
     }
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save signature file

    \param[in]      orc_Node         Node
    \param[in,out]  orc_XmlParser    Xml parser
 */
 //----------------------------------------------------------------------------------------------------------------------
 void C_OscSupNodeDefinitionFiler_New::mh_SaveSignatureFile(
     const C_OscSupNodeDefinition &orc_Node,
     C_OscXmlParserBase &orc_XmlParser) {
     if (orc_Node.u8_SignaturePresent == hu8_ACTIVE_NODE) {
         Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild(mc_SIG_FILE) == mc_SIG_FILE);
         orc_XmlParser.SetAttributeString(mc_SIG_FILE_ATTR, orc_Node.c_SignatureFile);
         Q_ASSERT(orc_XmlParser.SelectNodeParent() == mc_ROOT_NAME);
     }
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load signature file

    \param[in]      orc_NodeFolderAbs   Node folder abs
    \param[in,out]  orc_Signature       Signature
    \param[in,out]  orc_XmlParser       Xml parser

    \return
    C_NO_ERR   File read
    C_RD_WR    File not read
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupNodeDefinitionFiler_New::mh_LoadSignatureFile(
     const QString &orc_NodeFolderAbs,
     QString &orc_Signature,
     C_OscXmlParserBase &orc_XmlParser) {
     int32_t s32_Retval = C_NO_ERR;

     if (orc_XmlParser.SelectNodeChild(mc_SIG_FILE) == mc_SIG_FILE) {
         const QString c_XmlAttr = orc_XmlParser.GetAttributeString(mc_SIG_FILE_ATTR);
         const QString c_PackagePathTmp =
             stw::opensyde_core::C_OscUtils::h_IncludeTrailingDelimiter(orc_NodeFolderAbs) +
             QFileInfo(c_XmlAttr).fileName();
         s32_Retval = C_OscSupSignatureFiler_New::h_LoadSignatureFile(c_PackagePathTmp, orc_Signature);
         Q_ASSERT(orc_XmlParser.SelectNodeParent() == mc_ROOT_NAME);
     }
     return s32_Retval;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save nodes (legacy - XML only)

    \see h_SaveNodes
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupNodeDefinitionFiler_New::h_SaveNodesLegacy(
     const QStringList &orc_Files,
     const QList<C_OscSupNodeDefinition> &orc_Nodes) {
     int32_t s32_Retval = C_NO_ERR;

     if (orc_Nodes.size() == orc_Files.size()) {
         for (uint32_t u32_Pos = 0;
              (u32_Pos < static_cast<uint32_t>(orc_Nodes.size())) && (s32_Retval == C_NO_ERR);
              ++u32_Pos) {
             if (orc_Nodes[u32_Pos].u8_Active == hu8_ACTIVE_NODE) {
                 s32_Retval = mh_SaveNode(orc_Files[u32_Pos], orc_Nodes[u32_Pos]);
             }
         }
     } else {
         s32_Retval = C_CONFIG;
     }
     return s32_Retval;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load nodes (legacy - XML only)

    \see h_LoadNodes
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupNodeDefinitionFiler_New::h_LoadNodesLegacy(
     const QStringList &orc_Files,
     const QStringList &orc_NodeFoldersAbs,
     const QByteArray &orc_ActiveNodes,
     QList<C_OscSuSequences::C_DoFlash> &orc_ApplicationsToWrite,
     QMap<uint32_t, uint32_t> &orc_UpdateOrderByNodes,
     const QList<uint32_t> &orc_UpdatePosition,
     QStringList &orc_Signatures) {
     int32_t s32_Retval = C_NO_ERR;

     Q_ASSERT((orc_Files.size() == orc_NodeFoldersAbs.size()) &&
              ((orc_Files.size() == orc_ActiveNodes.size()) &&
               (orc_Files.size() == orc_UpdatePosition.size())));
     if ((orc_Files.size() == orc_NodeFoldersAbs.size()) &&
         ((orc_Files.size() == orc_ActiveNodes.size()) &&
          (orc_Files.size() == orc_UpdatePosition.size()))) {
         for (uint32_t u32_NodeCounter = 0;
              (u32_NodeCounter < static_cast<uint32_t>(orc_ActiveNodes.size())) &&
              (s32_Retval == C_NO_ERR);
              ++u32_NodeCounter) {
             QString c_Signature;
             C_OscSuSequences::C_DoFlash c_DoFlash;
             if (orc_ActiveNodes[u32_NodeCounter] == hu8_ACTIVE_NODE) {
                 C_OscXmlParser c_XmlParser;
                 s32_Retval = c_XmlParser.LoadFromFile(orc_Files[u32_NodeCounter]);
                 if (s32_Retval == C_NO_ERR) {
                     const uint32_t u32_UpdatePosition = orc_UpdatePosition[u32_NodeCounter];
                     Q_ASSERT(c_XmlParser.SelectRoot() == mc_ROOT_NAME);
                     mh_LoadFilesSection(c_DoFlash.c_FilesToFlash, u32_NodeCounter, u32_UpdatePosition,
                                         orc_UpdateOrderByNodes, orc_NodeFoldersAbs[u32_NodeCounter],
                                         c_XmlParser, mc_FILES, mc_FILE);
                     mh_LoadFilesSection(c_DoFlash.c_FilesToWriteToNvm, u32_NodeCounter, u32_UpdatePosition,
                                         orc_UpdateOrderByNodes, orc_NodeFoldersAbs[u32_NodeCounter],
                                         c_XmlParser, mc_PARAM_FILES, mc_PARAM_FILE);
                     mh_LoadPemConfigSection(c_DoFlash, u32_NodeCounter, u32_UpdatePosition,
                                             orc_UpdateOrderByNodes, orc_NodeFoldersAbs[u32_NodeCounter],
                                             c_XmlParser);
                     s32_Retval = mh_LoadSignatureFile(orc_NodeFoldersAbs[u32_NodeCounter],
                                                        c_Signature, c_XmlParser);
                 }
             }
             orc_ApplicationsToWrite.push_back(c_DoFlash);
             orc_Signatures.push_back(c_Signature);
         }
     } else {
         s32_Retval = C_CONFIG;
     }
     if (s32_Retval != C_NO_ERR) {
         s32_Retval = C_RD_WR;
     }
     return s32_Retval;
 }
