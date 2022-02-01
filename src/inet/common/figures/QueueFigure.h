//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QUEUEFIGURE_H
#define __INET_QUEUEFIGURE_H

#include "inet/common/INETMath.h"

namespace inet {

class INET_API QueueFigure : public cRectangleFigure
{
  protected:
    cFigure::Color color;
    bool continuous = false;
    double spacing = 2;
    double elementWidth = 16;
    double elementHeight = 4;
    int elementCount = -1;
    int maxElementCount = -1;
    std::vector<cRectangleFigure *> boxes;

  public:
    QueueFigure(const char *name = nullptr);

    cFigure::Color getColor() const { return color; }
    void setColor(cFigure::Color color) { this->color = color; }

    double getSpacing() const { return spacing; }
    void setSpacing(double spacing) { this->spacing = spacing; }

    void setElementCount(int elementCount);
    void setMaxElementCount(int maxElementCount);
};

} // namespace inet

#endif

