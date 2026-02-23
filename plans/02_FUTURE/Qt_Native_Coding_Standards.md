# openSYDE Qt-Native Coding Standards

**Created**: 2026-02-03
**Status**: RECOMMENDED STANDARDS
**Scope**: All new code and code being refactored
**Priority**: HIGH

---

## Purpose

This document establishes Qt-native coding standards for the openSYDE project. Following these standards ensures:
- Consistency with Qt 6 best practices
- Better integration with Qt APIs
- Improved code maintainability
- Reduced code duplication through Qt idioms

---

## Core Principle

**Use Qt-native solutions over STL equivalents when working in Qt code.**

While STL is perfectly valid C++, Qt provides optimized, integrated alternatives that work better within the Qt ecosystem.

---

## Container Standards

### ✅ PREFER Qt Containers

| Instead of | Use | Reason |
|------------|-----|--------|
| `std::vector<T>` | `QList<T>` | Implicit sharing, Qt 6 performance equivalent |
| `std::vector<QString>` | `QStringList` | Rich API (join, filter, etc.) |
| `std::map<QString, T>` | `QHash<QString, T>` | O(1) vs O(log n), better for string keys |
| `std::unordered_map<T1, T2>` | `QHash<T1, T2>` | Qt-native, implicit sharing |
| `std::set<T>` | `QSet<T>` | O(1) contains, Qt-native |
| `std::pair<T1, T2>` | `QPair<T1, T2>` | Qt-native (either is fine) |

### Examples

**Good** (Qt-native):
```cpp
QList<C_OscNode> mc_Nodes;
QStringList mc_FileNames;
QHash<QString, C_OscDataPool> mc_DataPools;
QSet<uint32_t> mc_UsedIds;
```

**Avoid** (mixed STL/Qt):
```cpp
std::vector<C_OscNode> mc_Nodes;          // Use QList
std::vector<QString> mc_FileNames;         // Use QStringList
std::map<QString, C_OscDataPool> mc_DataPools;  // Use QHash
std::set<uint32_t> mc_UsedIds;            // Use QSet
```

### When STL is Acceptable

**Use STL containers when**:
- Interfacing with non-Qt libraries (e.g., Vector BLF, DBC libraries)
- Template-heavy code where Qt containers cause issues
- Performance-critical code where profiling shows STL is faster (rare)
- Legacy API contracts (e.g., `std::list<C_OscSystemNameMaxCharLimitChangeReportItem>*` passed to external code)

**Provide conversion at boundaries**:
```cpp
// External API uses std::vector
std::vector<uint8_t> GetData() const {
   return mc_Data.toStdVector();  // Convert at API boundary
}

// Internal: use Qt containers
private:
   QList<uint8_t> mc_Data;
```

### Migration Strategy

**Current State**: All major container migrations are complete:
- `std::vector<QString>` → `QStringList`: ✅ Complete
- `std::map<K,V>` → `QHash<K,V>`: ✅ Complete (core GUI and system logic)
- `std::list<T>` → `QList<T>`: ✅ Complete (including change reporting)
- `std::set<T>` → `QSet<T>`: ✅ Complete where performance allowed
- `std::vector<uint8_t>` → `QList<uint8_t>`: ✅ Complete for internal buffers

**Remaining STL containers** are preserved intentionally:
- `std::set<uint16_t>` in `C_OscCanSignal`/`C_OscCanMessage` — for ordered bit-position tracking
- `std::set<uint32_t>` in `C_SdNdeDbProperties` — for fast UI selection
- `std::set<C_SdBueMlvSignalManager *>` in `C_SdBueMlvGraphicsScene` — for pointer deduplication
- `std::list<E_Change>` in `C_SdBueSignalPropertiesWidget` — for sequential change history
- `std::list<C_OscSystemNameMaxCharLimitChangeReportItem>` in `C_OscNode` — legacy API contract

**No further migration is planned**. All remaining STL uses are:
- Performance-critical
- Semantic (ordering, uniqueness)
- External API-compatible
- Low volume

**Strategy moving forward**:
1. **Preserve** remaining STL containers — do not refactor unless changing functionality
2. **New code**: Always use Qt containers (QList, QHash, QSet, QString)
3. **Refactor**: Only migrate STL containers when modifying the file — do not refactor for refactoring's sake
4. **Binary data**: Keep `std::vector<uint8_t>` — do not convert to QList<uint8_t> for protocol I/O

**Verification**:
- All changes are documented in `plans/00_ACTIVE/Container_Migration_Complete.md`
- Unit tests pass
- Build is clean
- clang-format applied
- No regression in performance

**Final Note**:
This document now reflects the **final state** of the container migration project. Future changes should follow the "preserved exceptions" listed above.

---

## String Standards

### ✅ PREFER QString

| Instead of | Use | Reason |
|------------|-----|--------|
| `std::string` | `QString` | Unicode support, Qt integration |
| `C_SclString` | `QString` | Legacy, being removed |
| `const char *` | `QString` | Safer, easier to use |

### Examples

**Good** (Qt-native):
```cpp
QString c_Name = "Node Name";
QString c_Path = QDir::homePath() + "/config.ini";
QString c_Number = QString::number(42);
```

**Avoid**:
```cpp
std::string c_Name = "Node Name";  // Use QString
C_SclString c_Path = "...";         // Being removed
```

### String Operations

**Prefer Qt methods**:
```cpp
// String joining
QStringList items = {"a", "b", "c"};
QString joined = items.join(",");  // "a,b,c"

// String splitting
QString data = "a,b,c";
QStringList parts = data.split(",");  // ["a", "b", "c"]

// String filtering
QStringList filtered = items.filter("prefix");

// String replacement
items.replaceInStrings("old", "new");

// String conversion
int value = text.toInt();
double val = text.toDouble();
```

---

## File I/O Standards

### ✅ PREFER Qt File APIs

| Instead of | Use | Reason |
|------------|-----|--------|
| `std::ifstream/ofstream` | `QFile + QTextStream` | Better Unicode, Qt integration |
| `std::filesystem::path` | `QString + QFileInfo` | Cross-platform, Qt-native |
| `std::filesystem::exists` | `QFileInfo::exists()` | Simpler API |
| `std::filesystem::file_size` | `QFileInfo::size()` | Qt-native |

### Examples

**Good** (Qt-native):
```cpp
// Reading text file
QFile file(filePath);
if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
   QTextStream stream(&file);
   while (!stream.atEnd()) {
      QString line = stream.readLine();
      // Process line
   }
}

// Writing text file
QFile file(filePath);
if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
   QTextStream stream(&file);
   stream << "Content" << Qt::endl;
}

// File metadata
QFileInfo fileInfo(filePath);
if (fileInfo.exists()) {
   qint64 size = fileInfo.size();
   QDateTime modified = fileInfo.lastModified();
}

// Directory operations
QDir dir(path);
dir.mkpath(".");  // Create directory tree
QStringList files = dir.entryList(QDir::Files);
```

**Avoid**:
```cpp
// STL file I/O
std::ifstream file(path.toStdString());  // Use QFile
std::filesystem::exists(path.toStdString());  // Use QFileInfo::exists()
```

---

## Serialization Standards

### ✅ PREFER Qt Serialization

| Format | Use Case | API |
|--------|----------|-----|
| **QDataStream** | Binary, cache, performance | `QDataStream` |
| **QJsonDocument** | Config files, modern | `QJsonDocument`, `QJsonObject` |
| **XML** | Project files, compatibility | TinyXML2 (existing), QXmlStreamReader (new) |

### Examples

**Binary Serialization** (fast, internal use):
```cpp
// Save
QFile file("cache.dat");
file.open(QIODevice::WriteOnly);
QDataStream stream(&file);
stream.setVersion(QDataStream::Qt_6_0);
stream << mc_Nodes << mc_DataPools;

// Load
QFile file("cache.dat");
file.open(QIODevice::ReadOnly);
QDataStream stream(&file);
stream >> mc_Nodes >> mc_DataPools;
```

**JSON Serialization** (modern, config):
```cpp
// Save
QJsonObject obj;
obj["name"] = mc_Name;
obj["enabled"] = mq_Enabled;
QJsonDocument doc(obj);
QFile file("config.json");
file.open(QIODevice::WriteOnly);
file.write(doc.toJson());

// Load
QFile file("config.json");
file.open(QIODevice::ReadOnly);
QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
QJsonObject obj = doc.object();
mc_Name = obj["name"].toString();
mq_Enabled = obj["enabled"].toBool();
```

---

## GUI Standards

### ✅ PREFER Qt Framework Patterns

#### 1. Use QSS for Styling

**Good** (declarative QSS):
```css
/* In styles.qss */
QPushButton#primaryButton {
   background-color: #0066CC;
   color: white;
   border-radius: 4px;
   padding: 8px 16px;
}

QPushButton#primaryButton:hover {
   background-color: #0052A3;
}
```

```cpp
// In C++ - just set object name
button->setObjectName("primaryButton");
// Styling handled by QSS
```

**Avoid** (C++ styling code):
```cpp
// Don't do manual styling in C++
void paintEvent(QPaintEvent * event) {
   QPainter painter(this);
   painter.setBrush(QColor(0, 102, 204));
   // ... manual painting
}
```

#### 2. Use Qt Property System

**Good** (Qt properties):
```cpp
class C_OgeLabHeading : public QLabel {
   Q_OBJECT
   Q_PROPERTY(QString styleVariant READ styleVariant WRITE setStyleVariant)

public:
   void setStyleVariant(const QString & variant) {
      m_StyleVariant = variant;
      style()->unpolish(this);
      style()->polish(this);
   }

private:
   QString m_StyleVariant = "default";
};

// Usage
label->setProperty("styleVariant", "bold");

// QSS responds to property
QLabel[styleVariant="bold"] { font-weight: bold; }
```

**Avoid** (multiple subclasses):
```cpp
// Don't create separate classes for styling variants
class C_OgeLabBold : public QLabel { };
class C_OgeLabItalic : public QLabel { };
class C_OgeLabHeading1 : public QLabel { };
// ... 70 label types for different styles
```

#### 3. Use Qt Signals/Slots

**Good** (Qt signals/slots):
```cpp
class C_ProtocolEngine : public QObject {
   Q_OBJECT

signals:
   void SigMessageReceived(const QString & orc_Message);
   void SigError(const QString & orc_Error);

public slots:
   void ProcessMessage(const T_STWCAN_Msg_RX & orc_Msg);
};

// Usage
connect(engine, &C_ProtocolEngine::SigMessageReceived,
        this, &MyClass::OnMessageReceived);
```

**Avoid** (function pointers/callbacks):
```cpp
// Don't use raw function pointers
typedef void (*MessageCallback)(const QString &);
void SetCallback(MessageCallback callback);
```

#### 4. Use Qt Model/View

**Good** (Qt Model/View):
```cpp
class C_DataPoolTableModel : public QAbstractTableModel {
   Q_OBJECT

public:
   int rowCount(const QModelIndex & parent) const override;
   int columnCount(const QModelIndex & parent) const override;
   QVariant data(const QModelIndex & index, int role) const override;
   bool setData(const QModelIndex & index, const QVariant & value, int role) override;

   // Qt handles view updates automatically via signals
};

// Usage
tableView->setModel(new C_DataPoolTableModel(this));
// View updates automatically when model changes
```

**Avoid** (manual view updates):
```cpp
// Don't manually update views
void UpdateCell(int row, int col, const QString & value) {
   QTableWidgetItem * item = table->item(row, col);
   item->setText(value);
   // ... manual update logic
}
```

---

## Thread Safety

### ✅ PREFER Qt Threading

| Instead of | Use | Reason |
|------------|-----|--------|
| `std::thread` | `QThread` | Qt integration, signals/slots |
| `std::mutex` | `QMutex` | Qt-native |
| `std::condition_variable` | `QWaitCondition` | Qt-native |

### Example

**Good** (Qt threading):
```cpp
class Worker : public QObject {
   Q_OBJECT

signals:
   void SigFinished();
   void SigProgress(int percentage);

public slots:
   void DoWork() {
      // Work on separate thread
      emit SigProgress(50);
      // ...
      emit SigFinished();
   }
};

// Usage
QThread * thread = new QThread;
Worker * worker = new Worker;
worker->moveToThread(thread);

connect(thread, &QThread::started, worker, &Worker::DoWork);
connect(worker, &Worker::SigFinished, thread, &QThread::quit);

thread->start();
```

---

## Migration Guidelines

### When Refactoring Code

1. **If touching containers**: Migrate to Qt containers
2. **If touching strings**: Migrate to QString
3. **If touching file I/O**: Migrate to QFile
4. **If adding styling**: Use QSS, not C++ code
5. **If adding events**: Use Qt signals/slots

### Don't

- Don't refactor code that isn't being changed (avoid churn)
- Don't break external APIs without approval
- Don't mix Qt and STL in the same class (be consistent)

### Do

- Do migrate opportunistically (when already editing a file)
- Do use Qt idioms to reduce code
- Do document why STL is used (if necessary)

---

## Code Review Checklist

### For Reviewers

- [ ] Are Qt containers used instead of STL? (QList, QHash, QSet)
- [ ] Is QString used instead of std::string?
- [ ] Is QFile used for file I/O?
- [ ] Is QSS used for styling (not C++ code)?
- [ ] Are Qt signals/slots used for events?
- [ ] Is Qt Model/View used for tables?
- [ ] Are Qt properties used for configuration?

### For Authors

Before submitting code:
- [ ] I used Qt containers where appropriate
- [ ] I used QString for all string operations
- [ ] I used QFile for file I/O
- [ ] I used QSS for any styling needs
- [ ] I used Qt signals/slots for events
- [ ] I followed Qt naming conventions (camelCase for methods)

---

## Reference: Qt vs STL Quick Lookup

| Task | Qt Solution | STL Equivalent |
|------|-------------|----------------|
| Dynamic array | `QList<T>` | `std::vector<T>` |
| String list | `QStringList` | `std::vector<QString>` |
| Hash map | `QHash<K, V>` | `std::unordered_map<K, V>` |
| Ordered map | `QMap<K, V>` | `std::map<K, V>` |
| Set | `QSet<T>` | `std::set<T>` / `std::unordered_set<T>` |
| String | `QString` | `std::string` |
| File I/O | `QFile` + `QTextStream` | `std::ifstream/ofstream` |
| Path | `QString` + `QFileInfo` | `std::filesystem::path` |
| JSON | `QJsonDocument` | External library |
| Threading | `QThread` | `std::thread` |
| Mutex | `QMutex` | `std::mutex` |

---

## Benefits Summary

### Why Qt-Native?

1. **Implicit Sharing**: QList, QHash copy-on-write reduces memory
2. **Richer APIs**: QStringList::join(), filter() vs manual loops
3. **Better Integration**: Works seamlessly with Qt functions
4. **Cross-Platform**: Qt handles platform differences
5. **Consistent**: One approach, not mixed STL/Qt
6. **Signals/Slots**: Thread-safe event communication
7. **Model/View**: Automatic view updates
8. **QSS**: Declarative styling

---

## Getting Help

### Qt Documentation
- [Qt Containers](https://doc.qt.io/qt-6/containers.html)
- [QString](https://doc.qt.io/qt-6/qstring.html)
- [QFile](https://doc.qt.io/qt-6/qfile.html)
- [Model/View Programming](https://doc.qt.io/qt-6/model-view-programming.html)
- [Qt Style Sheets](https://doc.qt.io/qt-6/stylesheet.html)

### Questions?
- Ask in code reviews
- Refer to Phase 1-3 implementation plans
- See `Qt_Native_Prioritization_Strategy.md`

---

**Document Status**: RECOMMENDED STANDARDS
**Enforcement**: Code review
**Exceptions**: Document in code comments (with rationale)
**Created**: 2026-02-03
**Owner**: Development Team
