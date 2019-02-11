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

#ifndef __INET_PROGRESSMETERFIGURE_H
#define __INET_PROGRESSMETERFIGURE_H

#include "inet/common/INETDefs.h"
#include "inet/common/INETMath.h"
#include "inet/common/figures/IIndicatorFigure.h"

namespace inet {

class INET_API ProgressMeterFigure : public cGroupFigure, public inet::IIndicatorFigure
{
    cRectangleFigure *borderFigure;
    cRectangleFigure *stripFigure;
    cRectangleFigure *backgroundFigure;
    cTextFigure *valueFigure;
    cTextFigure *labelFigure;

    double min = 0;
    double max = 100;
    double value = NaN;
    int labelOffset = 10;
    std::string textFormat = "%g";

  protected:
    virtual void parse(cProperty *property) override;
    virtual const char **getAllowedPropertyKeys() const override;
    void addChildren();
    void refresh();
    void layout();

  public:
    ProgressMeterFigure(const char *name = nullptr);
    virtual ~ProgressMeterFigure() {};

    virtual const Point getSize() const override { return getBounds().getSize(); }
    virtual void setValue(int series, simtime_t timestamp, double value) override;

    // getters and setters
    const Color& getBackgroundColor() const;
    void setBackgroundColor(const Color& color);

    const Color& getStripColor() const;
    void setStripColor(const Color& color);

    double getCornerRadius() const;
    void setCornerRadius(double radius);

    double getBorderWidth() const;
    void setBorderWidth(double width);

    const char *getText() const;
    void setText(const char *text);

    const Font& getTextFont() const;
    void setTextFont(const Font& font);

    const Color& getTextColor() const;
    void setTextColor(const Color& color);

    const char *getLabel() const;
    void setLabel(const char *text);

    int getLabelOffset() const;
    void setLabelOffset(int);

    const Font& getLabelFont() const;
    void setLabelFont(const Font& font);

    const Color& getLabelColor() const;
    void setLabelColor(const Color& color);

    const Rectangle& getBounds() const;
    void setBounds(const Rectangle& bounds);

    double getMinValue() const;
    void setMinValue(double value);

    double getMaxValue() const;
    void setMaxValue(double value);

};

} // namespace inet

#endif

