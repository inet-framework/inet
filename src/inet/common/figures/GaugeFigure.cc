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
#include "GaugeFigure.h"

//TODO namespace inet { -- for the moment commented out, as OMNeT++ 5.0 cannot instantiate a figure from a namespace
using namespace inet;

Register_Class(GaugeFigure);

#if OMNETPP_VERSION >= 0x500

#define M_PI 3.14159265358979323846

static const double BORDER_WIDTH_PERCENT = 0.015;
static const double CURVE_WIDTH_PERCENT = 0.02;
static const double TICK_BIG_LENGTH_PERCENT = 0.44;
static const double TICK_SMALL_LENGTH_PERCENT = 0.46;
static const double FONT_SIZE_PERCENT = 0.07;
static const double NEEDLE_WIDTH_PERCENT = 0.03;
static const double NEEDLE_HEIGHT_PERCENT = 0.4;
static const double VALUE_Y_PERCENT = 0.9;
static const double TICK_LINE_WIDTH_PERCENT = 0.008;
static const double START_ANGLE = -M_PI/2;
static const double END_ANGLE = M_PI;

static const char *PKEY_BACKGROUND_COLOR = "backgroundColor";
static const char *PKEY_NEEDLE_COLOR = "needleColor";
static const char *PKEY_LABEL = "label";
static const char *PKEY_LABEL_FONT = "labelFont";
static const char *PKEY_LABEL_COLOR = "labelColor";
static const char *PKEY_MIN_VALUE = "min";
static const char *PKEY_MAX_VALUE = "max";
static const char *PKEY_TICK_SIZE = "tickSize";
static const char *PKEY_COLOR_STRIP = "colorStrip";
static const char *PKEY_POS = "pos";
static const char *PKEY_SIZE = "size";
static const char *PKEY_ANCHOR = "anchor";
static const char *PKEY_BOUNDS = "bounds";

inline double zeroToOne(double x) { return x == 0 ? 1 : x;}

GaugeFigure::GaugeFigure(const char *name) : cGroupFigure(name)
{
    addChildren();
}

GaugeFigure::~GaugeFigure()
{
    // delete figures which is not in canvas
    for(int i = curvesOnCanvas; i < curveFigures.size(); ++i)
        delete curveFigures[i];

    for(int i = numTicks; i < tickFigures.size(); ++i)
    {
        delete tickFigures[i];
        delete numberFigures[i];
    }
}

cFigure::Rectangle GaugeFigure::getBounds() const
{
    return backgroundFigure->getBounds();
}

void GaugeFigure::setBounds(Rectangle rect)
{
    backgroundFigure->setBounds(rect);
    layout();
}

cFigure::Color GaugeFigure::getBackgroundColor() const
{
    return backgroundFigure->getFillColor();
}

void GaugeFigure::setBackgroundColor(cFigure::Color color)
{
    backgroundFigure->setFillColor(color);
}

cFigure::Color GaugeFigure::getNeedleColor() const
{
    return needle->getFillColor();
}

void GaugeFigure::setNeedleColor(cFigure::Color color)
{
    needle->setFillColor(color);
}

const char* GaugeFigure::getLabel() const
{
    return labelFigure->getText();
}

void GaugeFigure::setLabel(const char *text)
{
    labelFigure->setText(text);
}

cFigure::Font GaugeFigure::getLabelFont() const
{
    return labelFigure->getFont();
}

void GaugeFigure::setLabelFont(cFigure::Font font)
{
    labelFigure->setFont(font);
}

cFigure::Color GaugeFigure::getLabelColor() const
{
    return labelFigure->getColor();
}

void GaugeFigure::setLabelColor(cFigure::Color color)
{
    labelFigure->setColor(color);
}

double GaugeFigure::getMin() const
{
    return min;
}

void GaugeFigure::setMin(double value)
{
    if(min != value)
    {
        min = value;
        redrawTicks();
        refresh();
    }
}

double GaugeFigure::getMax() const
{
    return max;
}

void GaugeFigure::setMax(double value)
{
    if(max != value)
    {
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
    if(tickSize != value)
    {
        tickSize = value;
        redrawTicks();
        refresh();
    }
}

const char* GaugeFigure::getColorStrip() const
{
    return colorStrip;
}

void GaugeFigure::setColorStrip(const char* colorStrip)
{
    if(strcmp(this->colorStrip, colorStrip) != 0)
    {
        this->colorStrip = colorStrip;
        redrawCurves();
    }
}

void GaugeFigure::parse(cProperty *property)
{
    cGroupFigure::parse(property);
    setBounds(parseBounds(property));

    // Set default
    redrawTicks();

    const char *s;
    if ((s = property->getValue(PKEY_BACKGROUND_COLOR)) != nullptr)
        setBackgroundColor(parseColor(s));
    if ((s = property->getValue(PKEY_NEEDLE_COLOR)) != nullptr)
        setNeedleColor(parseColor(s));
    if ((s = property->getValue(PKEY_LABEL)) != nullptr)
        setLabel(s);
    if ((s = property->getValue(PKEY_LABEL_FONT)) != nullptr)
        setLabelFont(parseFont(s));
    if ((s = property->getValue(PKEY_LABEL_COLOR)) != nullptr)
        setLabelColor(parseColor(s));
    // This must be initialize before min and max because it is possible to be too much unnecessary tick and number
    if ((s = property->getValue(PKEY_TICK_SIZE)) != nullptr)
        setTickSize(atof(s));
    if ((s = property->getValue(PKEY_MIN_VALUE)) != nullptr)
        setMin(atof(s));
    if ((s = property->getValue(PKEY_MAX_VALUE)) != nullptr)
        setMax(atof(s));
    if ((s = property->getValue(PKEY_COLOR_STRIP)) != nullptr)
        setColorStrip(s);
}

const char **GaugeFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {PKEY_BACKGROUND_COLOR, PKEY_NEEDLE_COLOR, PKEY_LABEL, PKEY_LABEL_FONT,
                                   PKEY_LABEL_COLOR, PKEY_MIN_VALUE, PKEY_MAX_VALUE, PKEY_TICK_SIZE,
                                   PKEY_COLOR_STRIP, PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_BOUNDS, nullptr};
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
    arcBounds.x += offset/2;
    arcBounds.y += offset/2;

    curve->setBounds(arcBounds);
    curve->setLineWidth(lineWidth);

    Transform trans;
    trans.rotate(-M_PI/4, getBounds().getCenter());
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
    trans.rotate(index*(M_PI + M_PI/2)/(numTicks-1), getBounds().getCenter()).
            rotate(-M_PI/4, getBounds().getCenter());
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
    trans.rotate(-index*(M_PI + M_PI/2)/(numTicks-1), textPos).
            rotate(M_PI/4, textPos).
            rotate(index*(M_PI + M_PI/2)/(numTicks-1), getBounds().getCenter()).
            rotate(-M_PI/4, getBounds().getCenter());
    number->setTransform(trans);
}

void GaugeFigure::setNeedleGeometry()
{
    double cx = getBounds().getCenter().x;
    double cy = getBounds().getCenter().y;
    double w = getBounds().width;
    needle->clearPath();

    needle->addMoveTo(cx, cy);
    needle->addLineTo(cx - (w * NEEDLE_WIDTH_PERCENT), cy - (w * 2*NEEDLE_WIDTH_PERCENT));
    needle->addLineTo(cx - (w * NEEDLE_HEIGHT_PERCENT), cy);
    needle->addLineTo(cx - (w * NEEDLE_WIDTH_PERCENT), cy + (w * 2*NEEDLE_WIDTH_PERCENT));
    needle->addClosePath();

    setNeedleTransform();
}

void GaugeFigure::setNeedleTransform()
{
    double angle;
    const double fiveDegreeInRad = 0.0872664626;
    if(std::isnan(value) || value < min)
        angle = -fiveDegreeInRad;
    else if(value > max)
        angle = (END_ANGLE - START_ANGLE) + fiveDegreeInRad;
    else
        angle = (value - min)/(max - min)*(END_ANGLE - START_ANGLE);

    cFigure::Transform t;
    t.rotate(angle, getBounds().getCenter()).rotate(-M_PI/4, getBounds().getCenter());
    needle->setTransform(t);
}

void GaugeFigure::redrawTicks()
{
    ASSERT(tickFigures.size() == numberFigures.size());

    int prevNumTicks = numTicks;
    numTicks = std::max(0.0, std::abs(max - min) / tickSize + 1);

    // Allocate ticks and numbers if needed
    if(numTicks > tickFigures.size())
        while(numTicks > tickFigures.size())
        {
            cLineFigure *tick = new cLineFigure();
            cTextFigure *number = new cTextFigure();

            number->setAnchor(cFigure::ANCHOR_CENTER);

            tickFigures.push_back(tick);
            numberFigures.push_back(number);
        }

    // Add or remove figures from canvas according to previous number of ticks
    for(int i = numTicks; i < prevNumTicks; ++i)
    {
        removeFigure(tickFigures[i]);
        removeFigure(numberFigures[i]);
    }
    for(int i = prevNumTicks; i < numTicks; ++i)
    {
        addFigure(tickFigures[i]);
        addFigure(numberFigures[i]);
    }

    for(int i = 0; i < numTicks; ++i)
    {
        setTickGeometry(tickFigures[i], i);

        char buf[32];
        sprintf(buf, "%g", min + i*tickSize);
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
    int index = 0;
    const double deg270InRad = 6*M_PI/4;
    while (signalTokenizer.hasMoreTokens())
    {
        const char *token = signalTokenizer.nextToken();
        newStop = atof(token);
        if (newStop > lastStop)
        {
            if(index == curveFigures.size())
                curveFigures.push_back(new cArcFigure());

            setColorCurve(color, M_PI - (deg270InRad * newStop), M_PI - (deg270InRad * lastStop), curveFigures[index]);

            ++index;
            lastStop = newStop;
        }
        if (newStop == 0.0)
            color = Color(token);
    }
    if (lastStop < 1.0)
    {
        if(index == curveFigures.size())
            curveFigures.push_back(new cArcFigure());
        setColorCurve(color, M_PI - deg270InRad, M_PI - (deg270InRad * lastStop), curveFigures[index]);
        ++index;
    }

    int prevCurvesOnCanvas = curvesOnCanvas;
    curvesOnCanvas = index;

    // Add or remove figures from canvas according to previous number of curves
    for(int i = prevCurvesOnCanvas; i < curvesOnCanvas; ++i)
        addFigureBelow(curveFigures[i], needle);
    for(int i = curvesOnCanvas; i < prevCurvesOnCanvas; ++i)
        removeFigure(curveFigures[index]);
}

void GaugeFigure::layout()
{
    backgroundFigure->setLineWidth(zeroToOne(getBounds().width * BORDER_WIDTH_PERCENT));

    for(cArcFigure *item : curveFigures)
        setCurveGeometry(item);

    for(int i = 0; i < numTicks; ++i)
    {
        setTickGeometry(tickFigures[i], i);
        setNumberGeometry(numberFigures[i], i);
    }

    setNeedleGeometry();

    valueFigure->setFont(Font("", getBounds().width * FONT_SIZE_PERCENT, 0));
    valueFigure->setPosition(Point(getBounds().getCenter().x, getBounds().y + getBounds().height * VALUE_Y_PERCENT));

    labelFigure->setPosition(Point(getBounds().getCenter().x, getBounds().y + getBounds().height));
}

void GaugeFigure::refresh()
{
    setNeedleTransform();

    // update displayed number
    if (std::isnan(value))
        valueFigure->setText("");
    else
    {
        char buf[32];
        sprintf(buf, "%g", value);
        valueFigure->setText(buf);
    }
}

#endif // omnetpp 5

// } // namespace inet
