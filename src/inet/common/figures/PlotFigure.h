//
// Copyright (C) 2016 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_PLOTFIGURE_H
#define __INET_PLOTFIGURE_H

#include "inet/common/INETDefs.h"
#include "inet/common/INETMath.h"
#include "inet/common/figures/IIndicatorFigure.h"

namespace inet {

class INET_API PlotFigure : public cGroupFigure, public inet::IIndicatorFigure
{
    struct Tick
    {
        cLineFigure *tick;
        cLineFigure *dashLine;
        cTextFigure *number;

        Tick(cLineFigure *tick, cLineFigure *dashLine, cTextFigure *number) :
            tick(tick), dashLine(dashLine), number(number) {}
    };

    std::vector<cPathFigure *> seriesPlotFigures;
    cTextFigure *labelFigure;
    cTextFigure *xAxisLabelFigure;
    cTextFigure *yAxisLabelFigure;
    cRectangleFigure *backgroundFigure;
    std::vector<Tick> xTicks;
    std::vector<Tick> yTicks;

    Rectangle bounds;
    int numSeries = -1;
    double timeWindow = NaN;
    double yTickSize = 2.5;
    double xTickSize = 3;
    int labelOffset = 0;
    double numberSizeFactor = 1;
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
    virtual ~PlotFigure() {};

    virtual void parse(cProperty *property) override;
    const char **getAllowedPropertyKeys() const override;

    virtual void refreshDisplay() override;

    virtual void setNumSeries(int numSeries);
    virtual int getNumSeries() const override { return numSeries; }

    virtual const Point getSize() const override { return getBounds().getSize(); }
    virtual void setValue(int series, simtime_t timestamp, double value) override { setValue(series, timestamp.dbl(), value); }
    virtual void setValue(int series, double x, double y);
    virtual void clearValues(int series) { seriesValues[series].clear(); invalidPlot = true; }

    //getters and setters
    void setPlotSize(const Point& p);

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

    const char* getXAxisLabel() const { return xAxisLabelFigure->getText(); }
    void setXAxisLabel(const char* text) { xAxisLabelFigure->setText(text); }

    const char* getYAxisLabel() const { return yAxisLabelFigure->getText(); }
    void setYAxisLabel(const char* text) { yAxisLabelFigure->setText(text); }

    const char* getLabel() const { return labelFigure->getText(); }
    void setLabel(const char* text) { labelFigure->setText(text); }

    int getLabelOffset() const;
    void setLabelOffset(int offset);

    const Font& getLabelFont() const;
    void setLabelFont(const Font& font);

    const Color& getLabelColor() const;
    void setLabelColor(const Color& color);
};

} // namespace inet

#endif // ifndef __INET_PLOTFIGURE_H

