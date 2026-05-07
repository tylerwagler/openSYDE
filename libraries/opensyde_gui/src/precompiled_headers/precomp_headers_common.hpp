//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Shared precompiled-header base for all Qt GUI targets

   Pre-includes the C++ stdlib + Qt headers that every GUI target uses. Each target has its own
   small precomp_headers.hpp wrapper that #includes this file via the include path; the openSYDE
   tool's wrapper additionally pre-includes its own STW project headers (stwtypes, C_PuiSdHandler,
   etc.). Targets wire up their wrapper via target_precompile_headers(<target> PRIVATE ...).

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef  PRECOMP_HEADERS_COMMON_HPP
#define  PRECOMP_HEADERS_COMMON_HPP

//lint -esym(766,"precomp_headers_common.hpp")   effectively not used in lint "builds"; but that's exactly what we want
#ifndef _lint //speed up linting: don't include all of the headers for each linted .cpp file

/* -- Includes ------------------------------------------------------------------------------------------------------ */

/* Add C includes here */

#if defined __cplusplus

/* Add C++ includes here */
#include <vector>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

// Qt includes
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QCursor>
#include <QDialog>
#include <QEvent>
#include <QFile>
#include <QFont>
#include <QFrame>
#include <QGraphicsItem>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QPoint>
#include <QPointF>
#include <QPushButton>
#include <QSize>
#include <QString>
#include <QTimer>
#include <QUndoCommand>
#include <QUndoStack>
#include <QVariant>
#include <QVector>
#include <QWidget>

#endif

#endif

#endif
