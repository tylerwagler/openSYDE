# QCustomPlot (vendored)

Upstream source of QCustomPlot, vendored so the openSYDE chart-tab dashboard
feature builds identically on Linux, macOS and Windows without a system package
(Linux), a committed binary (Windows), or an unavailable one (macOS).

- **Version:** 2.1.1 (2022-11-06)
- **Source:** https://www.qcustomplot.com/release/2.1.1/QCustomPlot-source.tar.gz
- **SHA-256:** 5e2d22dec779db8f01f357cbdb25e54fbcf971adaee75eae8d7ad2444487182f
- **License:** GPL v3 or later (see GPL.txt) — openSYDE is GPL v3, so this is compatible.

Only `qcustomplot.cpp` / `qcustomplot.h` are compiled. They are built as the
`qcustomplot` static library (see opensyde_tool/pjt/openSYDE/CMakeLists.txt), with
Qt keywords enabled (`-UQT_NO_KEYWORDS`) because QCustomPlot uses bare
signals/slots/foreach, and with warnings off — it is third-party code held to its
own standard, like miniz.

The sibling `../qcustomplot.h` is a thin wrapper that re-enables the Qt keywords
around this header for *consumers*, which build under QT_NO_KEYWORDS. Do not edit
these files; re-vendor from upstream to update.
