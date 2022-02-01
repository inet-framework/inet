//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LABELEDPOLYLINEFIGURE_H
#define __INET_LABELEDPOLYLINEFIGURE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API LabeledPolylineFigure : public cGroupFigure
{
  protected:
    cPolylineFigure *polylineFigure;
    cPanelFigure *panelFigure;
    cTextFigure *labelFigure;

  protected:
    void updateLabelPosition();

  public:
    LabeledPolylineFigure(const char *name = nullptr);

    cPolylineFigure *getPolylineFigure() const { return polylineFigure; }
    cTextFigure *getLabelFigure() const { return labelFigure; }

    void setPoints(const std::vector<cFigure::Point>& points);
};

} // namespace inet

#endif

