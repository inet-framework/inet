//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/figures/CounterFigure.h"

#include "inet/common/INETUtils.h"

namespace inet {

Register_Figure("counter", CounterFigure);

static const int PADDING = 4;
static const int DIGIT_PADDING = 2;
static const double DIGIT_WIDTH_PERCENT = 0.9;
static const double DIGIT_HEIGHT_PERCENT = 1.1;
static const char *INIT_BACKGROUND_COLOR = "#808080";
static const char *INIT_DIGIT_BACKGROUND_COLOR = "white";
static const char *INIT_DIGIT_BORDER_COLOR = "black";
static const char *INIT_DIGIT_TEXT_COLOR = "black";
static const char *INIT_FONT_NAME = "Arial";
static const double INIT_FONT_SIZE = 16;
static const int INIT_DECIMAL_PLACES = 3;

static const char *PKEY_BACKGROUND_COLOR = "backgroundColor";
static const char *PKEY_DECIMAL_PLACES = "decimalPlaces";
static const char *PKEY_DIGIT_BACKGROUND_COLOR = "digitBackgroundColor";
static const char *PKEY_DIGIT_BORDER_COLOR = "digitBorderColor";
static const char *PKEY_DIGIT_FONT = "digitFont";
static const char *PKEY_DIGIT_COLOR = "digitColor";
static const char *PKEY_LABEL = "label";
static const char *PKEY_LABEL_FONT = "labelFont";
static const char *PKEY_LABEL_COLOR = "labelColor";
static const char *PKEY_INITIAL_VALUE = "initialValue";
static const char *PKEY_POS = "pos";
static const char *PKEY_ANCHOR = "anchor";
static const char *PKEY_LABEL_OFFSET = "labelOffset";

CounterFigure::CounterFigure(const char *name) : cGroupFigure(name)
{
    addChildren();
}

const cFigure::Color& CounterFigure::getBackgroundColor() const
{
    return backgroundFigure->getFillColor();
}

void CounterFigure::setBackgroundColor(const Color& color)
{
    backgroundFigure->setFillColor(color);
}

int CounterFigure::getDecimalPlaces() const
{
    return digits.size();
}

void CounterFigure::setDecimalPlaces(int number)
{
    ASSERT(number > 0);
    if (digits.size() != (unsigned int)number) {
        if (digits.size() > (unsigned int)number)
            // Remove unnecessary figures from canvas
            for (int i = digits.size() - 1; i > number - 1; --i) {
                delete removeFigure(digits[i].bounds);
                delete removeFigure(digits[i].text);
                digits.pop_back();
            }
        else
            // Add figure to canvas if it's necessary
            while (digits.size() < (unsigned int)number) {
                Digit digit(new cRectangleFigure(), new cTextFigure());
                digit.bounds->setFilled(true);
                digit.bounds->setFillColor(getDigitBackgroundColor());
                digit.bounds->setZoomLineWidth(true);
                digit.text->setAnchor(ANCHOR_CENTER);
                digit.text->setFont(getDigitFont());

                addFigure(digit.bounds);
                addFigure(digit.text);

                digits.push_back(digit);
            }

        calculateBounds();
        layout();
    }
}

cFigure::Color CounterFigure::getDigitBackgroundColor() const
{
    return digits.size() ? digits[0].bounds->getFillColor() : Color(INIT_DIGIT_BACKGROUND_COLOR);
}

void CounterFigure::setDigitBackgroundColor(const Color& color)
{
    for (Digit digit : digits)
        digit.bounds->setFillColor(color);
}

cFigure::Color CounterFigure::getDigitBorderColor() const
{
    return digits.size() ? digits[0].bounds->getLineColor() : Color(INIT_DIGIT_BORDER_COLOR);
}

void CounterFigure::setDigitBorderColor(const Color& color)
{
    for (Digit digit : digits)
        digit.bounds->setLineColor(color);
}

cFigure::Font CounterFigure::getDigitFont() const
{
    return digits.size() ? digits[0].text->getFont() : Font(INIT_FONT_NAME, INIT_FONT_SIZE, cFigure::FONT_BOLD);
}

void CounterFigure::setDigitFont(const Font& font)
{
    for (Digit digit : digits)
        digit.text->setFont(font);

    calculateBounds();
    layout();
}

cFigure::Color CounterFigure::getDigitColor() const
{
    return digits.size() ? digits[0].text->getColor() : Color(INIT_DIGIT_TEXT_COLOR);
}

void CounterFigure::setDigitColor(const Color& color)
{
    for (Digit digit : digits)
        digit.text->setColor(color);
}

const char *CounterFigure::getLabel() const
{
    return labelFigure->getText();
}

void CounterFigure::setLabel(const char *text)
{
    labelFigure->setText(text);
}

int CounterFigure::getLabelOffset() const
{
    return labelOffset;
}

void CounterFigure::setLabelOffset(int offset)
{
    if (labelOffset != offset) {
        labelOffset = offset;
        labelFigure->setPosition(Point(backgroundFigure->getBounds().x + backgroundFigure->getBounds().width / 2, backgroundFigure->getBounds().y + backgroundFigure->getBounds().height + labelOffset));
    }
}

const cFigure::Font& CounterFigure::getLabelFont() const
{
    return labelFigure->getFont();
}

void CounterFigure::setLabelFont(const Font& font)
{
    labelFigure->setFont(font);
}

const cFigure::Color& CounterFigure::getLabelColor() const
{
    return labelFigure->getColor();
}

void CounterFigure::setLabelColor(const Color& color)
{
    labelFigure->setColor(color);
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

void CounterFigure::setPos(const Point& pos)
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
    if (anchor != this->anchor) {
        this->anchor = anchor;

        calculateBounds();
        layout();
    }
}

// Get North West Point according to anchor
cFigure::Point CounterFigure::calculateRealPos(const Point& pos)
{
    Rectangle bounds = backgroundFigure->getBounds();
    Point position = pos;
    switch (anchor) {
        case cFigure::ANCHOR_CENTER:
            position.x -= bounds.width / 2;
            position.y -= bounds.height / 2;
            break;

        case cFigure::ANCHOR_N:
            position.x -= bounds.width / 2;
            break;

        case cFigure::ANCHOR_E:
            position.x -= bounds.width;
            position.y -= bounds.height / 2;
            break;

        case cFigure::ANCHOR_S:
        case cFigure::ANCHOR_BASELINE_MIDDLE:
            position.x -= bounds.width / 2;
            position.y -= bounds.height;
            break;

        case cFigure::ANCHOR_W:
            position.y -= bounds.height / 2;
            break;

        case cFigure::ANCHOR_NW:
            break;

        case cFigure::ANCHOR_NE:
            position.x -= bounds.width;
            break;

        case cFigure::ANCHOR_SE:
        case cFigure::ANCHOR_BASELINE_END:
            position.x -= bounds.width;
            position.y -= bounds.height;
            break;

        case cFigure::ANCHOR_SW:
        case cFigure::ANCHOR_BASELINE_START:
            position.y -= bounds.width;
            break;
    }
    return position;
}

void CounterFigure::calculateBounds()
{
    double rectWidth = getDigitFont().pointSize * DIGIT_WIDTH_PERCENT;
    double rectHeight = getDigitFont().pointSize * DIGIT_HEIGHT_PERCENT;

    Rectangle bounds = backgroundFigure->getBounds();
    backgroundFigure->setBounds(Rectangle(0, 0, 2 * PADDING + (rectWidth + DIGIT_PADDING) * digits.size(), rectHeight + 2 * PADDING));
    Point pos = calculateRealPos(Point(bounds.x, bounds.y));
    bounds = backgroundFigure->getBounds();
    bounds.x = pos.x;
    bounds.y = pos.y;
    backgroundFigure->setBounds(bounds);
}

void CounterFigure::parse(cProperty *property)
{
    cGroupFigure::parse(property);

    const char *s;

    setPos(parsePoint(property, PKEY_POS, 0));

    if ((s = property->getValue(PKEY_BACKGROUND_COLOR)) != nullptr)
        setBackgroundColor(parseColor(s));
    if ((s = property->getValue(PKEY_ANCHOR)) != nullptr)
        setAnchor(parseAnchor(s));
    if ((s = property->getValue(PKEY_DECIMAL_PLACES)) != nullptr)
        setDecimalPlaces(utils::atod(s));
    else
        setDecimalPlaces(INIT_DECIMAL_PLACES);
    if ((s = property->getValue(PKEY_DIGIT_FONT)) != nullptr)
        setDigitFont(parseFont(s));
    if ((s = property->getValue(PKEY_DIGIT_BACKGROUND_COLOR)) != nullptr)
        setDigitBackgroundColor(parseColor(s));
    if ((s = property->getValue(PKEY_DIGIT_BORDER_COLOR)) != nullptr)
        setDigitBorderColor(Color(parseColor(s)));
    if ((s = property->getValue(PKEY_DIGIT_COLOR)) != nullptr)
        setDigitColor(parseColor(s));
    if ((s = property->getValue(PKEY_LABEL)) != nullptr)
        setLabel(s);
    if ((s = property->getValue(PKEY_LABEL_OFFSET)) != nullptr)
        setLabelOffset(atoi(s));
    if ((s = property->getValue(PKEY_LABEL_FONT)) != nullptr)
        setLabelFont(parseFont(s));
    if ((s = property->getValue(PKEY_LABEL_COLOR)) != nullptr)
        setLabelColor(parseColor(s));
    if ((s = property->getValue(PKEY_INITIAL_VALUE)) != nullptr)
        setValue(0, simTime(), utils::atod(s));

    refresh();
}

const char **CounterFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {
            PKEY_BACKGROUND_COLOR, PKEY_DECIMAL_PLACES, PKEY_DIGIT_BACKGROUND_COLOR,
            PKEY_DIGIT_BORDER_COLOR, PKEY_DIGIT_FONT, PKEY_DIGIT_COLOR, PKEY_LABEL, PKEY_LABEL_FONT,
            PKEY_LABEL_COLOR, PKEY_INITIAL_VALUE, PKEY_POS, PKEY_ANCHOR, PKEY_LABEL_OFFSET, nullptr
        };
        concatArrays(keys, cGroupFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void CounterFigure::layout()
{
    Rectangle bounds = backgroundFigure->getBounds();

    // Add frame
    bounds.x += PADDING + DIGIT_PADDING / 2;

    for (uint32_t i = 0; i < digits.size(); ++i) {
        double rectWidth = getDigitFont().pointSize * DIGIT_WIDTH_PERCENT;
        double rectHeight = getDigitFont().pointSize * DIGIT_HEIGHT_PERCENT;
        double x = bounds.x + (rectWidth + DIGIT_PADDING) * i;
        double y = bounds.y + PADDING;

        digits[i].bounds->setBounds(Rectangle(x, y, rectWidth, rectHeight));
        digits[i].text->setPosition(digits[i].bounds->getBounds().getCenter());
    }

    labelFigure->setPosition(Point(bounds.x + bounds.width / 2, bounds.y + bounds.height + labelOffset));
}

void CounterFigure::addChildren()
{
    backgroundFigure = new cRectangleFigure("background");
    labelFigure = new cTextFigure("label");

    backgroundFigure->setFilled(true);
    backgroundFigure->setFillColor(Color(INIT_BACKGROUND_COLOR));
    backgroundFigure->setZoomLineWidth(true);

    labelFigure->setAnchor(ANCHOR_N);

    addFigure(backgroundFigure);
    addFigure(labelFigure);
}

void CounterFigure::setValue(int series, simtime_t timestamp, double newValue)
{
    ASSERT(series == 0);

    // Note: we currently ignore timestamp
    if (value != newValue) {
        value = newValue;
        refresh();
    }
}

void CounterFigure::refresh()
{
    // update displayed number
    int max = std::pow(10, digits.size());
    if (std::isnan(value))
        for (Digit digit : digits)
            digit.text->setText("");

    else if (value >= max || value < 0)
        for (Digit digit : digits)
            digit.text->setText("*");

    else {
        int pow = 1;
        for (int i = digits.size() - 1; i >= 0; --i) {
            char buf[32];
            pow *= 10;
            int actValue = ((int)value % pow) / (pow / 10);
            sprintf(buf, "%d", actValue);
            digits[i].text->setText(buf);
        }
    }
}

} // namespace inet

