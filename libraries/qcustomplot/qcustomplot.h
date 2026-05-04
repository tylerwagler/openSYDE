//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       QCustomPlot compatibility wrapper

   QCustomPlot uses bare Qt keywords (signals, slots, foreach) which are disabled
   in this project via QT_NO_KEYWORDS (required for DBC library compatibility).
   This wrapper temporarily defines the needed keywords around the system header.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef QCUSTOMPLOT_WRAPPER_H
#define QCUSTOMPLOT_WRAPPER_H

// Temporarily restore Qt keywords that qcustomplot.h requires
#ifndef signals
#define signals Q_SIGNALS
#define QCUSTOMPLOT_WRAPPER_UNDEF_SIGNALS
#endif

#ifndef slots
#define slots Q_SLOTS
#define QCUSTOMPLOT_WRAPPER_UNDEF_SLOTS
#endif

#ifndef foreach
#define foreach Q_FOREACH
#define QCUSTOMPLOT_WRAPPER_UNDEF_FOREACH
#endif

// Include the real system header
#include_next <qcustomplot.h>

// Clean up: restore QT_NO_KEYWORDS state
#ifdef QCUSTOMPLOT_WRAPPER_UNDEF_SIGNALS
#undef signals
#undef QCUSTOMPLOT_WRAPPER_UNDEF_SIGNALS
#endif

#ifdef QCUSTOMPLOT_WRAPPER_UNDEF_SLOTS
#undef slots
#undef QCUSTOMPLOT_WRAPPER_UNDEF_SLOTS
#endif

#ifdef QCUSTOMPLOT_WRAPPER_UNDEF_FOREACH
#undef foreach
#undef QCUSTOMPLOT_WRAPPER_UNDEF_FOREACH
#endif

#endif
