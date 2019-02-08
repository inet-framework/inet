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

#ifndef __INET_LINEARGAUGEFIGURE_H
#define __INET_LINEARGAUGEFIGURE_H

#include "inet/common/INETDefs.h"
#include "inet/common/INETMath.h"
#include "inet/common/figures/IIndicatorFigure.h"

namespace inet {

class INET_API LinearGaugeFigure : public cGroupFigure, public inet::IIndicatorFigure
{
    cLineFigure *needle;
    cTextFigure *labelFigure;
    cRectangleFigure *backgroundFigure;
    cLineFigure *axisFigure;
    std::vector<cLineFigure *> tickFigures;
    std::vector<cTextFigure *> numberFigures;
    double min = 0;
    double max = 100;
    double tickSize = 10;
    double value = NaN;
    int numTicks = 0;
    double shifting = 0;
    int labelOffset = 10;

  protected:
    virtual void parse(cProperty *property) override;
    virtual const char **getAllowedPropertyKeys() const override;
    void addChildren();

    void setTickGeometry(cLineFigure *tick, int index);
    void setNumberGeometry(cTextFigure *number, int index);
    void setNeedleGeometry();

    void redrawTicks();
    void layout();
    void refresh();

  public:
    LinearGaugeFigure(const char *name = nullptr);
    virtual ~LinearGaugeFigure();

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

    double getCornerRadius() const;
    void setCornerRadius(double radius);
};

} // namespace inet

#endif // ifndef __INET_LinearGaugeFigure_H

