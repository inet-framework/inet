//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GAUGEFIGURE_H
#define __INET_GAUGEFIGURE_H

#include "inet/common/INETMath.h"
#include "inet/common/figures/IIndicatorFigure.h"

namespace inet {

class INET_API GaugeFigure : public cGroupFigure, public inet::IIndicatorFigure
{
    cPathFigure *needle;
    cTextFigure *valueFigure;
    cTextFigure *labelFigure;
    cOvalFigure *backgroundFigure;
    std::vector<cArcFigure *> curveFigures;

    // TODO Create a structure with cLineFigure* and cTextFigure*
    std::vector<cLineFigure *> tickFigures;
    std::vector<cTextFigure *> numberFigures;
    const char *colorStrip = "";
    double min = 0;
    double max = 100;
    double tickSize = 10;
    double value = NaN;
    int numTicks = 0;
    double shifting = 0;
    int curvesOnCanvas = 0;
    int labelOffset = 10;

  protected:
    virtual void parse(cProperty *property) override;
    virtual const char **getAllowedPropertyKeys() const override;
    void addChildren();

    void setColorCurve(const cFigure::Color& curveColor, double startAngle, double endAngle, cArcFigure *arc);
    void setCurveGeometry(cArcFigure *curve);
    void setTickGeometry(cLineFigure *tick, int index);
    void setNumberGeometry(cTextFigure *number, int index);
    void setNeedleGeometry();
    void setNeedleTransform();

    void redrawTicks();
    void redrawCurves();
    void layout();
    void refresh();

  public:
    GaugeFigure(const char *name = nullptr);
    virtual ~GaugeFigure();

    virtual const Point getSize() const override { return getBounds().getSize(); }
    virtual void setValue(int series, simtime_t timestamp, double value) override;

    const Rectangle& getBounds() const;
    void setBounds(const Rectangle& rect);

    const Color& getBackgroundColor() const;
    void setBackgroundColor(const Color& color);

    const Color& getNeedleColor() const;
    void setNeedleColor(const Color& color);

    const char *getLabel() const;
    void setLabel(const char *text);

    int getLabelOffset() const;
    void setLabelOffset(int offset);

    const Font& getLabelFont() const;
    void setLabelFont(const Font& font);

    const Color& getLabelColor() const;
    void setLabelColor(const Color& color);

    double getMinValue() const;
    void setMinValue(double value);

    double getMaxValue() const;
    void setMaxValue(double value);

    double getTickSize() const;
    void setTickSize(double value);

    const char *getColorStrip() const;
    void setColorStrip(const char *colorStrip);
};

} // namespace inet

#endif

