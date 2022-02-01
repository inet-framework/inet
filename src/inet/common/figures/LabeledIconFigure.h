//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LABELEDICONFIGURE_H
#define __INET_LABELEDICONFIGURE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API LabeledIconFigure : public cGroupFigure
{
  protected:
    cIconFigure *iconFigure;
    cLabelFigure *labelFigure;

  public:
    LabeledIconFigure(const char *name = nullptr);

    cIconFigure *getIconFigure() const { return iconFigure; }
    cLabelFigure *getLabelFigure() const { return labelFigure; }

    void setTooltip(const char *tooltip);
    void setAssociatedObject(cObject *object);

    cFigure::Rectangle getBounds() const;

    void setOpacity(double opacity);

    void setPosition(cFigure::Point position);
};

} // namespace inet

#endif

