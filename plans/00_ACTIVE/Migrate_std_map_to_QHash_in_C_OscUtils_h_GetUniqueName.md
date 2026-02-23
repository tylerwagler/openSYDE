## Task: Migrate std::map<QString, bool> to QHash in C_OscUtils::h_GetUniqueName

### Overview
Migrate usage of `std::map<QString, bool>` to `QHash<QString, bool>` in the function `C_OscUtils::h_GetUniqueName` to align with Qt-native coding standards.

### Code Changes

1. Update the function signature in `C_OscUtils.hpp`:

```cpp
// Before
static QString
h_GetUniqueName(const std::map<QString, bool> &orc_ExistingStrings,
                const QString &orc_ProposedName,
                const uint32_t ou32_MaxCharLimit,
                const QString &orc_SkipName = "");

// After
static QString
h_GetUniqueName(const QHash<QString, bool> &orc_ExistingStrings,
                const QString &orc_ProposedName,
                const uint32_t ou32_MaxCharLimit,
                const QString &orc_SkipName = "");
```

2. Update the function implementation in `C_OscUtils.cpp`:

```cpp
// Before
QString
C_OscUtils::h_GetUniqueName(const std::map<QString, bool> &orc_ExistingStrings,
                            const QString &orc_ProposedName,
                            const uint32_t ou32_MaxCharLimit,
                            const QString &orc_SkipName) {
  QString c_Retval = orc_ProposedName;
  bool q_Conflict;
  int32_t s32_MaxDeviation;
  QString c_BaseStr;

  std::map<QString, bool>::const_iterator c_ItString;

  // Apply restriction
  if ((ou32_MaxCharLimit > 0UL) && (c_Retval.length() > ou32_MaxCharLimit)) {
    c_Retval = c_Retval.mid(0, ou32_MaxCharLimit);
  }

  do {
    q_Conflict = false;
    c_ItString = orc_ExistingStrings.find(c_Retval);
    if (c_ItString != orc_ExistingStrings.end()) {
      q_Conflict = true;
      mh_GetBaseNameAndCurrentConflictNumberFromString(
          c_ItString->first, orc_SkipName, c_BaseStr, s32_MaxDeviation);
      // Do not use 0 and 1 for name adaptation
      if (s32_MaxDeviation <= 0) {
        s32_MaxDeviation = 1;
      }
      {
        const QString c_Appendix =
            '_' + QString::number(s32_MaxDeviation + static_cast<int32_t>(1));
        c_Retval = c_BaseStr + c_Appendix;
        if ((ou32_MaxCharLimit > 0UL) &&
            (c_Retval.length() > ou32_MaxCharLimit)) {
          const uint32_t u32_ReqLength = c_Appendix.length();
          if (u32_ReqLength < ou32_MaxCharLimit) {
            c_Retval =
                c_BaseStr.mid(0, ou32_MaxCharLimit - c_Appendix.length()) +
                c_Appendix;
          }
        }
      }
    }
  } while (q_Conflict == true);
  return c_Retval;
}


// After
QString
C_OscUtils::h_GetUniqueName(const QHash<QString, bool> &orc_ExistingStrings,
                            const QString &orc_ProposedName,
                            const uint32_t ou32_MaxCharLimit,
                            const QString &orc_SkipName) {
  QString c_Retval = orc_ProposedName;
  bool q_Conflict;
  int32_t s32_MaxDeviation;
  QString c_BaseStr;

  QHash<QString, bool>::const_iterator c_ItString;

  // Apply restriction
  if ((ou32_MaxCharLimit > 0UL) && (c_Retval.length() > ou32_MaxCharLimit)) {
    c_Retval = c_Retval.mid(0, ou32_MaxCharLimit);
  }

  do {
    q_Conflict = false;
    c_ItString = orc_ExistingStrings.find(c_Retval);
    if (c_ItString != orc_ExistingStrings.end()) {
      q_Conflict = true;
      mh_GetBaseNameAndCurrentConflictNumberFromString(
          c_ItString.key(), orc_SkipName, c_BaseStr, s32_MaxDeviation);
      // Do not use 0 and 1 for name adaptation
      if (s32_MaxDeviation <= 0) {
        s32_MaxDeviation = 1;
      }
      {
        const QString c_Appendix =
            '_' + QString::number(s32_MaxDeviation + static_cast<int32_t>(1));
        c_Retval = c_BaseStr + c_Appendix;
        if ((ou32_MaxCharLimit > 0UL) &&
            (c_Retval.length() > ou32_MaxCharLimit)) {
          const uint32_t u32_ReqLength = c_Appendix.length();
          if (u32_ReqLength < ou32_MaxCharLimit) {
            c_Retval =
                c_BaseStr.mid(0, ou32_MaxCharLimit - c_Appendix.length()) +
                c_Appendix;
          }
        }
      }
    }
  } while (q_Conflict == true);
  return c_Retval;
}
```

3. No other changes are needed — the function logic remains unchanged.

### Verification Steps

1. Run `clang-format -i --style=file /home/tyler/Projects/openSYDE/opensyde_tool/libs/opensyde_core/C_OscUtils.{hpp,cpp}`
2. Build the project with `.\build.ps1 -Component Core`.
3. Confirm compilation succeeds with no errors.

### Testing Instructions

1. Run unit tests:
   ```bash
   cd /home/tyler/Projects/openSYDE/opensyde_tool/test/core
   cmake --build . --target test_core
   ```
2. Verify all tests pass, especially those involving `C_OscUtils::h_GetUniqueName`.
3. In GUI: Create a new node with a duplicate name — ensure unique name generation works.

### Notes
- The logic is unchanged — only container type was updated.
- `QHash::find()` returns an iterator, and `.key()` extracts the key — identical semantics to `std::map::find()->first`.
- `QHash` provides O(1) average lookup, matching performance of `std::map` in this case.
- **No external API impact** — the function signature change is internal.
- **Reference**: `plans/02_FUTURE/Qt_Native_Coding_Standards.md` — Section 2.2: Containers
- **Commit message suggestion**: `[opensyde_core] Migrate std::map<QString, bool> to QHash in C_OscUtils::h_GetUniqueName`

---

> ✅ The migration is complete and ready for implementation.
