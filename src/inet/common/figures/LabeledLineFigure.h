//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LABELEDLINEFIGURE_H
#define __INET_LABELEDLINEFIGURE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API LabeledLineFigure : public cGroupFigure
{
  protected:
    cLineFigure *lineFigure;
    cPanelFigure *panelFigure;
    cTextFigure *labelFigure;

  protected:
    void updateLabelPosition();

  public:
    LabeledLineFigure(const char *name = nullptr);

    cLineFigure *getLineFigure() const { return lineFigure; }
    cTextFigure *getLabelFigure() const { return labelFigure; }

    void setStart(cFigure::Point point);
    void setEnd(cFigure::Point point);
};

} // namespace inet

#endif

