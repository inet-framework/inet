//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#endif

