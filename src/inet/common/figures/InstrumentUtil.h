//
// Copyright (C) 2016 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_INSTRUMENTUTIL_H
#define __INET_INSTRUMENTUTIL_H

class InstrumentUtil
{
    typedef int OutCode;

    static const int INSIDE = 0; // 0000
    static const int LEFT = 1;   // 0001
    static const int RIGHT = 2;  // 0010
    static const int BOTTOM = 4; // 0100
    static const int TOP = 8;    // 1000

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

#endif // ifndef __INET_INSTRUMENTUTIL_H
