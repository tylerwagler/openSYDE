## Task: Migrate std::map<uint32_t, QString> to QHash in C_OscSystemDefinitionFiler.hpp and .cpp

### Objective
Migrate all instances of `std::map<uint32_t, QString>` to `QHash<uint32_t, QString>` in the files `C_OscSystemDefinitionFiler.hpp` and `C_OscSystemDefinitionFiler.cpp`.

### Implementation Plan
1. Open `C_OscSystemDefinitionFiler.hpp` and `C_OscSystemDefinitionFiler.cpp` in a text editor.
2. In `C_OscSystemDefinitionFiler.hpp`, locate the private static method:
   ```cpp
   static std::map<uint32_t, QString> mh_MapNodeIndicesToName(const QList<C_OscNode> & orc_Nodes);
   ```
   Replace it with:
   ```cpp
   static QHash<uint32_t, QString> mh_MapNodeIndicesToName(const QList<C_OscNode> & orc_Nodes);
   ```
3. In `C_OscSystemDefinitionFiler.cpp`, locate the implementation:
   ```cpp
   std::map<uint32_t, QString> C_OscSystemDefinitionFiler::mh_MapNodeIndicesToName(
       const QList<C_OscNode> &orc_Nodes) {
     std::map<uint32_t, QString> c_Retval;
     for (uint32_t u32_It = 0UL; u32_It < orc_Nodes.size(); ++u32_It) {
       const C_OscNode &rc_Node = orc_Nodes[u32_It];
       c_Retval[u32_It] = rc_Node.c_Properties.c_Name;
     }
     return c_Retval;
   }
   ```
   Replace it with:
   ```cpp
   QHash<uint32_t, QString> C_OscSystemDefinitionFiler::mh_MapNodeIndicesToName(
       const QList<C_OscNode> &orc_Nodes) {
     QHash<uint32_t, QString> c_Retval;
     for (uint32_t u32_It = 0UL; u32_It < orc_Nodes.size(); ++u32_It) {
       const C_OscNode &rc_Node = orc_Nodes[u32_It];
       c_Retval.insert(u32_It, rc_Node.c_Properties.c_Name);
     }
     return c_Retval;
   }
   ```
4. Save the changes to both files.

### Verification Steps
1. Verify the current usage:
   ```bash
   grep -n "std::map<uint32_t, QString>" opensyde_tool/libs/opensyde_core/project/system/C_OscSystemDefinitionFiler.*
   ```
   Confirm only one match in each file.
2. Run `clang-format -i --style=file opensyde_tool/libs/opensyde_core/project/system/C_OscSystemDefinitionFiler.*`
3. Build the project with `.\build.ps1`. Ensure no compilation errors.
4. Verify that the code still compiles and links successfully.

### Testing Instructions
1. Run unit tests:
   ```bash
   cd opensyde_tool/test/system_definition
   cmake --build . --target test_system_definition
   ```
2. Execute the test binary and confirm all tests pass.
3. Test system definition loading/saving in the GUI: 
   - Load a sample `.osd` file.
   - Modify a node name.
   - Save the file.
   - Reopen the file — verify all data is preserved.

### Notes
- This change is **non-breaking** because `QHash` and `std::map` have compatible semantics for this use case (key-value lookup).
- `QHash::insert(key, value)` replaces `std::map::operator[]` for assignment.
- No further changes needed to calling code — the return type is internal.
- **Do not commit until tests pass**.
- Reference: `plans/02_FUTURE/Qt_Native_Coding_Standards.md` — Section 2.2: Containers
- **Commit message suggestion**: `[opensyde_core] Migrate std::map<uint32_t, QString> to QHash in C_OscSystemDefinitionFiler`
- **Performance note**: `QHash` provides average O(1) lookup, identical to `std::map` in this context; no performance regression expected.