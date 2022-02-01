//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BARFIGURE_H
#define __INET_BARFIGURE_H

#include "inet/common/INETMath.h"

namespace inet {

class INET_API BarFigure : public cRectangleFigure
{
  protected:
    double value = NaN;
    double minValue = NaN;
    double maxValue = NaN;
    double spacing = 2;
    cRectangleFigure *valueFigure = nullptr;

  protected:
    virtual void refreshDisplay();

  public:
    BarFigure(const char *name = nullptr);

    void setColor(const cFigure::Color& color) { valueFigure->setFillColor(color); }

    void setSpacing(double spacing) { this->spacing = spacing; }
    void setMinValue(double minValue) { this->minValue = minValue; }
    void setMaxValue(double maxValue) { this->maxValue = maxValue; }
    void setValue(double value);
};

} // namespace inet

#endif

