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

#ifndef __INET_HEATMAPPLOTFIGURE_H
#define __INET_HEATMAPPLOTFIGURE_H

#include "inet/common/INETMath.h"

namespace inet {

class INET_API HeatMapPlotFigure : public cGroupFigure
{
    struct Tick
    {
        cLineFigure *tick;
        cLineFigure *dashLine;
        cLabelFigure *number;

        Tick(cLineFigure *tick, cLineFigure *dashLine, cLabelFigure *number) :
            tick(tick), dashLine(dashLine), number(number) {}
    };

    cPixmapFigure *pixmapFigure;
    cRectangleFigure *backgroundFigure;
    cLabelFigure *labelFigure;
    cLabelFigure *xAxisLabelFigure;
    cLabelFigure *yAxisLabelFigure;
    std::vector<Tick> xTicks;
    std::vector<Tick> yTicks;

    Rectangle bounds;
    double yTickSize = INFINITY;
    double xTickSize = INFINITY;
    bool invertedXAxis = false;
    bool invertedYAxis = false;
    int labelOffset = 0;
    double minX = 0;
    double maxX = 1;
    double minY = 0;
    double maxY = 1;
    double minValue = 0;
    double maxValue = 1;
    const char *xValueFormat = "%g";
    const char *yValueFormat = "%g";
    bool invalidLayout = true;

  protected:
    Color getHeatColor(int x, int y, double c, int channel);
    void redrawYTicks();
    void redrawXTicks();
    void addChildren();
    void layout();
    void setPixelSafe(int x, int y, double c, int channel);

  public:
    HeatMapPlotFigure(const char *name = nullptr);
    virtual ~HeatMapPlotFigure() {};

    virtual void parse(cProperty *property) override;
    const char **getAllowedPropertyKeys() const override;

    virtual void refreshDisplay() override;

    void clearValues();
    void setValue(int x, int y, double v, int channel);
    void setValue(double x, double y, double v, int channel);
    void setConstantValue(double x1, double x2, double y1, double y2, double v, int channel);
    void setLinearValue(double x1, double x2, double y1, double y2, double vl, double vu, int axis, int channel);
    void setBilinearValue(double x1, double x2, double y1, double y2, double v11, double v21, double v12, double v22, int channel);
    void bakeValues();

    //getters and setters
    void setPlotSize(const Point& figureSize, const Point& pixmapSize);
    const Point getPlotSize() const { return pixmapFigure->getBounds().getSize(); }
    const Point getPixmapSize() const { return Point(pixmapFigure->getPixmapWidth(), pixmapFigure->getPixmapHeight()); }

    const Point getSize() const { return getBounds().getSize(); }
    const Rectangle& getBounds() const;
    void setBounds(const Rectangle& rect);

    double getXTickSize() const;
    void setXTickSize(double size);
    void setXTickCount(int count);

    double getYTickSize() const;
    void setYTickSize(double size);
    void setYTickCount(int count);

    double getMinX() const { return minX; }
    void setMinX(double value);

    double getMaxX() const { return maxX; }
    void setMaxX(double value);

    double getMinY() const { return minY; }
    void setMinY(double value);

    double getMaxY() const { return maxY; }
    void setMaxY(double value);

    double getMinValue() const { return minValue; }
    void setMinValue(double value);

    double getMaxValue() const { return maxValue; }
    void setMaxValue(double value);

    void setXValueFormat(const char *format) { xValueFormat = format; }
    void setYValueFormat(const char *format) { yValueFormat = format; }

    void invertXAxis() { invertedXAxis = !invertedXAxis; }
    void invertYAxis() { invertedYAxis = !invertedYAxis; }

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

#endif // ifndef __INET_HEATMAPPLOTFIGURE_H

