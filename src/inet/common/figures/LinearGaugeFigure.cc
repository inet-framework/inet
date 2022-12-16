//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/figures/LinearGaugeFigure.h"

#include "inet/common/INETUtils.h"

namespace inet {

Register_Figure("linearGauge", LinearGaugeFigure);

static const double BORDER_WIDTH_PERCENT = 0.05;
static const double AXIS_WIDTH_PERCENT = 0.03;
static const double FRAME_PERCENT = 0.1;
static const double TICK_BIG_LENGTH_PERCENT = 0.17;
static const double TICK_SMALL_LENGTH_PERCENT = 0.27;
static const double NUMBER_Y_PERCENT = 0.01;
static const double NUMBER_FONTSIZE_PERCENT = 0.3;
static const double NEEDLE_WIDTH_PERCENT = 0.05;
static const double NEEDLE_HEIGHT_PERCENT = 0.6;
static const double NEEDLE_OFFSET_PERCENT = 0.03;

// Properties
static const char *PKEY_BACKGROUND_COLOR = "backgroundColor";
static const char *PKEY_NEEDLE_COLOR = "needleColor";
static const char *PKEY_CORNER_RADIUS = "cornerRadius";
static const char *PKEY_MIN_VALUE = "minValue";
static const char *PKEY_MAX_VALUE = "maxValue";
static const char *PKEY_LABEL = "label";
static const char *PKEY_LABEL_FONT = "labelFont";
static const char *PKEY_LABEL_COLOR = "labelColor";
static const char *PKEY_TICK_SIZE = "tickSize";
static const char *PKEY_INITIAL_VALUE = "initialValue";
static const char *PKEY_POS = "pos";
static const char *PKEY_SIZE = "size";
static const char *PKEY_ANCHOR = "anchor";
static const char *PKEY_BOUNDS = "bounds";
static const char *PKEY_LABEL_OFFSET = "labelOffset";

LinearGaugeFigure::LinearGaugeFigure(const char *name) : cGroupFigure(name)
{
    addChildren();
}

LinearGaugeFigure::~LinearGaugeFigure()
{
    // delete figures which is not in canvas
    for (uint32_t i = numTicks; i < tickFigures.size(); ++i) {
        dropAndDelete(tickFigures[i]);
        dropAndDelete(numberFigures[i]);
    }
}

const cFigure::Rectangle& LinearGaugeFigure::getBounds() const
{
    return backgroundFigure->getBounds();
}

void LinearGaugeFigure::setBounds(const Rectangle& rect)
{
    backgroundFigure->setBounds(rect);
    layout();
}

const cFigure::Color& LinearGaugeFigure::getBackgroundColor() const
{
    return backgroundFigure->getFillColor();
}

void LinearGaugeFigure::setBackgroundColor(const Color& color)
{
    backgroundFigure->setFillColor(color);
}

const cFigure::Color& LinearGaugeFigure::getNeedleColor() const
{
    return needle->getLineColor();
}

void LinearGaugeFigure::setNeedleColor(const Color& color)
{
    needle->setLineColor(color);
}

const char *LinearGaugeFigure::getLabel() const
{
    return labelFigure->getText();
}

void LinearGaugeFigure::setLabel(const char *text)
{
    labelFigure->setText(text);
}

int LinearGaugeFigure::getLabelOffset() const
{
    return labelOffset;
}

void LinearGaugeFigure::setLabelOffset(int offset)
{
    if (labelOffset != offset) {
        labelOffset = offset;
        labelFigure->setPosition(Point(getBounds().getCenter().x, getBounds().y + getBounds().height + labelOffset));
    }
}

const cFigure::Font& LinearGaugeFigure::getLabelFont() const
{
    return labelFigure->getFont();
}

void LinearGaugeFigure::setLabelFont(const Font& font)
{
    labelFigure->setFont(font);
}

const cFigure::Color& LinearGaugeFigure::getLabelColor() const
{
    return labelFigure->getColor();
}

void LinearGaugeFigure::setLabelColor(const Color& color)
{
    labelFigure->setColor(color);
}

double LinearGaugeFigure::getMinValue() const
{
    return min;
}

void LinearGaugeFigure::setMinValue(double value)
{
    if (min != value) {
        min = value;
        redrawTicks();
        refresh();
    }
}

double LinearGaugeFigure::getMaxValue() const
{
    return max;
}

void LinearGaugeFigure::setMaxValue(double value)
{
    if (max != value) {
        max = value;
        redrawTicks();
        refresh();
    }
}

double LinearGaugeFigure::getTickSize() const
{
    return tickSize;
}

void LinearGaugeFigure::setTickSize(double value)
{
    if (tickSize != value) {
        tickSize = value;
        redrawTicks();
        refresh();
    }
}

double LinearGaugeFigure::getCornerRadius() const
{
    return backgroundFigure->getCornerRx();
}

void LinearGaugeFigure::setCornerRadius(double radius)
{
    backgroundFigure->setCornerRadius(radius);
}

void LinearGaugeFigure::parse(cProperty *property)
{
    cGroupFigure::parse(property);

    const char *s;

    setBounds(parseBounds(property, getBounds()));

    // Set default
    redrawTicks();

    if ((s = property->getValue(PKEY_BACKGROUND_COLOR)) != nullptr)
        setBackgroundColor(parseColor(s));
    if ((s = property->getValue(PKEY_NEEDLE_COLOR)) != nullptr)
        setNeedleColor(parseColor(s));
    if ((s = property->getValue(PKEY_LABEL)) != nullptr)
        setLabel(s);
    if ((s = property->getValue(PKEY_LABEL_OFFSET)) != nullptr)
        setLabelOffset(atoi(s));
    if ((s = property->getValue(PKEY_LABEL_FONT)) != nullptr)
        setLabelFont(parseFont(s));
    if ((s = property->getValue(PKEY_LABEL_COLOR)) != nullptr)
        setLabelColor(parseColor(s));
    // This must be initialize before min and max because it is possible to be too much unnecessary tick and number
    if ((s = property->getValue(PKEY_TICK_SIZE)) != nullptr)
        setTickSize(utils::atod(s));
    if ((s = property->getValue(PKEY_MIN_VALUE)) != nullptr)
        setMinValue(utils::atod(s));
    if ((s = property->getValue(PKEY_MAX_VALUE)) != nullptr)
        setMaxValue(utils::atod(s));
    if ((s = property->getValue(PKEY_CORNER_RADIUS)) != nullptr)
        setCornerRadius(utils::atod(s));
    if ((s = property->getValue(PKEY_INITIAL_VALUE)) != nullptr)
        setValue(0, simTime(), utils::atod(s));
}

const char **LinearGaugeFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {
            PKEY_BACKGROUND_COLOR, PKEY_NEEDLE_COLOR, PKEY_LABEL, PKEY_LABEL_FONT,
            PKEY_LABEL_COLOR, PKEY_MIN_VALUE, PKEY_MAX_VALUE, PKEY_TICK_SIZE,
            PKEY_CORNER_RADIUS, PKEY_INITIAL_VALUE, PKEY_POS, PKEY_SIZE, PKEY_ANCHOR,
            PKEY_BOUNDS, PKEY_LABEL_OFFSET, nullptr
        };
        concatArrays(keys, cGroupFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void LinearGaugeFigure::addChildren()
{
    backgroundFigure = new cRectangleFigure("background");
    needle = new cLineFigure("needle");
    labelFigure = new cTextFigure("label");
    axisFigure = new cLineFigure("axis");

    backgroundFigure->setFilled(true);
    backgroundFigure->setFillColor(Color("#b8afa6"));

    needle->setLineColor("red");
    labelFigure->setAnchor(cFigure::ANCHOR_N);

    addFigure(backgroundFigure);
    addFigure(axisFigure);
    addFigure(needle);
    addFigure(labelFigure);
}

void LinearGaugeFigure::setValue(int series, simtime_t timestamp, double newValue)
{
    ASSERT(series == 0);
    // Note: we currently ignore timestamp
    if (value != newValue) {
        value = newValue;
        refresh();
    }
}

void LinearGaugeFigure::setTickGeometry(cLineFigure *tick, int index)
{
    double axisWidth = axisFigure->getEnd().x - axisFigure->getStart().x - axisFigure->getLineWidth();
    double x = axisFigure->getStart().x + axisFigure->getLineWidth() / 2
        + axisWidth * (index * tickSize + shifting) / (max - min);
    tick->setStart(Point(x, getBounds().getCenter().y));

    Point endPos = tick->getStart();
    endPos.y -= !(index % 3) ? getBounds().height * TICK_SMALL_LENGTH_PERCENT :
        getBounds().height * TICK_BIG_LENGTH_PERCENT;
    tick->setEnd(endPos);
    tick->setLineWidth(getBounds().height * AXIS_WIDTH_PERCENT);
}

void LinearGaugeFigure::setNumberGeometry(cTextFigure *number, int index)
{
    double axisWidth = axisFigure->getEnd().x - axisFigure->getStart().x - axisFigure->getLineWidth() / 2;
    double x = axisFigure->getStart().x + axisFigure->getLineWidth() / 2
        + axisWidth * (index * tickSize + shifting) / (max - min);
    Point textPos = Point(x, axisFigure->getStart().y + getBounds().height * NUMBER_Y_PERCENT);
    number->setPosition(textPos);
    number->setFont(cFigure::Font("", getBounds().height * NUMBER_FONTSIZE_PERCENT, 0));
}

void LinearGaugeFigure::setNeedleGeometry()
{
    needle->setLineWidth(getBounds().height * NEEDLE_WIDTH_PERCENT);

    double x = axisFigure->getStart().x + axisFigure->getLineWidth() / 2;
    double axisWidth = axisFigure->getEnd().x - axisFigure->getStart().x - axisFigure->getLineWidth();

    needle->setVisible(true);
    if (std::isnan(value)) {
        needle->setVisible(false);
        return;
    }
    else if (value < min)
        x -= getBounds().width * NEEDLE_OFFSET_PERCENT;
    else if (value > max)
        x = axisFigure->getEnd().x + getBounds().width * NEEDLE_OFFSET_PERCENT;
    else
        x += (value - min) * axisWidth / (max - min);

    needle->setStart(Point(x, axisFigure->getStart().y - getBounds().height * NEEDLE_HEIGHT_PERCENT / 2));
    needle->setEnd(Point(x, axisFigure->getStart().y + getBounds().height * NEEDLE_HEIGHT_PERCENT / 2));
}

void LinearGaugeFigure::redrawTicks()
{
    ASSERT(tickFigures.size() == numberFigures.size());

    double fraction = std::abs(fmod(min / tickSize, 1));
    shifting = tickSize * (min < 0 ? fraction : 1 - fraction);
    // if fraction == 0 then shifting == tickSize therefore don't have to shift the ticks
    if (shifting == tickSize)
        shifting = 0;

    int prevNumTicks = numTicks;
    numTicks = std::max(0.0, std::abs(max - min - shifting) / tickSize + 1);

    // Allocate ticks and numbers if needed
    if ((size_t)numTicks > tickFigures.size())
        while ((size_t)numTicks > tickFigures.size()) {
            cLineFigure *tick = new cLineFigure();
            cTextFigure *number = new cTextFigure();
            take(tick);
            take(number);

            number->setAnchor(cFigure::ANCHOR_N);

            tickFigures.push_back(tick);
            numberFigures.push_back(number);
        }

    // Add or remove figures from canvas according to previous number of ticks
    for (int i = numTicks; i < prevNumTicks; ++i) {
        removeFigure(tickFigures[i]);
        removeFigure(numberFigures[i]);
        take(tickFigures[i]);
        take(numberFigures[i]);
    }
    for (int i = prevNumTicks; i < numTicks; ++i) {
        drop(tickFigures[i]);
        drop(numberFigures[i]);
        tickFigures[i]->insertBelow(needle);
        numberFigures[i]->insertBelow(needle);
    }

    for (int i = 0; i < numTicks; ++i) {
        setTickGeometry(tickFigures[i], i);

        double number = min + i * tickSize + shifting;
        if (std::abs(number) < tickSize / 2)
            number = 0;

        char buf[32];
        sprintf(buf, "%g", number);
        numberFigures[i]->setText(buf);
        setNumberGeometry(numberFigures[i], i);
    }
}

void LinearGaugeFigure::layout()
{
    backgroundFigure->setLineWidth(getBounds().height * BORDER_WIDTH_PERCENT);

    double y = getBounds().getCenter().y;
    axisFigure->setLineWidth(getBounds().height * AXIS_WIDTH_PERCENT);
    axisFigure->setStart(Point(getBounds().x + getBounds().width * FRAME_PERCENT + axisFigure->getLineWidth() / 2, y));
    axisFigure->setEnd(Point(getBounds().x + getBounds().width * (1 - FRAME_PERCENT) + axisFigure->getLineWidth() / 2, y));

    for (int i = 0; i < numTicks; ++i) {
        setTickGeometry(tickFigures[i], i);
        setNumberGeometry(numberFigures[i], i);
    }

    setNeedleGeometry();
    labelFigure->setPosition(Point(getBounds().getCenter().x, getBounds().y + getBounds().height + labelOffset));
}

void LinearGaugeFigure::refresh()
{
    setNeedleGeometry();
}

} // namespace inet

