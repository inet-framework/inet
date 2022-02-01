//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/figures/GaugeFigure.h"

#include "inet/common/INETUtils.h"

namespace inet {

Register_Figure("gauge", GaugeFigure);

static const double BORDER_WIDTH_PERCENT = 0.015;
static const double CURVE_WIDTH_PERCENT = 0.02;
static const double TICK_BIG_LENGTH_PERCENT = 0.44;
static const double TICK_SMALL_LENGTH_PERCENT = 0.46;
static const double FONT_SIZE_PERCENT = 0.07;
static const double NEEDLE_WIDTH_PERCENT = 0.12;
static const double NEEDLE_HEIGHT_PERCENT = 0.4;
static const double VALUE_Y_PERCENT = 0.9;
static const double TICK_LINE_WIDTH_PERCENT = 0.008;
static const double START_ANGLE = -M_PI / 2;
static const double END_ANGLE = M_PI;
static const double NEEDLE_OFFSET_IN_DEGREE = 12;

static const char *PKEY_BACKGROUND_COLOR = "backgroundColor";
static const char *PKEY_NEEDLE_COLOR = "needleColor";
static const char *PKEY_LABEL = "label";
static const char *PKEY_LABEL_FONT = "labelFont";
static const char *PKEY_LABEL_COLOR = "labelColor";
static const char *PKEY_MIN_VALUE = "minValue";
static const char *PKEY_MAX_VALUE = "maxValue";
static const char *PKEY_TICK_SIZE = "tickSize";
static const char *PKEY_COLOR_STRIP = "colorStrip";
static const char *PKEY_INITIAL_VALUE = "initialValue";
static const char *PKEY_POS = "pos";
static const char *PKEY_SIZE = "size";
static const char *PKEY_ANCHOR = "anchor";
static const char *PKEY_BOUNDS = "bounds";
static const char *PKEY_LABEL_OFFSET = "labelOffset";

inline double zeroToOne(double x) { return x == 0 ? 1 : x; }

GaugeFigure::GaugeFigure(const char *name) : cGroupFigure(name)
{
    addChildren();
}

GaugeFigure::~GaugeFigure()
{
    // delete figures which is not in canvas
    for (size_t i = curvesOnCanvas; i < curveFigures.size(); ++i)
        dropAndDelete(curveFigures[i]);

    for (size_t i = numTicks; i < tickFigures.size(); ++i) {
        dropAndDelete(tickFigures[i]);
        dropAndDelete(numberFigures[i]);
    }
}

const cFigure::Rectangle& GaugeFigure::getBounds() const
{
    return backgroundFigure->getBounds();
}

void GaugeFigure::setBounds(const Rectangle& rect)
{
    backgroundFigure->setBounds(rect);
    layout();
}

const cFigure::Color& GaugeFigure::getBackgroundColor() const
{
    return backgroundFigure->getFillColor();
}

void GaugeFigure::setBackgroundColor(const Color& color)
{
    backgroundFigure->setFillColor(color);
}

const cFigure::Color& GaugeFigure::getNeedleColor() const
{
    return needle->getFillColor();
}

void GaugeFigure::setNeedleColor(const Color& color)
{
    needle->setFillColor(color);
}

const char *GaugeFigure::getLabel() const
{
    return labelFigure->getText();
}

void GaugeFigure::setLabel(const char *text)
{
    labelFigure->setText(text);
}

int GaugeFigure::getLabelOffset() const
{
    return labelOffset;
}

void GaugeFigure::setLabelOffset(int offset)
{
    if (labelOffset != offset) {
        labelOffset = offset;
        labelFigure->setPosition(Point(getBounds().getCenter().x, getBounds().y + getBounds().height + labelOffset));
    }
}

const cFigure::Font& GaugeFigure::getLabelFont() const
{
    return labelFigure->getFont();
}

void GaugeFigure::setLabelFont(const Font& font)
{
    labelFigure->setFont(font);
}

const cFigure::Color& GaugeFigure::getLabelColor() const
{
    return labelFigure->getColor();
}

void GaugeFigure::setLabelColor(const Color& color)
{
    labelFigure->setColor(color);
}

double GaugeFigure::getMinValue() const
{
    return min;
}

void GaugeFigure::setMinValue(double value)
{
    if (min != value) {
        min = value;
        redrawTicks();
        refresh();
    }
}

double GaugeFigure::getMaxValue() const
{
    return max;
}

void GaugeFigure::setMaxValue(double value)
{
    if (max != value) {
        max = value;
        redrawTicks();
        refresh();
    }
}

double GaugeFigure::getTickSize() const
{
    return tickSize;
}

void GaugeFigure::setTickSize(double value)
{
    if (tickSize != value) {
        tickSize = value;
        redrawTicks();
        refresh();
    }
}

const char *GaugeFigure::getColorStrip() const
{
    return colorStrip;
}

void GaugeFigure::setColorStrip(const char *colorStrip)
{
    if (strcmp(this->colorStrip, colorStrip) != 0) {
        this->colorStrip = colorStrip;
        redrawCurves();
    }
}

void GaugeFigure::parse(cProperty *property)
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
        setTickSize(atof(s));
    if ((s = property->getValue(PKEY_MIN_VALUE)) != nullptr)
        setMinValue(atof(s));
    if ((s = property->getValue(PKEY_MAX_VALUE)) != nullptr)
        setMaxValue(atof(s));
    if ((s = property->getValue(PKEY_COLOR_STRIP)) != nullptr)
        setColorStrip(s);
    if ((s = property->getValue(PKEY_INITIAL_VALUE)) != nullptr)
        setValue(0, simTime(), utils::atod(s));

}

const char **GaugeFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {
            PKEY_BACKGROUND_COLOR, PKEY_NEEDLE_COLOR, PKEY_LABEL, PKEY_LABEL_FONT,
            PKEY_LABEL_COLOR, PKEY_MIN_VALUE, PKEY_MAX_VALUE, PKEY_TICK_SIZE,
            PKEY_COLOR_STRIP, PKEY_INITIAL_VALUE, PKEY_POS, PKEY_SIZE, PKEY_ANCHOR,
            PKEY_BOUNDS, PKEY_LABEL_OFFSET, nullptr
        };
        concatArrays(keys, cGroupFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void GaugeFigure::addChildren()
{
    backgroundFigure = new cOvalFigure("background");
    needle = new cPathFigure("needle");
    valueFigure = new cTextFigure("value");
    labelFigure = new cTextFigure("label");

    backgroundFigure->setFilled(true);
    backgroundFigure->setFillColor(Color("#b8afa6"));

    needle->setFilled(true);
    needle->setFillColor(cFigure::Color("#dba672"));

    valueFigure->setAnchor(cFigure::ANCHOR_CENTER);
    labelFigure->setAnchor(cFigure::ANCHOR_N);

    addFigure(backgroundFigure);
    addFigure(needle);
    addFigure(valueFigure);
    addFigure(labelFigure);
}

void GaugeFigure::setColorCurve(const cFigure::Color& curveColor, double startAngle, double endAngle, cArcFigure *arc)
{
    arc->setLineColor(curveColor);
    arc->setStartAngle(startAngle);
    arc->setEndAngle(endAngle);
    setCurveGeometry(arc);
}

void GaugeFigure::setValue(int series, simtime_t timestamp, double newValue)
{
    ASSERT(series == 0);
    // Note: we currently ignore timestamp
    if (value != newValue) {
        value = newValue;
        refresh();
    }
}

void GaugeFigure::setCurveGeometry(cArcFigure *curve)
{
    double lineWidth = getBounds().width * CURVE_WIDTH_PERCENT;
    Rectangle arcBounds = getBounds();
    double offset = lineWidth + backgroundFigure->getLineWidth();

    arcBounds.height -= offset;
    arcBounds.width -= offset;
    arcBounds.x += offset / 2;
    arcBounds.y += offset / 2;

    curve->setBounds(arcBounds);
    curve->setLineWidth(lineWidth);

    Transform trans;
    trans.rotate(-M_PI / 4, getBounds().getCenter());
    curve->setTransform(trans);
}

void GaugeFigure::setTickGeometry(cLineFigure *tick, int index)
{
    tick->setStart(Point(getBounds().x, getBounds().getCenter().y));

    Point endPos = getBounds().getCenter();
    endPos.x -= !(index % 3) ? getBounds().width * TICK_BIG_LENGTH_PERCENT :
        getBounds().width * TICK_SMALL_LENGTH_PERCENT;
    tick->setEnd(endPos);
    tick->setLineWidth(zeroToOne(getBounds().width * TICK_LINE_WIDTH_PERCENT));

    Transform trans;
    trans.rotate((M_PI + M_PI / 2) * (index * tickSize + shifting) / (max - min), getBounds().getCenter()).
            rotate(-M_PI / 4, getBounds().getCenter());
    tick->setTransform(trans);
}

void GaugeFigure::setNumberGeometry(cTextFigure *number, int index)
{
    ASSERT(tickFigures.size() > 0);

    double distanceToBorder = tickFigures[0]->getStart().distanceTo(tickFigures[0]->getEnd());
    number->setFont(cFigure::Font("", getBounds().width * FONT_SIZE_PERCENT, 0));
    Point textPos = Point(getBounds().x + distanceToBorder + number->getFont().pointSize, getBounds().getCenter().y);
    number->setPosition(textPos);

    Transform trans;
    trans.rotate(-(M_PI + M_PI / 2) * (index * tickSize + shifting) / (max - min), textPos).
            rotate(M_PI / 4, textPos).
            rotate((M_PI + M_PI / 2) * (index * tickSize + shifting) / (max - min), getBounds().getCenter()).
            rotate(-M_PI / 4, getBounds().getCenter());
    number->setTransform(trans);
}

void GaugeFigure::setNeedleGeometry()
{
    double cx = getBounds().getCenter().x;
    double cy = getBounds().getCenter().y;
    double w = getBounds().width;

    // draw needle in horizontal position, pointing 9:00 hours
    double needleHalfWidth = w * NEEDLE_WIDTH_PERCENT / 2;
    double needleLength = w * NEEDLE_HEIGHT_PERCENT;
    needle->clearPath();
    needle->addMoveTo(cx + needleHalfWidth, cy);
    needle->addLineTo(cx, cy - needleHalfWidth);
    needle->addLineTo(cx - needleLength, cy);
    needle->addLineTo(cx, cy + needleHalfWidth);
    needle->addClosePath();

    setNeedleTransform();
}

void GaugeFigure::setNeedleTransform()
{
    double angle;
    const double offset = NEEDLE_OFFSET_IN_DEGREE * M_PI / 180;
    needle->setVisible(true);
    if (value < min)
        angle = -offset;
    else if (value > max)
        angle = (END_ANGLE - START_ANGLE) + offset;
    else if (std::isnan(value)) {
        needle->setVisible(false);
        return;
    }
    else
        angle = (value - min) / (max - min) * (END_ANGLE - START_ANGLE);

    cFigure::Transform t;
    t.rotate(angle, getBounds().getCenter()).rotate(-M_PI / 4, getBounds().getCenter());
    needle->setTransform(t);
}

void GaugeFigure::redrawTicks()
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

            number->setAnchor(cFigure::ANCHOR_CENTER);

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
        addFigure(tickFigures[i]);
        numberFigures[i]->insertBelow(needle);
    }

    for (int i = 0; i < numTicks; ++i) {
        setTickGeometry(tickFigures[i], i);

        char buf[32];

        double number = min + i * tickSize + shifting;
        if (std::abs(number) < tickSize / 2)
            number = 0;

        sprintf(buf, "%g", number);
        numberFigures[i]->setText(buf);
        setNumberGeometry(numberFigures[i], i);
    }
}

void GaugeFigure::redrawCurves()
{
    // create color strips
    cStringTokenizer signalTokenizer(colorStrip, " ,");

    double lastStop = 0.0;
    double newStop = 0.0;
    Color color;
    size_t index = 0;
    const double deg270InRad = 6 * M_PI / 4;
    while (signalTokenizer.hasMoreTokens()) {
        const char *token = signalTokenizer.nextToken();
        newStop = atof(token);
        if (newStop > lastStop) {
            if (index == curveFigures.size()) {
                cArcFigure *arc = new cArcFigure("colorStrip");
                take(arc);
                arc->setZoomLineWidth(true);
                curveFigures.push_back(arc);
            }

            setColorCurve(color, M_PI - (deg270InRad * newStop), M_PI - (deg270InRad * lastStop), curveFigures[index]);

            ++index;
            lastStop = newStop;
        }
        if (newStop == 0.0)
            color = Color(token);
    }
    if (lastStop < 1.0) {
        if (index == curveFigures.size()) {
            cArcFigure *arc = new cArcFigure("colorStrip");
            take(arc);
            arc->setZoomLineWidth(true);
            curveFigures.push_back(arc);
        }
        setColorCurve(color, M_PI - deg270InRad, M_PI - (deg270InRad * lastStop), curveFigures[index]);
        ++index;
    }

    int prevCurvesOnCanvas = curvesOnCanvas;
    curvesOnCanvas = index;

    // Add or remove figures from canvas according to previous number of curves
    for (int i = prevCurvesOnCanvas; i < curvesOnCanvas; ++i) {
        drop(curveFigures[i]);
        curveFigures[i]->insertBelow(needle);
    }
    for (int i = curvesOnCanvas; i < prevCurvesOnCanvas; ++i) {
        removeFigure(curveFigures[index]);
        take(curveFigures[index]);
    }
}

void GaugeFigure::layout()
{
    backgroundFigure->setLineWidth(zeroToOne(getBounds().width * BORDER_WIDTH_PERCENT));

    for (cArcFigure *item : curveFigures)
        setCurveGeometry(item);

    for (int i = 0; i < numTicks; ++i) {
        setTickGeometry(tickFigures[i], i);
        setNumberGeometry(numberFigures[i], i);
    }

    setNeedleGeometry();

    valueFigure->setFont(Font("", getBounds().width * FONT_SIZE_PERCENT, 0));
    valueFigure->setPosition(Point(getBounds().getCenter().x, getBounds().y + getBounds().height * VALUE_Y_PERCENT));

    labelFigure->setPosition(Point(getBounds().getCenter().x, getBounds().y + getBounds().height + labelOffset));
}

void GaugeFigure::refresh()
{
    setNeedleTransform();

    // update displayed number
    if (std::isnan(value))
        valueFigure->setText("");
    else {
        char buf[32];
        sprintf(buf, "%g", value);
        valueFigure->setText(buf);
    }
}

} // namespace inet

