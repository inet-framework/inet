//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GATEFIGURE_H
#define __INET_GATEFIGURE_H

#include "inet/common/INETMath.h"

namespace inet {

class INET_API GateFigure : public cRectangleFigure
{
  protected:
    double spacing = 2;
    double position = 0;
    cLabelFigure *labelFigure = nullptr;
    cLineFigure *positionFigure = nullptr;
    std::vector<cRectangleFigure *> scheduleFigures;

  public:
    GateFigure(const char *name = nullptr);

    virtual void setBounds(const Rectangle& bounds) override;

    const char *getLabel() const { return labelFigure->getText(); }
    void setLabel(const char *text);

    double getSpacing() const { return spacing; }
    void setSpacing(double spacing) { this->spacing = spacing; }

    double getPosition() const { return position; }
    void setPosition(double position);

    void addSchedule(double start, double end, bool open);
    void clearSchedule();
};

} // namespace inet

#endif

