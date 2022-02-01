//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/figures/ThermometerFigure.h"

#include "inet/common/INETUtils.h"

namespace inet {

Register_Figure("thermometer", ThermometerFigure);

static const double CONTAINER_LINE_WIDTH_PERCENT = 0.05;
static const double TICK_LINE_WIDTH_PERCENT = 0.01;
static const double TICK_LENGTH_PERCENT = 0.2;
static const double FONT_SIZE_PERCENT = 0.05;
static const double CONTAINER_POS_PERCENT = 0.05;
static const double CONTAINER_WIDTH_PERCENT = 0.35;
static const double CONTAINER_OFFSET_PERCENT = 0.02;
static const double NUMBER_DISTANCE_FROM_TICK_PERCENT = 0.15;

static const char *PKEY_MERCURY_COLOR = "mercuryColor";
static const char *PKEY_LABEL = "label";
static const char *PKEY_LABEL_FONT = "labelFont";
static const char *PKEY_LABEL_COLOR = "labelColor";
static const char *PKEY_MIN_VALUE = "minValue";
static const char *PKEY_MAX_VALUE = "maxValue";
static const char *PKEY_TICK_SIZE = "tickSize";
static const char *PKEY_INITIAL_VALUE = "initialValue";
static const char *PKEY_POS = "pos";
static const char *PKEY_SIZE = "size";
static const char *PKEY_ANCHOR = "anchor";
static const char *PKEY_BOUNDS = "bounds";
static const char *PKEY_LABEL_OFFSET = "labelOffset";

ThermometerFigure::ThermometerFigure(const char *name) : cGroupFigure(name)
{
    addChildren();
}

ThermometerFigure::~ThermometerFigure()
{
    // delete figures which is not in canvas
    for (size_t i = numTicks; i < tickFigures.size(); ++i) {
        dropAndDelete(tickFigures[i]);
        dropAndDelete(numberFigures[i]);
    }
}

const cFigure::Rectangle& ThermometerFigure::getBounds() const
{
    return bounds;
}

void ThermometerFigure::setBounds(const Rectangle& rect)
{
    bounds = rect;
    layout();
}

const cFigure::Color& ThermometerFigure::getMercuryColor() const
{
    return mercuryFigure->getLineColor();
}

void ThermometerFigure::setMercuryColor(const Color& color)
{
    mercuryFigure->setFillColor(color);
}

const char *ThermometerFigure::getLabel() const
{
    return labelFigure->getText();
}

void ThermometerFigure::setLabel(const char *text)
{
    labelFigure->setText(text);
}

int ThermometerFigure::getLabelOffset() const
{
    return labelOffset;
}

void ThermometerFigure::setLabelOffset(int offset)
{
    if (labelOffset != offset) {
        labelOffset = offset;
        labelFigure->setPosition(Point(getBounds().getCenter().x, getBounds().y + getBounds().height + labelOffset));
    }
}

const cFigure::Font& ThermometerFigure::getLabelFont() const
{
    return labelFigure->getFont();
}

void ThermometerFigure::setLabelFont(const Font& font)
{
    labelFigure->setFont(font);
}

const cFigure::Color& ThermometerFigure::getLabelColor() const
{
    return labelFigure->getColor();
}

void ThermometerFigure::setLabelColor(const Color& color)
{
    labelFigure->setColor(color);
}

double ThermometerFigure::getMinValue() const
{
    return min;
}

void ThermometerFigure::setMinValue(double value)
{
    if (min != value) {
        min = value;
        redrawTicks();
        refresh();
    }
}

double ThermometerFigure::getMaxValue() const
{
    return max;
}

void ThermometerFigure::setMaxValue(double value)
{
    if (max != value) {
        max = value;
        redrawTicks();
        refresh();
    }
}

double ThermometerFigure::getTickSize() const
{
    return tickSize;
}

void ThermometerFigure::setTickSize(double value)
{
    if (tickSize != value) {
        tickSize = value;
        redrawTicks();
        refresh();
    }
}

void ThermometerFigure::parse(cProperty *property)
{
    cGroupFigure::parse(property);

    setBounds(parseBounds(property, getBounds()));

    // Set default
    redrawTicks();

    const char *s;
    if ((s = property->getValue(PKEY_MERCURY_COLOR)) != nullptr)
        setMercuryColor(parseColor(s));
    if ((s = property->getValue(PKEY_LABEL)) != nullptr)
        setLabel(s);
    if ((s = property->getValue(PKEY_LABEL_OFFSET)) != nullptr)
        setLabelOffset(atoi(s));
    if ((s = property->getValue(PKEY_LABEL_FONT)) != nullptr)
        setLabelFont(parseFont(s));
    if ((s = property->getValue(PKEY_LABEL_COLOR)) != nullptr)
        setLabelColor(parseColor(s));
    // This must be initialized before min and max, because it is possible to have too many unnecessary ticks
    if ((s = property->getValue(PKEY_TICK_SIZE)) != nullptr)
        setTickSize(utils::atod(s));
    if ((s = property->getValue(PKEY_MIN_VALUE)) != nullptr)
        setMinValue(utils::atod(s));
    if ((s = property->getValue(PKEY_MAX_VALUE)) != nullptr)
        setMaxValue(utils::atod(s));
    if ((s = property->getValue(PKEY_INITIAL_VALUE)) != nullptr)
        setValue(0, simTime(), utils::atod(s));
}

const char **ThermometerFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {
            PKEY_MERCURY_COLOR, PKEY_LABEL, PKEY_LABEL_FONT,
            PKEY_LABEL_COLOR, PKEY_MIN_VALUE, PKEY_MAX_VALUE, PKEY_TICK_SIZE,
            PKEY_INITIAL_VALUE, PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_BOUNDS, PKEY_LABEL_OFFSET, nullptr
        };
        concatArrays(keys, cGroupFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void ThermometerFigure::addChildren()
{
    mercuryFigure = new cPathFigure("mercury");
    containerFigure = new cPathFigure("container");
    labelFigure = new cTextFigure("label");

    mercuryFigure->setFilled(true);
    mercuryFigure->setFillColor("darkorange");
    mercuryFigure->setLineOpacity(0);

    labelFigure->setAnchor(cFigure::ANCHOR_N);

    addFigure(mercuryFigure);
    addFigure(containerFigure);
    addFigure(labelFigure);
}

void ThermometerFigure::setValue(int series, simtime_t timestamp, double newValue)
{
    ASSERT(series == 0);
    // Note: we currently ignore timestamp
    if (value != newValue) {
        value = newValue;
        refresh();
    }
}

void ThermometerFigure::setTickGeometry(cLineFigure *tick, int index)
{
    if (numTicks - 1 == 0)
        return;

    double x, y, width, height, offset;
    getContainerGeometry(x, y, width, height, offset);

    double lineWidth = getBounds().height * TICK_LINE_WIDTH_PERCENT / 2;
    x += width + lineWidth;
    y += offset + height - height * (index * tickSize + shifting) / (max - min);
    tick->setStart(Point(x, y));
    tick->setEnd(Point(x + getBounds().width * TICK_LENGTH_PERCENT, y));
    tick->setLineWidth(lineWidth);
}

void ThermometerFigure::getContainerGeometry(double& x, double& y, double& width, double& height, double& offset)
{
    x = getBounds().x + getBounds().width * CONTAINER_POS_PERCENT;
    y = getBounds().y + getBounds().width * CONTAINER_POS_PERCENT;
    width = getBounds().width * CONTAINER_WIDTH_PERCENT;
    offset = getBounds().height * CONTAINER_OFFSET_PERCENT;
    height = getBounds().height - width * (1 + 2 * CONTAINER_POS_PERCENT) - getBounds().width * CONTAINER_POS_PERCENT - offset;
}

void ThermometerFigure::setNumberGeometry(cTextFigure *number, int index)
{
    if (numTicks - 1 == 0)
        return;

    double x, y, width, height, offset;
    getContainerGeometry(x, y, width, height, offset);

    double lineWidth = getBounds().height * TICK_LINE_WIDTH_PERCENT;
    x += width + lineWidth + getBounds().width * TICK_LENGTH_PERCENT;
    y += offset + height - height * (index * tickSize + shifting) / (max - min);

    double pointSize = getBounds().height * FONT_SIZE_PERCENT;

    number->setPosition(Point(x + pointSize * NUMBER_DISTANCE_FROM_TICK_PERCENT, y));
    number->setFont(cFigure::Font("", pointSize, 0));
}

void ThermometerFigure::setMercuryAndContainerGeometry()
{
    containerFigure->clearPath();
    mercuryFigure->clearPath();

    containerFigure->setLineWidth(getBounds().width * CONTAINER_LINE_WIDTH_PERCENT);

    double x, y, width, height, offset;
    getContainerGeometry(x, y, width, height, offset);

    containerFigure->addMoveTo(x, y);
    containerFigure->addLineRel(0, height + 2 * offset);
    // TODO this does not work with Qtenv:
//    containerFigure->addCubicBezierCurveRel(0, width, width, width, width, 0);
    containerFigure->addArcRel(width / 2, width / 2, 0, true, false, width, 0);
    containerFigure->addLineRel(0, -height - 2 * offset);
    containerFigure->addArcRel(width / 2, width / 2, 0, true, false, -width, 0);

    double mercuryLevel;
    double overflow = 0;
    if (std::isnan(value))
        return;
    else if (value < min) {
        mercuryFigure->addMoveTo(x, y + 2 * offset + height);
        mercuryFigure->addArcRel(width / 2, width / 2, 0, true, false, width, 0);
        mercuryFigure->addClosePath();
        return;
    }
    else if (value > max) {
        mercuryLevel = 1;
        // value < max so the mercury will be overflow
        overflow = 2 * offset;
        offset = 0;
    }
    else
        mercuryLevel = (value - min) / (max - min);

    mercuryFigure->addMoveTo(x, y + offset + height * (1 - mercuryLevel));
    mercuryFigure->addLineRel(0, height * mercuryLevel + overflow + offset);
    // TODO this does not work with Qtenv:
//    mercuryFigure->addCubicBezierCurveRel(0, width, width, width, width, 0);
    mercuryFigure->addArcRel(width / 2, width / 2, 0, true, false, width, 0);
    mercuryFigure->addLineRel(0, -height * mercuryLevel - overflow - offset);
    if (overflow > 0)
        mercuryFigure->addArcRel(width / 2, width / 2, 0, true, false, -width, 0);
    mercuryFigure->addClosePath();
}

void ThermometerFigure::redrawTicks()
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
    if ((size_t)numTicks > tickFigures.size()) {
        while ((size_t)numTicks > tickFigures.size()) {
            cLineFigure *tick = new cLineFigure();
            cTextFigure *number = new cTextFigure();
            take(tick);
            take(number);

            number->setAnchor(cFigure::ANCHOR_W);

            tickFigures.push_back(tick);
            numberFigures.push_back(number);
        }
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
        addFigure(tickFigures[i]);
        addFigure(numberFigures[i]);
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

void ThermometerFigure::layout()
{
    setMercuryAndContainerGeometry();

    for (int i = 0; i < numTicks; ++i) {
        setTickGeometry(tickFigures[i], i);
        setNumberGeometry(numberFigures[i], i);
    }

    labelFigure->setPosition(Point(getBounds().getCenter().x, getBounds().y + getBounds().height + labelOffset));
}

void ThermometerFigure::refresh()
{
    setMercuryAndContainerGeometry();
}

} // namespace inet

