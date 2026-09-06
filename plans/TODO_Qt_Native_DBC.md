# TODO: Migrate from Vector DBC Library to Qt Native DBC Parsing

## Overview
The current implementation uses the Vector DBC library (`Vector::DBC`) for CAN database parsing and export. Qt 6.x now provides native support for CAN bus configuration and DBC file parsing through the `QtCAN` module, which should replace the external Vector dependency.

## Current State
- **Location**: `opensyde_tool/src/com_import_export/`
  - `C_CieExportDbc.cpp` / `C_CieExportDbc.hpp`
  - `C_CieImportDbc.cpp` / `C_CieImportDbc.hpp`
- **Usage**: 
  - `std::map<std::string, Vector::DBC::Node>` for node management
  - `std::map<unsigned int, Vector::DBC::Message>` for message handling
  - `std::map<std::string, Vector::DBC::Attribute>` for attributes
  - `std::map<std::string, Vector::DBC::AttributeDefinition>` for attribute definitions

## Why Migrate?
1. **Licensing**: Vector DBC library requires commercial licensing for production use
2. **Qt Integration**: Native Qt CAN support provides better integration with Qt's event system and object model
3. **Portability**: Reduces external dependencies and improves cross-platform compatibility
4. **Maintenance**: Qt's CAN implementation is actively maintained as part of the Qt framework

## Qt Native Alternatives

### Qt CAN Modules (Qt 6.11)
- **Qt CAN Bus** (`QtCanBus`): Provides CAN bus interface abstraction ✅
- **Qt CAN FD**: Support for CAN FD protocol ✅
- **Qt CAN Database**: `QCanDbcFileParser` (Qt 6.5+) ⚠️ **LIMITED**

### Key Qt Classes
- `QCanBus`: Main CAN bus interface
- `QCanBusDevice`: Abstract CAN device interface
- `QCanBusFrame`: CAN frame representation
- **DBC Parsing**: `QCanDbcFileParser` (Qt 6.5+) - **PRELIMINARY/UNSTABLE**

### QCanDbcFileParser Capabilities (Qt 6.11)
**✅ Supported Keywords:**
- `BO_` - message descriptions
- `SG_` - signal descriptions
- `SIG_VALTYPE_` - signal type descriptions
- `SG_MUL_VAL_` - extended multiplexing
- `CM_` - comments (messages/signals only)
- `VAL_` - value descriptions

**❌ NOT Supported:**
- `BA_DEF_`, `BA_`, `BA_DEF_DEF_` - attributes
- `EV_`, `ENVVAR_DATA_` - environment variables
- `SIG_GROUP_` - signal groups
- `BO_TX_BU_` - transmission matrix
- `VAL_TABLE_` - value tables
- **DBC Export/Generation** - NO write capability

**⚠️ Status:** "Preliminary" - Subject to change

### Research Findings (2026-04-03)
- Qt 6.11 HAS `QCanDbcFileParser` but it's **incomplete**
- Only supports 6 of 30+ DBC keywords
- **NO export capability** - critical for openSYDE
- openSYDE uses attributes, environment variables, transmission matrix - **NOT SUPPORTED**
- Building custom DBC parser/exporter required if Vector dependency must be removed

### DBC File Format
DBC files are text-based with a well-defined format:
```
VERSION "1.0"
NS_ : 
BS_:
BO_ 123 MessageName: 8 NodeName
  SG_ SignalName : 0|8@1+ (1,0) [0|0] "" NodeName
```

## Migration Plan

### Phase 1: Research & Design
- [ ] Evaluate Qt 6.5+ CAN DB support capabilities
- [ ] Design Qt-native DBC parser if not available in Qt
- [ ] Define data model mapping from Vector::DBC to Qt structures
- [ ] Create abstraction layer for DBC operations

### Phase 2: Implementation
- [ ] Implement Qt-native DBC parser (or integrate Qt 6.5+ support)
- [ ] Create `C_QtDbcFile` class for DBC file operations
- [ ] Implement message, signal, and node parsing
- [ ] Implement attribute and value table parsing
- [ ] Implement DBC file generation/export

### Phase 3: Integration
- [ ] Update `C_CieImportDbc` to use Qt-native parser
- [ ] Update `C_CieExportDbc` to use Qt-native generator
- [ ] Remove Vector DBC library dependency
- [ ] Update build configuration (CMake)

### Phase 4: Testing
- [ ] Create test suite with known DBC files
- [ ] Verify round-trip (import → export → import) consistency
- [ ] Performance benchmarking
- [ ] Regression testing with existing DBC files

## Technical Notes

### DBC Parser Requirements
- Parse version information
- Parse message definitions (ID, name, length, nodes)
- Parse signal definitions (name, start bit, length, byte order, factor, offset, min/max)
- Parse node definitions
- Parse attribute definitions and values
- Parse value tables
- Parse environment variables
- Handle comments and custom attributes

### Qt Integration Points
- Use `QFile` for file I/O
- Use `QString` for all string data
- Use `QList`/`QHash` for collections
- Use `QVariant` for flexible attribute values
- Consider using `QJsonDocument` for intermediate representation

## References
- [Qt CAN Bus Documentation](https://doc.qt.io/qt-6/qcanbus.html)
- [DBC File Format Specification](https://www.vector.com/en/introduction-can-open-technology/can-open-technology/can-open-protocol/dbc-file-format/)
- [Qt 6.5 Release Notes - CAN Support](https://wiki.qt.io/Qt_6.5_Release)

## Priority
**Medium** - This is a long-term improvement that should be addressed after Phase 2 and 3 are complete. The current Vector DBC implementation works correctly, so this is not urgent but should be planned for future releases.

## Estimated Effort
- Research: 2-3 days
- Implementation: 2-3 weeks
- Testing: 1 week
- **Total**: ~1 month
