//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BOXEDLABELFIGURE_H
#define __INET_BOXEDLABELFIGURE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API BoxedLabelFigure : public cGroupFigure
{
  protected:
    double inset = 3;
    cLabelFigure *labelFigure;
    cRectangleFigure *rectangleFigure;

  public:
    BoxedLabelFigure(const char *name = nullptr);

    cLabelFigure *getLabelFigure() const { return labelFigure; }
    cRectangleFigure *getRectangleFigure() const { return rectangleFigure; }

    double getInset() const { return inset; }
    void setInset(double inset);

    void setTags(const char *tags);
    void setTooltip(const char *tooltip);
    void setAssociatedObject(cObject *object);

    const cFigure::Rectangle& getBounds() const;

    const cFigure::Font& getFont() const;
    void setFont(cFigure::Font font);

    const cFigure::Color& getLabelColor() const;
    void setLabelColor(cFigure::Color color);

    const cFigure::Color& getBackgroundColor() const;
    void setBackgroundColor(cFigure::Color color);

    const cFigure::Color& getLineColor() const;
    void setLineColor(cFigure::Color color);

    const char *getText() const;
    void setText(const char *text);

    cFigure::Alignment getAlignment() const;
    void setAlignment(cFigure::Alignment alignment);

    double getOpacity() const;
    void setOpacity(double opacity);

    bool isFilled() const;
    void setFilled(bool filled);

    bool isOutlined() const;
    void setOutlined(bool outlined);
};

} // namespace inet

#endif

