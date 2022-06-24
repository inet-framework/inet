//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INSTRUMENTUTIL_H
#define __INET_INSTRUMENTUTIL_H

#include "inet/common/INETDefs.h"

class INET_API InstrumentUtil
{
    typedef int OutCode;

    static const int INSIDE = 0; // 0000
    static const int LEFT   = 1; // 0001
    static const int RIGHT  = 2; // 0010
    static const int BOTTOM = 4; // 0100
    static const int TOP    = 8; // 1000

    static OutCode ComputeOutCode(double x, double y, double xmin, double xmax, double ymin, double ymax);

  public:

    /**
     * Cohen–Sutherland clipping algorithm clips a line from
     * P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with
     * diagonal from (xmin, ymin) to (xmax, ymax).
     *
     * Returns true if the line segment is intersecting with the
     * rectangle, false otherwise.
     */
    static bool CohenSutherlandLineClip(double& x0, double& y0, double& x1, double& y1, double xmin, double xmax, double ymin, double ymax);
};

#endif

