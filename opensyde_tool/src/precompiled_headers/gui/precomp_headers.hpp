//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Precompiled-header wrapper for the main openSYDE tool

   Pulls in the shared base (precomp_headers_common.hpp from libraries/opensyde_gui) plus the
   project-specific headers heavily used by the tool (stwtypes ~45% of TUs,
   C_PuiSdHandler ~17%) and the Qt umbrella headers — these are all cheap-to-include here and
   substantially reduce per-TU parse time across the tool's ~1300 source files.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef  PRECOMP_HEADERS_GUI_HPP
#define  PRECOMP_HEADERS_GUI_HPP

#ifndef _lint  // speed up linting: don't include all of the headers for each linted .cpp file

#include "precomp_headers_common.hpp"

// Tool-only Qt umbrella modules (DBC keyword conflict already handled globally via QT_NO_KEYWORDS)
#include <QtGui>
#include <QtCore>
#include <QtWidgets>

// Tool-only STW project headers
#include <cstdint>
#include "C_PuiProject.hpp"
#include "C_PuiSdHandler.hpp"
#include "C_SdTopologyScene.hpp"
#include "C_SclChecksums.hpp"

#endif

#endif
