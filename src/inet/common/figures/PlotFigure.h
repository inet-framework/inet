//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PLOTFIGURE_H
#define __INET_PLOTFIGURE_H

#include "inet/common/INETMath.h"
#include "inet/common/figures/IIndicatorFigure.h"

namespace inet {

// TODO extend with margin parameters
class INET_API PlotFigure : public cGroupFigure, public inet::IIndicatorFigure
{
    struct Tick {
        cLineFigure *tick;
        cLineFigure *dashLine;
        cLabelFigure *number;

        Tick(cLineFigure *tick, cLineFigure *dashLine, cLabelFigure *number) :
            tick(tick), dashLine(dashLine), number(number) {}
    };

    std::vector<cPathFigure *> seriesPlotFigures;
    cLabelFigure *labelFigure;
    cLabelFigure *xAxisLabelFigure;
    cLabelFigure *yAxisLabelFigure;
    cRectangleFigure *backgroundFigure;
    std::vector<Tick> xTicks;
    std::vector<Tick> yTicks;

    Rectangle bounds;
    int numSeries = -1;
    double timeWindow = NaN;
    double yTickSize = INFINITY;
    double xTickSize = INFINITY;
    int labelOffset = 0;
    double minX = 0;
    double maxX = 1;
    double minY = 0;
    double maxY = 1;
    const char *xValueFormat = "%g";
    const char *yValueFormat = "%g";
    bool invalidLayout = true;
    bool invalidPlot = true;

    std::vector<std::list<std::pair<double, double>>> seriesValues;

  protected:
    void redrawYTicks();
    void redrawXTicks();
    void addChildren();
    void layout();
    void plot();

  public:
    PlotFigure(const char *name = nullptr);
    virtual ~PlotFigure() {}

    virtual void parse(cProperty *property) override;
    const char **getAllowedPropertyKeys() const override;

    virtual void refreshDisplay() override;

    virtual void setNumSeries(int numSeries);
    virtual int getNumSeries() const override { return numSeries; }

    virtual void setValue(int series, simtime_t timestamp, double value) override { setValue(series, timestamp.dbl(), value); }
    virtual void setValue(int series, double x, double y);
    virtual void clearValues(int series) { seriesValues[series].clear(); invalidPlot = true; }

    // getters and setters
    const Point getPlotSize() const { return backgroundFigure->getBounds().getSize(); }
    void setPlotSize(const Point& p);

    virtual const Point getSize() const override { return getBounds().getSize(); }
    const Rectangle& getBounds() const;
    void setBounds(const Rectangle& rect);

    const Color& getBackgrouncColor() const;
    void setBackgroundColor(const Color& color);

    double getXTickSize() const;
    void setXTickSize(double size);
    void setXTickCount(int count);

    double getYTickSize() const;
    void setYTickSize(double size);
    void setYTickCount(int count);

    double getTimeWindow() const;
    void setTimeWindow(double timeWindow);

    const Color& getLineColor(int series) const;
    void setLineColor(int series, const Color& color);

    double getMinX() const { return minX; }
    void setMinX(double value);

    double getMaxX() const { return maxX; }
    void setMaxX(double value);

    double getMinY() const { return minY; }
    void setMinY(double value);

    double getMaxY() const { return maxY; }
    void setMaxY(double value);

    void setXValueFormat(const char *format) { xValueFormat = format; }
    void setYValueFormat(const char *format) { yValueFormat = format; }

    const char *getXAxisLabel() const { return xAxisLabelFigure->getText(); }
    void setXAxisLabel(const char *text) { xAxisLabelFigure->setText(text); }

    const char *getYAxisLabel() const { return yAxisLabelFigure->getText(); }
    void setYAxisLabel(const char *text) { yAxisLabelFigure->setText(text); }

    const char *getLabel() const { return labelFigure->getText(); }
    void setLabel(const char *text) { labelFigure->setText(text); }

    int getLabelOffset() const;
    void setLabelOffset(int offset);

    const Font& getLabelFont() const;
    void setLabelFont(const Font& font);

    const Color& getLabelColor() const;
    void setLabelColor(const Color& color);
};

} // namespace inet

#endif

