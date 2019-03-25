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

#ifndef __INET_COUNTERFIGURE_H
#define __INET_COUNTERFIGURE_H

#include "inet/common/INETDefs.h"
#include "inet/common/INETMath.h"
#include "inet/common/figures/IIndicatorFigure.h"

namespace inet {

class INET_API CounterFigure : public cGroupFigure, public inet::IIndicatorFigure
{
    struct Digit
    {
        cRectangleFigure *bounds;
        cTextFigure *text;

        Digit(cRectangleFigure *bounds, cTextFigure *text) : bounds(bounds), text(text) {}
    };

    cRectangleFigure *backgroundFigure;
    std::vector<Digit> digits;
    cTextFigure *labelFigure;

    double value = NaN;
    int labelOffset = 10;
    Anchor anchor = ANCHOR_NW;

  protected:
    virtual void parse(cProperty *property) override;
    virtual const char **getAllowedPropertyKeys() const override;
    Point calculateRealPos(const Point& pos);
    void calculateBounds();
    void addChildren();
    void refresh();
    void layout();

  public:
    CounterFigure(const char *name = nullptr);
    virtual ~CounterFigure() {};

    virtual void setValue(int series, simtime_t timestamp, double value) override;

    virtual const Point getSize() const override { return backgroundFigure->getBounds().getSize(); }

    // getters and setters
    const Color& getBackgroundColor() const;
    void setBackgroundColor(const Color& color);

    int getDecimalPlaces() const;
    void setDecimalPlaces(int radius);

    Color getDigitBackgroundColor() const;
    void setDigitBackgroundColor(const Color& color);

    Color getDigitBorderColor() const;
    void setDigitBorderColor(const Color& color);

    Font getDigitFont() const;
    void setDigitFont(const Font& font);

    Color getDigitColor() const;
    void setDigitColor(const Color& color);

    const char *getLabel() const;
    void setLabel(const char *text);

    int getLabelOffset() const;
    void setLabelOffset(int offset);

    const Font& getLabelFont() const;
    void setLabelFont(const Font& font);

    const Color& getLabelColor() const;
    void setLabelColor(const Color& color);

    const Point& getLabelPos() const;
    void setLabelPos(const Point& pos);

    Anchor getLabelAnchor() const;
    void setLabelAnchor(Anchor anchor);

    Point getPos() const;
    void setPos(const Point& bounds);

    Anchor getAnchor() const;
    void setAnchor(Anchor anchor);

};

} // namespace inet

#endif

