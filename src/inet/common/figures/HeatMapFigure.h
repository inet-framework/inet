//
// Copyright (C) 2016 OpenSim Ltd.
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

#ifndef __INET_HEATMAPFIGURE_H
#define __INET_HEATMAPFIGURE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API HeatMapFigure : public cPixmapFigure
{
  protected:
    double alpha = 0.5;
    double coolingSpeed = 0.01;
    cFigure::Color fromColor = cFigure::BLACK;
    cFigure::Color toColor = cFigure::RED;
    simtime_t lastCoolDown;

  protected:
    double getHeat(int x, int y);
    void setHeat(int x, int y, double value);

  public:
    HeatMapFigure(int size, const char *name);

    virtual const char *getClassNameForRenderer() const { return "cPixmapFigure"; }

    virtual void heatPoint(int x, int y);
    virtual void heatLine(int x1, int y1, int x2, int y2);
    virtual void coolDown();
};

} // namespace inet

#endif // ifndef __INET_HEATMAPFIGURE_H

