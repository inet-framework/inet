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
//

#include <cstdlib>
#include "ProgressMeterFigure.h"

// for the moment commented out as omnet cannot instatiate it from a namespace
using namespace inet;
// namespace inet {

Register_Class(ProgressMeterFigure);

#if OMNETPP_VERSION >= 0x500

#define M_PI 3.14159265358979323846
static const char *BACKGROUND_COLOR_PROPERTY = "backgroundColor";
static const char *STRIP_COLOR_PROPERTY = "stripColor";
static const char *CORNER_RADIUS_PROPERTY = "cornerRadius";
static const char *BORDER_WIDTH_PROPERTY = "borderWidth";
static const char *MIN_PROPERTY = "min";
static const char *MAX_PROPERTY = "max";
static const char *TEXT_PROPERTY = "text";
static const char *TEXT_FONT_PROPERTY = "textFont";
static const char *TEXT_COLOR_PROPERTY = "textColor";
static const char *LABEL_PROPERTY = "label";
static const char *LABEL_FONT_PROPERTY = "labelFont";
static const char *LABEL_COLOR_PROPERTY = "labelColor";
static const char *POS_PROPERTY = "pos";
static const char *SIZE_PROPERTY = "size";
static const char *ANCHOR_PROPERTY = "anchor";
static const char *BOUNDS_PROPERTY = "bounds";

ProgressMeterFigure::ProgressMeterFigure(const char *name) : cGroupFigure(name)
{
    addChildren();
}

cFigure::Color ProgressMeterFigure::getBackgroundColor() const
{
    return backgroundFigure->getFillColor();
}

void ProgressMeterFigure::setBackgroundColor(cFigure::Color color)
{
    backgroundFigure->setFillColor(color);
}

cFigure::Color ProgressMeterFigure::getStripColor() const
{
    return stripFigure->getFillColor();
}

void ProgressMeterFigure::setStripColor(cFigure::Color color)
{
    stripFigure->setFillColor(color);
}

double ProgressMeterFigure::getCornerRadius() const
{
    return backgroundFigure->getCornerRx();
}

void ProgressMeterFigure::setCornerRadius(double radius)
{
    backgroundFigure->setCornerRadius(radius);
    stripFigure->setCornerRadius(radius);
    borderFigure->setCornerRadius(radius);
}

double ProgressMeterFigure::getBorderWidth() const
{
    return borderFigure->getLineWidth();
}

void ProgressMeterFigure::setBorderWidth(double width)
{
    borderFigure->setLineWidth(width);
}

const char *ProgressMeterFigure::getText() const
{
    return textFormat.c_str();
}

void ProgressMeterFigure::setText(const char *text)
{
    textFormat = text;
    refresh();
}

cFigure::Font ProgressMeterFigure::getTextFont() const
{
    return valueFigure->getFont();
}

void ProgressMeterFigure::setTextFont(cFigure::Font font)
{
    valueFigure->setFont(font);
}

cFigure::Color ProgressMeterFigure::getTextColor() const
{
    return valueFigure->getColor();
}

void ProgressMeterFigure::setTextColor(cFigure::Color color)
{
    valueFigure->setColor(color);
}

const char *ProgressMeterFigure::getLabel() const
{
    return labelFigure->getText();
}

void ProgressMeterFigure::setLabel(const char *text)
{
    labelFigure->setText(text);
}

cFigure::Font ProgressMeterFigure::getLabelFont() const
{
    return labelFigure->getFont();
}

void ProgressMeterFigure::setLabelFont(cFigure::Font font)
{
    labelFigure->setFont(font);
}

cFigure::Color ProgressMeterFigure::getLabelColor() const
{
    return labelFigure->getColor();
}

void ProgressMeterFigure::setLabelColor(cFigure::Color color)
{
    labelFigure->setColor(color);
}

cFigure::Rectangle ProgressMeterFigure::getBounds() const
{
    return borderFigure->getBounds();
}

void ProgressMeterFigure::setBounds(Rectangle bounds)
{
    borderFigure->setBounds(bounds);
    layout();
}

double ProgressMeterFigure::getMin() const
{
    return min;
}

void ProgressMeterFigure::setMin(double value)
{
    min = value;
    refresh();
}

double ProgressMeterFigure::getMax() const
{
    return max;
}

void ProgressMeterFigure::setMax(double value)
{
    max = value;
    refresh();
}

void ProgressMeterFigure::parse(cProperty *property)
{
    cGroupFigure::parse(property);

    setBounds(parseBounds(property));

    const char *s;
    if ((s = property->getValue(BACKGROUND_COLOR_PROPERTY)) != nullptr)
        setBackgroundColor(parseColor(s));
    if ((s = property->getValue(STRIP_COLOR_PROPERTY)) != nullptr)
        setStripColor(parseColor(s));
    if ((s = property->getValue(CORNER_RADIUS_PROPERTY)) != nullptr)
        setCornerRadius(atof(s));
    if ((s = property->getValue(BORDER_WIDTH_PROPERTY)) != nullptr)
        setBorderWidth(atof(s));
    if ((s = property->getValue(MIN_PROPERTY)) != nullptr)
        setMin(atof(s));
    if ((s = property->getValue(MAX_PROPERTY)) != nullptr)
        setMax(atof(s));
    if ((s = property->getValue(TEXT_PROPERTY)) != nullptr)
        setText(s);
    if ((s = property->getValue(TEXT_FONT_PROPERTY)) != nullptr)
        setTextFont(parseFont(s));
    if ((s = property->getValue(TEXT_COLOR_PROPERTY)) != nullptr)
        setTextColor(parseColor(s));
    if ((s = property->getValue(LABEL_PROPERTY)) != nullptr)
        setLabel(s);
    if ((s = property->getValue(LABEL_FONT_PROPERTY)) != nullptr)
        setLabelFont(parseFont(s));
    if ((s = property->getValue(TEXT_COLOR_PROPERTY)) != nullptr)
        setLabelColor(parseColor(s));

}

const char **ProgressMeterFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {BACKGROUND_COLOR_PROPERTY, STRIP_COLOR_PROPERTY, CORNER_RADIUS_PROPERTY, BORDER_WIDTH_PROPERTY,
                                   MIN_PROPERTY, MAX_PROPERTY, TEXT_PROPERTY, TEXT_FONT_PROPERTY, TEXT_COLOR_PROPERTY, LABEL_PROPERTY,
                                   LABEL_FONT_PROPERTY, LABEL_COLOR_PROPERTY, POS_PROPERTY, SIZE_PROPERTY,ANCHOR_PROPERTY, BOUNDS_PROPERTY, nullptr};
        concatArrays(keys, cGroupFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void ProgressMeterFigure::layout()
{
    Rectangle bounds = borderFigure->getBounds();
    stripFigure->setBounds(bounds);
    backgroundFigure->setBounds(bounds);

    valueFigure->setPosition(Point(bounds.x + bounds.width/2, bounds.y + bounds.height/2));
    labelFigure->setPosition(Point(bounds.x + bounds.width/2, bounds.y + bounds.height));
}

void ProgressMeterFigure::addChildren()
{
    borderFigure = new cRectangleFigure("border");
    stripFigure = new cRectangleFigure("strip");
    backgroundFigure = new cRectangleFigure("background");
    valueFigure = new cTextFigure("value");
    labelFigure = new cTextFigure("label");

    backgroundFigure->setOutlined(false);
    backgroundFigure->setFilled(true);
    backgroundFigure->setFillColor(Color("grey"));

    stripFigure->setOutlined(false);
    stripFigure->setFilled(true);
    stripFigure->setFillColor(Color("lightblue"));

    borderFigure->setOutlined(true);
    borderFigure->setFilled(false);
    borderFigure->setLineWidth(2);

    valueFigure->setAnchor(ANCHOR_CENTER);
    labelFigure->setAnchor(ANCHOR_N);

    addFigure(backgroundFigure);
    addFigure(stripFigure);
    addFigure(borderFigure);
    addFigure(valueFigure);
    addFigure(labelFigure);
}

void ProgressMeterFigure::setValue(int series, simtime_t timestamp, double newValue)
{
    ASSERT(series == 0);
    // Note: we currently ignore timestamp
    if (value != newValue) {
        value = newValue;
        refresh();
    }
}

void ProgressMeterFigure::refresh()
{
    // adjust strip
    double stripValue = std::isnan(value) ? min : std::max(min, std::min(max, value));
    cFigure::Rectangle bounds = borderFigure->getBounds();
    bounds.width = bounds.width * (stripValue - min)/(max - min);
    stripFigure->setBounds(bounds);

    // update displayed number
    if (std::isnan(value))
        valueFigure->setText("");
    else {
        char buf[32];
        double percent = (value - min) / (max - min) * 100;
        sprintf(buf, getText(), value, percent);
        valueFigure->setText(buf);
    }
}

#endif // omnetpp 5

// } // namespace inet
