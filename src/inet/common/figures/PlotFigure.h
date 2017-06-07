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
#include "IIndicatorFigure.h"

// for the moment commented out as omnet cannot instatiate it from a namespace
//namespace inet {

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

    cPathFigure *plotFigure;
    cTextFigure *labelFigure;
    cRectangleFigure *backgroundFigure;
    std::vector<Tick> timeTicks;
    std::vector<Tick> valueTicks;

    simtime_t timeWindow = 10;
    double valueTickSize = 2.5;
    simtime_t timeTickSize = 3;
    int labelOffset = 0;
    double numberSizeFactor = 1;
    double min = 0;
    double max = 10;
    std::list<std::pair<simtime_t, double>> values;

  protected:
    void redrawValueTicks();
    void redrawTimeTicks();
    void addChildren();
    void layout();
    void refresh();

  public:
    PlotFigure(const char *name = nullptr);
    virtual ~PlotFigure() {};

    virtual void parse(cProperty *property) override;
    const char **getAllowedPropertyKeys() const override;

    virtual void refreshDisplay() override;

    virtual void setValue(int series, simtime_t timestamp, double value) override;

    //getters and setters
    const Rectangle& getBounds() const;
    void setBounds(const Rectangle& rect);

    const Color& getBackgrouncColor() const;
    void setBackgroundColor(const Color& color);

    double getValueTickSize() const;
    void setValueTickSize(double size);

    simtime_t getTimeWindow() const;
    void setTimeWindow(simtime_t timeWindow);

    simtime_t getTimeTickSize() const;
    void setTimeTickSize(simtime_t size);

    const Color& getLineColor() const;
    void setLineColor(const Color& color);

    double getMinValue() const;
    void setMinValue(double value);

    double getMaxValue() const;
    void setMaxValue(double value);

    const char *getLabel() const;
    void setLabel(const char *text);

    const int getLabelOffset() const;
    void setLabelOffset(int offset);

    const Font& getLabelFont() const;
    void setLabelFont(const Font& font);

    const Color& getLabelColor() const;
    void setLabelColor(const Color& color);
};

// } // namespace inet

#endif // ifndef __INET_PLOTFIGURE_H

