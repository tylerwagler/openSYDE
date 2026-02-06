# QString Migration Guidelines

**Version**: 1.0
**Date**: 2026-02-03
**Scope**: openSYDE QString Migration (Phases 1-4)
**Audience**: Developers working on QString migration

---

## Table of Contents

1. [When to Migrate vs. When to Defer](#when-to-migrate-vs-when-to-defer)
2. [Migration Patterns](#migration-patterns)
3. [API Boundary Handling](#api-boundary-handling)
4. [Testing Requirements](#testing-requirements)
5. [Common Pitfalls](#common-pitfalls)
6. [Code Review Checklist](#code-review-checklist)
7. [Performance Considerations](#performance-considerations)

---

## When to Migrate vs. When to Defer

### ✅ MIGRATE NOW

**Local Variables**:
```cpp
// Simple case - no external dependencies
void someFunction() {
    C_SclString c_Text = "hello";  // ✅ MIGRATE
    c_Text = c_Text.UpperCase();
}

// After migration:
void someFunction() {
    QString c_Text = "hello";
    c_Text = c_Text.toUpper();
}
```

**Function Parameters (Internal Functions)**:
```cpp
// Private/protected helper functions
private:
    void mh_Helper(const C_SclString & orc_Text);  // ✅ MIGRATE
```

**Return Values (Internal Functions)**:
```cpp
// Internal utility with no external callers
C_SclString mh_ProcessText();  // ✅ MIGRATE to QString
```

**C_SclStringList Usage**:
```cpp
// ALWAYS migrate C_SclStringList → QStringList
C_SclStringList c_Items;  // ✅ MIGRATE
QStringList c_Items;      // Correct
```

### ⏸️ DEFER (Requires Coordination)

**Public API Functions**:
```cpp
// Public methods used by multiple modules
public:
    int32_t ProcessFile(const C_SclString & orc_Path);  // ⏸️ DEFER
    // Reason: May have many external callers
```

**Library Interfaces**:
```cpp
// Functions in exported library headers
class STW_EXPORT C_MyClass {
public:
    void DoWork(const C_SclString & orc_Input);  // ⏸️ DEFER
    // Reason: External API, requires coordination
};
```

**Performance-Critical Code**:
```cpp
// Hot path with high-frequency calls
void ProcessHighFrequency(const C_SclString & orc_Data) {
    // Called 1000s of times per second
}  // ⏸️ DEFER
// Reason: Needs performance testing
```

### ❌ DO NOT MIGRATE

**KEFEX Legacy Library**:
```cpp
// kefex_diaglib/ directory
namespace stw::diag_lib {
    // Keep C_SclString - legacy protocol
}  // ❌ DO NOT MIGRATE
// Reason: Stable legacy code, high risk
```

**SCL Library Definitions**:
```cpp
// scl/C_SclString.cpp, scl/C_SclStringList.cpp
// ❌ DO NOT MIGRATE
// Reason: Class implementations, keep until full deprecation
```

---

## Migration Patterns

### Pattern 1: Simple Type Replacement

```cpp
// BEFORE:
C_SclString c_Name = "test";
c_Name = c_Name.UpperCase();
int32_t s32_Len = c_Name.Length();
const char_t * pcn_Str = c_Name.c_str();

// AFTER:
QString c_Name = "test";
c_Name = c_Name.toUpper();
int32_t s32_Len = c_Name.length();
const char_t * pcn_Str = c_Name.toUtf8().constData();
```

### Pattern 2: Method Name Changes

| C_SclString | QString | Notes |
|-------------|---------|-------|
| `.Length()` | `.length()` | Or `.size()` |
| `.c_str()` | `.toUtf8().constData()` | For const char* |
| `.UpperCase()` | `.toUpper()` | Returns copy |
| `.LowerCase()` | `.toLower()` | Returns copy |
| `.Trim()` | `.trimmed()` | Returns copy |
| `.IsEmpty()` | `.isEmpty()` | |
| `.Pos(str)` | `.indexOf(str) + 1` | **Note +1!** |
| `.LastPos(str)` | `.lastIndexOf(str) + 1` | **Note +1!** |
| `.SubString(start, len)` | `.mid(start-1, len)` | **Note start-1!** |
| `.Delete(pos, len)` | `.remove(pos-1, len)` | **Note pos-1!** |
| `.Insert(str, pos)` | `.insert(pos-1, str)` | **Note pos-1!** |
| `.ToInt()` | `.toInt()` | Same |
| `.IntToStr(n)` | `QString::number(n)` | Static method |
| `.IntToHex(n, width)` | `QString::asprintf("%0*X", width, n)` | Format string |
| `.PrintFormatted(fmt, ...)` | `QString::asprintf(fmt, ...)` | Modern C++ |

### Pattern 3: C_SclStringList → QStringList

```cpp
// BEFORE:
C_SclStringList c_Items;
c_Items.Clear();
c_Items.Add("item1");
c_Items.Add("item2");
for (uint32_t u32_Idx = 0; u32_Idx < c_Items.GetCount(); u32_Idx++) {
    const C_SclString c_Item = c_Items.Strings[u32_Idx];
}
c_Items.Delete(0);
c_Items.LoadFromFile(c_Path);

// AFTER:
QStringList c_Items;
c_Items.clear();
c_Items.append("item1");
c_Items.append("item2");
for (uint32_t u32_Idx = 0; u32_Idx < static_cast<uint32_t>(c_Items.count()); u32_Idx++) {
    const QString c_Item = c_Items.at(u32_Idx);  // or c_Items[u32_Idx]
}
c_Items.removeAt(0);
// LoadFromFile needs manual implementation (see Pattern 5)
```

### Pattern 4: std::vector<C_SclString> → std::vector<QString>

```cpp
// BEFORE:
void ProcessFiles(const std::vector<C_SclString> & orc_Files) {
    for (const C_SclString & rc_File : orc_Files) {
        DoWork(rc_File);
    }
}

// AFTER:
void ProcessFiles(const std::vector<QString> & orc_Files) {
    for (const QString & rc_File : orc_Files) {
        DoWork(rc_File);
    }
}
```

### Pattern 5: LoadFromFile Replacement

```cpp
// BEFORE:
C_SclStringList c_Lines;
c_Lines.LoadFromFile(c_Path);

// AFTER:
QStringList c_Lines;
QFile c_File(c_Path);
if (c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream c_Stream(&c_File);
    while (!c_Stream.atEnd()) {
        c_Lines.append(c_Stream.readLine());
    }
    c_File.close();
}
```

### Pattern 6: SaveToFile Replacement

```cpp
// BEFORE:
C_SclStringList c_Lines;
c_Lines.Add("line1");
c_Lines.SaveToFile(c_Path);

// AFTER:
QStringList c_Lines;
c_Lines.append("line1");
QFile c_File(c_Path);
if (c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream c_Stream(&c_File);
    for (const QString & rc_Line : c_Lines) {
        c_Stream << rc_Line << "\n";
    }
    c_File.close();
}
```

---

## API Boundary Handling

### Strategy 1: Convert at Boundary (Preferred)

**When**: XML parser, external library returns C_SclString
**How**: Convert immediately to QString

```cpp
// BEFORE:
const C_SclString c_Value = xmlParser.GetNodeContent();
oru32_Result = static_cast<uint32_t>(c_Value.ToInt());

// AFTER:
const QString c_Value = xmlParser.GetNodeContent().ToQString();
oru32_Result = static_cast<uint32_t>(c_Value.toInt());
```

### Strategy 2: Wrapper Functions

**When**: Multiple call sites need same conversion
**How**: Create conversion helper

```cpp
// Helper function:
QString h_GetXmlNodeContentAsQString(C_OscXmlParserBase & orc_Parser) {
    return orc_Parser.GetNodeContent().ToQString();
}

// Usage:
const QString c_Value = h_GetXmlNodeContentAsQString(c_Parser);
```

### Strategy 3: Coordinated Migration

**When**: Function signature change affects many callers
**How**: Update all files in single commit

```cpp
// Step 1: Update header
// File: MyClass.hpp
void ProcessData(const std::vector<QString> & orc_Data);  // Was C_SclString

// Step 2: Update implementation
// File: MyClass.cpp
void MyClass::ProcessData(const std::vector<QString> & orc_Data) {
    // Update implementation
}

// Step 3: Update ALL callers in same commit
// Files: Caller1.cpp, Caller2.cpp, ...
obj.ProcessData(c_QStringVector);  // Update all call sites
```

---

## Testing Requirements

### Level 1: Build Verification (Minimum)

**For**: Low-risk local variable changes
**Test**:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
# Verify no compilation errors
```

### Level 2: Functional Testing (Standard)

**For**: Function parameter changes, moderate risk
**Test**:
1. Build all affected targets
2. Manual smoke test of affected features
3. Verify file I/O if LoadFromFile/SaveToFile changed
4. Check string manipulation correctness

**Example Test Cases**:
- Load configuration file → verify contents correct
- Generate output file → diff against expected
- Process string data → check transformations

### Level 3: Integration Testing (High Risk)

**For**: API changes, system_update_package, security
**Test**:
1. Full build all targets
2. Automated test suite if available
3. End-to-end workflow testing
4. Error path testing
5. Boundary condition testing

**Example Test Cases (system_update_package)**:
- Create update package with encryption
- Load package and verify contents
- Multi-node update sequence
- Error handling: missing files, invalid format
- Boundary: empty files, large files, special characters

### Level 4: Performance Testing (Performance-Critical)

**For**: High-frequency code paths
**Test**:
1. Benchmark before migration
2. Benchmark after migration
3. Compare timing (should be similar or better)
4. Profile memory usage

---

## Common Pitfalls

### Pitfall 1: Indexing Differences

**Problem**: C_SclString uses 1-based indexing, QString uses 0-based

```cpp
// ❌ WRONG:
// C_SclString::Pos() returns 1-based (0 = not found)
// QString::indexOf() returns 0-based (-1 = not found)

C_SclString c_Str = "hello";
int32_t s32_Pos = c_Str.Pos("ll");  // Returns 3

QString c_Str = "hello";
int32_t s32_Pos = c_Str.indexOf("ll");  // Returns 2 ❌ Different!

// ✅ CORRECT:
int32_t s32_Pos = c_Str.indexOf("ll") + 1;  // Returns 3 (C_SclString compatible)
```

### Pitfall 2: c_str() Lifetime

**Problem**: QString::toUtf8().constData() lifetime

```cpp
// ❌ WRONG:
const char_t * pcn_Data;
{
    QString c_Temp = "test";
    pcn_Data = c_Temp.toUtf8().constData();  // ❌ Dangling pointer!
}
// pcn_Data now points to freed memory

// ✅ CORRECT Option 1 - Keep QByteArray alive:
QString c_Temp = "test";
QByteArray c_Bytes = c_Temp.toUtf8();
const char_t * pcn_Data = c_Bytes.constData();  // Safe while c_Bytes alive

// ✅ CORRECT Option 2 - Use QString directly:
QString c_Temp = "test";
// Pass c_Temp directly if function accepts QString
```

### Pitfall 3: Immutable Methods

**Problem**: QString methods return copies, don't modify in place

```cpp
// ❌ WRONG:
QString c_Text = "hello";
c_Text.toUpper();  // ❌ Doesn't modify c_Text!
// c_Text is still "hello"

// ✅ CORRECT:
QString c_Text = "hello";
c_Text = c_Text.toUpper();  // Assign result
// c_Text is now "HELLO"

// Or use in-place Qt methods when available:
// (Note: Most QString methods return copies for const-correctness)
```

### Pitfall 4: GetCount() Type Change

**Problem**: C_SclStringList::GetCount() returns uint32_t, QStringList::count() returns int

```cpp
// ❌ POTENTIAL WARNING:
QStringList c_Items;
for (uint32_t u32_Idx = 0; u32_Idx < c_Items.count(); u32_Idx++) {
    // Warning: comparison signed/unsigned
}

// ✅ CORRECT:
for (uint32_t u32_Idx = 0; u32_Idx < static_cast<uint32_t>(c_Items.count()); u32_Idx++) {
    // Explicit cast
}

// Or use int:
for (int s32_Idx = 0; s32_Idx < c_Items.count(); s32_Idx++) {
    // Use int to match count() return type
}
```

### Pitfall 5: AddStrings vs. Append

**Problem**: C_SclStringList::AddStrings() takes pointer, QStringList uses += or append(list)

```cpp
// BEFORE:
C_SclStringList c_List1, c_List2;
c_List1.AddStrings(&c_List2);  // Takes pointer

// AFTER:
QStringList c_List1, c_List2;
c_List1 += c_List2;  // Operator overload

// Or:
c_List1.append(c_List2);  // Append method (Qt 5.14+)
```

### Pitfall 6: Namespace Confusion

**Problem**: Don't create types in wrong namespace

```cpp
// ❌ WRONG:
stw::scl::QStringList c_Items;  // ❌ QStringList not in stw::scl!
stw::scl::QString c_Text;       // ❌ QString not in stw::scl!

// ✅ CORRECT:
QStringList c_Items;  // QStringList is in global Qt namespace
QString c_Text;       // QString is in global Qt namespace
```

---

## Code Review Checklist

### Pre-Commit Checklist (Developer)

- [ ] All C_SclString → QString replacements correct
- [ ] Method names updated (.Length() → .length(), etc.)
- [ ] Index arithmetic correct (Pos +1, SubString -1)
- [ ] .c_str() replaced with .toUtf8().constData() correctly
- [ ] QByteArray lifetime managed if needed
- [ ] C_SclStringList → QStringList complete
- [ ] .GetCount() cast to uint32_t if needed
- [ ] .Strings[] → .at() or []
- [ ] std::vector<C_SclString> → std::vector<QString>
- [ ] No stw::scl::QString or stw::scl::QStringList
- [ ] Builds without errors or warnings
- [ ] Appropriate tests run and passing
- [ ] Git commit message follows conventions

### Code Review Checklist (Reviewer)

**Correctness**:
- [ ] Index arithmetic verified (1-based vs 0-based)
- [ ] .c_str() lifetime issues checked
- [ ] QString immutability handled (assignment of return values)
- [ ] Casts added where signed/unsigned comparison

**Completeness**:
- [ ] All occurrences in file migrated (or justified why not)
- [ ] Function signatures updated in .hpp and .cpp
- [ ] All callers updated if API changed
- [ ] Include statements updated (#include <QString>, #include <QStringList>)

**Testing**:
- [ ] Appropriate test level performed
- [ ] Build verification at minimum
- [ ] Functional tests for medium-risk changes
- [ ] Integration tests for high-risk changes

**Documentation**:
- [ ] Commit message describes changes
- [ ] API changes documented if public interface
- [ ] Migration pattern followed from guidelines

---

## Performance Considerations

### QString vs. C_SclString

**General**: QString performance is comparable or better than C_SclString

**UTF-8 Encoding**:
- QString internally uses UTF-16
- `.toUtf8()` has conversion overhead
- **Advice**: Minimize repeated .toUtf8() calls

```cpp
// ❌ INEFFICIENT:
for (int i = 0; i < 1000; i++) {
    SomeFunction(c_QString.toUtf8().constData());  // 1000 conversions
}

// ✅ EFFICIENT:
QByteArray c_Bytes = c_QString.toUtf8();
for (int i = 0; i < 1000; i++) {
    SomeFunction(c_Bytes.constData());  // 1 conversion
}
```

**String Building**:
- Use QString::arg() or QStringBuilder for concatenation
- Avoid repeated operator+

```cpp
// ❌ INEFFICIENT:
QString c_Result = c_Str1 + c_Str2 + c_Str3 + c_Str4;  // Multiple allocations

// ✅ EFFICIENT:
QString c_Result = QString("%1%2%3%4").arg(c_Str1, c_Str2, c_Str3, c_Str4);

// Or enable QStringBuilder:
// (Already enabled in Qt 6 by default)
QString c_Result = c_Str1 % c_Str2 % c_Str3 % c_Str4;
```

**Memory**:
- QString uses implicit sharing (COW - Copy on Write)
- Passing by const reference is cheap
- No need for QString* in most cases

### When to Profile

**Profile if**:
- Function called > 1000 times per second
- String processing in real-time path
- Large file parsing (> 10 MB)
- Network protocol encoding/decoding

**How to Profile**:
```cpp
#include <QElapsedTimer>

QElapsedTimer c_Timer;
c_Timer.start();

// Code to profile
ProcessStrings(c_Data);

qDebug() << "Elapsed:" << c_Timer.elapsed() << "ms";
```

---

## Decision Tree: Should I Migrate This?

```
START: Found C_SclString usage
    |
    ├─> Is it in kefex_diaglib/?
    |       └─> YES → ❌ DO NOT MIGRATE
    |
    ├─> Is it in scl/*.cpp (class definition)?
    |       └─> YES → ❌ DO NOT MIGRATE
    |
    ├─> Is it C_SclStringList?
    |       └─> YES → ✅ MIGRATE (high priority)
    |
    ├─> Is it local variable?
    |       └─> YES → ✅ MIGRATE (low risk)
    |
    ├─> Is it private/protected function parameter?
    |       └─> YES → ✅ MIGRATE (medium risk, check callers)
    |
    ├─> Is it public API with < 5 external callers?
    |       └─> YES → ✅ MIGRATE (coordinate with callers)
    |
    ├─> Is it public API with many external callers?
    |       └─> YES → ⏸️ DEFER (requires coordination)
    |
    ├─> Is it performance-critical (>1000 calls/sec)?
    |       └─> YES → ⏸️ DEFER (requires profiling)
    |
    └─> Default → ✅ MIGRATE (proceed with caution)
```

---

## Examples from Real Migrations

### Example 1: C_OscBuSequences.cpp (Sprint 2)

**What Changed**:
- Local variable C_SclStringList → QStringList
- Methods: .Clear() → .clear(), .Add() → .append()
- Array access: .Strings[i] → .at(i)
- Count with cast: static_cast<uint32_t>(c_Text.count())

**Result**: Clean migration, builds successfully

### Example 2: C_OscSupDefinitionFiler.cpp (Sprint 3)

**What Changed**:
- std::vector<C_SclString> → std::vector<QString> in function params
- XML parser boundary: GetNodeContent().ToQString()
- GetAttributeString().ToQString()

**Pattern Established**: Always convert at XML parser boundary

### Example 3: CKFXProjectOptions.cpp (Sprint 2)

**What Changed**:
- Member variable C_SclStringList c_Text → QStringList c_Text
- .Append(c_SclString) → .append(c_SclString.ToQString())
- .GetText() comparison → direct QStringList comparison

**Result**: Simpler code with QStringList built-in operators

---

## Additional Resources

**Qt Documentation**:
- QString: https://doc.qt.io/qt-6/qstring.html
- QStringList: https://doc.qt.io/qt-6/qstringlist.html
- QTextStream: https://doc.qt.io/qt-6/qtextstream.html

**Project Documentation**:
- CLAUDE.md: Project overview and conventions
- plans/QString_Migration_Master_Plan.md: Overall strategy
- plans/QString_Migration_Phase4_Audit.md: Remaining work analysis
- plans/QString_Migration_Phase4_Priority_Matrix.md: Priority rankings

**Git History**:
- Sprint 1 commits: C_SclStringList patterns
- Sprint 2 commits: Protocol driver migrations
- Sprint 3 commits: std::vector<C_SclString> patterns

---

## FAQ

**Q: Do I need to migrate everything in a file at once?**
A: No, but prefer complete file migration when possible. Partial migration is acceptable if some APIs must be deferred.

**Q: What if QString migration breaks something?**
A: Git revert the commit. File a bug report. Add to "high-risk" list for Phase 5.

**Q: Can I mix C_SclString and QString in the same file?**
A: Yes, temporarily. Convert at API boundaries. Long-term goal is full migration.

**Q: Should I migrate comments and log messages?**
A: Yes if they reference method names (.Length() → .length()), otherwise optional.

**Q: What about QString in GUI layer (opensyde_tool/src/)?**
A: Out of scope for Phase 1-4. GUI layer already uses QString extensively.

**Q: Performance regressed after migration. What do I do?**
A: Profile to find bottleneck. Check for repeated .toUtf8() calls. Optimize hot path.

---

**Version History**:
- 1.0 (2026-02-03): Initial version based on Phase 1-3 experience

**Maintained By**: openSYDE QString Migration Team
**Questions**: Refer to git commit messages or project maintainers
