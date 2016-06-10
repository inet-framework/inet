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
#include "CounterFigure.h"

//TODO namespace inet { -- for the moment commented out, as OMNeT++ 5.0 cannot instantiate a figure from a namespace
using namespace inet;

Register_Class(CounterFigure);

#if OMNETPP_VERSION >= 0x500

#define M_PI 3.14159265358979323846

static const int FRAME_SIZE = 4;
static const int DIGIT_FRAME_SIZE = 2;
static const double DIGIT_WIDTH_PERCENT = 0.9;
static const double DIGIT_HEIGHT_PERCENT = 1;

static const char *PKEY_BACKGROUND_COLOR = "backgroundColor";
static const char *PKEY_DECIMAL_PLACES = "decimalPlaces";
static const char *PKEY_DIGIT_BACKGROUND_COLOR = "digitBackgoundColor";
static const char *PKEY_DIGIT_BORDER_COLOR = "digitBorderColor";
static const char *PKEY_DIGIT_FONT = "digitFont";
static const char *PKEY_DIGIT_COLOR = "digitColor";
static const char *PKEY_LABEL = "label";
static const char *PKEY_LABEL_FONT = "labelFont";
static const char *PKEY_LABEL_COLOR = "labelColor";
static const char *LABEL_POS = "labelPos";
static const char *LABEL_ANCHOR = "labelAnchor";
static const char *PKEY_POS = "pos";
static const char *PKEY_ANCHOR = "anchor";

CounterFigure::CounterFigure(const char *name) : cGroupFigure(name)
{
    addChildren();
}

cFigure::Color CounterFigure::getBackgroundColor() const
{
    return backgroundFigure->getFillColor();
}

void CounterFigure::setBackgroundColor(cFigure::Color color)
{
    backgroundFigure->setFillColor(color);
}

int CounterFigure::getDecimalPlaces() const
{
    return decimalNumber;
}

void CounterFigure::setDecimalPlaces(int number)
{
    ASSERT(number > 0);
    if(decimalNumber != number)
    {
        prevDecimalNumber = decimalNumber;
        decimalNumber = number;

        calculateBounds();
        layout();
    }
}

cFigure::Color CounterFigure::getDigitBackgroundColor() const
{
    return digitRectFigures.size() ? digitRectFigures[0]->getFillColor() : Color();
}

void CounterFigure::setDigitBackgroundColor(cFigure::Color color)
{
    for(cRectangleFigure *figure : digitRectFigures)
        figure->setFillColor(color);
}

cFigure::Color CounterFigure::getDigitBorderColor() const
{
    return digitRectFigures.size() ? digitRectFigures[0]->getLineColor() : Color();
}

void CounterFigure::setDigitBorderColor(cFigure::Color color)
{
    for(cRectangleFigure *figure : digitRectFigures)
        figure->setLineColor(color);
}

cFigure::Font CounterFigure::getDigitFont() const
{
    return digitTextFigures.size() ? digitTextFigures[0]->getFont() : Font();
}

void CounterFigure::setDigitFont(cFigure::Font font)
{
    for(cTextFigure *figure : digitTextFigures)
        figure->setFont(font);

    calculateBounds();
    layout();
}

cFigure::Color CounterFigure::getDigitColor() const
{
    return digitTextFigures.size() ? digitTextFigures[0]->getColor() : Color();
}

void CounterFigure::setDigitColor(cFigure::Color color)
{
    for(cTextFigure *figure : digitTextFigures)
        figure->setColor(color);
}

const char *CounterFigure::getLabel() const
{
    return labelFigure->getText();
}

void CounterFigure::setLabel(const char *text)
{
    labelFigure->setText(text);
}

cFigure::Font CounterFigure::getLabelFont() const
{
    return labelFigure->getFont();
}

void CounterFigure::setLabelFont(cFigure::Font font)
{
    labelFigure->setFont(font);
}

cFigure::Color CounterFigure::getLabelColor() const
{
    return labelFigure->getColor();
}

void CounterFigure::setLabelColor(cFigure::Color color)
{
    labelFigure->setColor(color);
}

cFigure::Point CounterFigure::getLabelPos() const
{
    return labelFigure->getPosition();
}

void CounterFigure::setLabelPos(Point pos)
{
    labelFigure->setPosition(pos);
}

cFigure::Anchor CounterFigure::getLabelAnchor() const
{
    return labelFigure->getAnchor();
}

void CounterFigure::setLabelAnchor(Anchor anchor)
{
    labelFigure->setAnchor(anchor);
}

cFigure::Point CounterFigure::getPos() const
{
    Rectangle bounds = backgroundFigure->getBounds();
        switch (anchor) {
            case cFigure::ANCHOR_CENTER:
                bounds.x += bounds.width / 2;
                bounds.y += bounds.height / 2;
                break;

            case cFigure::ANCHOR_N:
                bounds.x += bounds.width / 2;
                break;

            case cFigure::ANCHOR_E:
                bounds.x += bounds.width;
                bounds.y += bounds.height / 2;
                break;

            case cFigure::ANCHOR_S:
            case cFigure::ANCHOR_BASELINE_MIDDLE:
                bounds.x += bounds.width / 2;
                bounds.y += bounds.height;
                break;

            case cFigure::ANCHOR_W:
                bounds.y += bounds.height / 2;
                break;

            case cFigure::ANCHOR_NW:
                break;

            case cFigure::ANCHOR_NE:
                bounds.x += bounds.width;
                break;

            case cFigure::ANCHOR_SE:
            case cFigure::ANCHOR_BASELINE_END:
                bounds.x += bounds.width;
                bounds.y += bounds.height;
                break;

            case cFigure::ANCHOR_SW:
            case cFigure::ANCHOR_BASELINE_START:
                bounds.y += bounds.width;
                break;
        }
        return Point(bounds.x, bounds.y);
}

void CounterFigure::setPos(Point pos)
{
    Rectangle bounds = backgroundFigure->getBounds();
    Point backgroundPos = calculateRealPos(pos);
    bounds.x = backgroundPos.x;
    bounds.y = backgroundPos.y;

    backgroundFigure->setBounds(bounds);
    layout();
}

cFigure::Anchor CounterFigure::getAnchor() const
{
    return anchor;
}

void CounterFigure::setAnchor(Anchor anchor)
{
    if(anchor != this->anchor)
    {
        this->anchor = anchor;

        calculateBounds();
        layout();
    }
}

// Get North West Point according to anchor
cFigure::Point CounterFigure::calculateRealPos(Point pos)
{
    Rectangle bounds = backgroundFigure->getBounds();
    switch (anchor) {
        case cFigure::ANCHOR_CENTER:
            pos.x -= bounds.width / 2;
            pos.y -= bounds.height / 2;
            break;

        case cFigure::ANCHOR_N:
            pos.x -= bounds.width / 2;
            break;

        case cFigure::ANCHOR_E:
            pos.x -= bounds.width;
            pos.y -= bounds.height / 2;
            break;

        case cFigure::ANCHOR_S:
        case cFigure::ANCHOR_BASELINE_MIDDLE:
            pos.x -= bounds.width / 2;
            pos.y -= bounds.height;
            break;

        case cFigure::ANCHOR_W:
            pos.y -= bounds.height / 2;
            break;

        case cFigure::ANCHOR_NW:
            break;

        case cFigure::ANCHOR_NE:
            pos.x -= bounds.width;
            break;

        case cFigure::ANCHOR_SE:
        case cFigure::ANCHOR_BASELINE_END:
            pos.x -= bounds.width;
            pos.y -= bounds.height;
            break;

        case cFigure::ANCHOR_SW:
        case cFigure::ANCHOR_BASELINE_START:
            pos.y -= bounds.width;
            break;
    }
    return Point(pos.x, pos.y);
}

void CounterFigure::calculateBounds()
{
    double rectWidth = getDigitFont().pointSize * DIGIT_WIDTH_PERCENT;
    double rectHeight = getDigitFont().pointSize * DIGIT_HEIGHT_PERCENT;

    Rectangle bounds = backgroundFigure->getBounds();
    backgroundFigure->setBounds(Rectangle(0, 0, 2*FRAME_SIZE + (rectWidth + DIGIT_FRAME_SIZE) * decimalNumber,
                                          rectHeight + 2*FRAME_SIZE));
    Point pos = calculateRealPos(Point(bounds.x, bounds.y));
    bounds = backgroundFigure->getBounds();
    bounds.x = pos.x;
    bounds.y = pos.y;
    backgroundFigure->setBounds(bounds);
}

void CounterFigure::parse(cProperty *property)
{
    cGroupFigure::parse(property);

    setPos(parsePoint(property, PKEY_POS, 0));

    const char *s;
    if ((s = property->getValue(PKEY_BACKGROUND_COLOR)) != nullptr)
        setBackgroundColor(parseColor(s));
    if ((s = property->getValue(PKEY_ANCHOR)) != nullptr)
        setAnchor(parseAnchor(s));
    if ((s = property->getValue(PKEY_DIGIT_FONT)) != nullptr)
        setDigitFont(parseFont(s));
    if ((s = property->getValue(PKEY_DECIMAL_PLACES)) != nullptr)
        setDecimalPlaces(atoi(s));
    if ((s = property->getValue(PKEY_DIGIT_BACKGROUND_COLOR)) != nullptr)
        setDigitBackgroundColor(parseColor(s));
    if ((s = property->getValue(PKEY_DIGIT_BORDER_COLOR)) != nullptr)
        setDigitBorderColor(Color(parseColor(s)));
    if ((s = property->getValue(PKEY_DIGIT_COLOR)) != nullptr)
        setDigitColor(parseColor(s));
    if ((s = property->getValue(PKEY_LABEL)) != nullptr)
        setLabel(s);
    if ((s = property->getValue(PKEY_LABEL_FONT)) != nullptr)
        setLabelFont(parseFont(s));
    if ((s = property->getValue(PKEY_LABEL_COLOR)) != nullptr)
        setLabelColor(parseColor(s));
    if ((s = property->getValue(LABEL_POS)) != nullptr)
        setLabelPos(parsePoint(property, LABEL_POS, 0));
    if ((s = property->getValue(LABEL_ANCHOR)) != nullptr)
        setLabelAnchor(parseAnchor(s));
}

const char **CounterFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {PKEY_BACKGROUND_COLOR, PKEY_DECIMAL_PLACES, PKEY_DIGIT_BACKGROUND_COLOR,
                                   PKEY_DIGIT_BORDER_COLOR, PKEY_DIGIT_FONT, PKEY_DIGIT_COLOR, PKEY_LABEL, PKEY_LABEL_FONT,
                                   PKEY_LABEL_COLOR, LABEL_POS, LABEL_ANCHOR, PKEY_POS, PKEY_ANCHOR, nullptr};
        concatArrays(keys, cGroupFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void CounterFigure::layout()
{
    ASSERT(digitRectFigures.size() == digitTextFigures.size());
    Rectangle bounds = backgroundFigure->getBounds();

    // Remove unnecessary figures from canvas
    for(int i = decimalNumber; i < prevDecimalNumber; ++i)
    {
        removeFigure(digitRectFigures[i]);
        removeFigure(digitTextFigures[i]);
    }
    // Add figure to canvas if it's necessary
    for(int i = prevDecimalNumber; i < decimalNumber; ++i)
    {
        if(i == digitRectFigures.size())
        {
            digitRectFigures.push_back(new cRectangleFigure());
            digitTextFigures.push_back(new cTextFigure());

            digitRectFigures[i]->setFilled(true);
            digitRectFigures[i]->setFillColor(getDigitBackgroundColor());

            digitTextFigures[i]->setAnchor(ANCHOR_CENTER);
            digitTextFigures[i]->setFont(getDigitFont());
        }
        addFigure(digitRectFigures[i]);
        addFigure(digitTextFigures[i]);
    }

    // Add frame
    bounds.x += FRAME_SIZE + DIGIT_FRAME_SIZE/2;

    for(int i = 0; i < decimalNumber; ++i)
    {
        double rectWidth = getDigitFont().pointSize * DIGIT_WIDTH_PERCENT;
        double rectHeight = getDigitFont().pointSize * DIGIT_HEIGHT_PERCENT;
        double x = bounds.x + (rectWidth + DIGIT_FRAME_SIZE)*i;
        double y = bounds.y + FRAME_SIZE;

        digitRectFigures[i]->setBounds(Rectangle(x, y, rectWidth, rectHeight));
        digitTextFigures[i]->setPosition(digitRectFigures[i]->getBounds().getCenter());
    }
}

void CounterFigure::addChildren()
{
    backgroundFigure = new cRectangleFigure("background");
    digitRectFigures.push_back(new cRectangleFigure());
    digitTextFigures.push_back(new cTextFigure());
    labelFigure = new cTextFigure("label");

    backgroundFigure->setFilled(true);
    backgroundFigure->setFillColor(Color("grey"));

    digitRectFigures[0]->setFilled(true);
    digitRectFigures[0]->setFillColor("lightgrey");

    digitTextFigures[0]->setAnchor(ANCHOR_CENTER);
    digitTextFigures[0]->setFont(Font("", 12, cFigure::FONT_BOLD));

    labelFigure->setAnchor(ANCHOR_N);

    addFigure(backgroundFigure);
    addFigure(digitRectFigures[0]);
    addFigure(digitTextFigures[0]);
    addFigure(labelFigure);
}

void CounterFigure::setValue(int series, simtime_t timestamp, double newValue)
{
    ASSERT(series == 0 && newValue >= 0);

    // Note: we currently ignore timestamp
    if (value != newValue) {
        value = newValue;
        refresh();
    }
}

void CounterFigure::refresh()
{
    // update displayed number
    if (std::isnan(value))
        for(cTextFigure *figure : digitTextFigures)
            figure->setText("");
    else {
        int max = std::pow(10, decimalNumber);
        if(value >= max)
            value = max - 1;

        int pow = 1;
        for(int i = decimalNumber - 1; i >= 0; --i)
        {
            char buf[32];
            pow *= 10;
            int actValue = (value % pow) / (pow/10);
            sprintf(buf, "%d", actValue);
            digitTextFigures[i]->setText(buf);
        }
    }
}

#endif // omnetpp 5

// } // namespace inet
