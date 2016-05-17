//
// Copyright (C) 2015 OpenSim Ltd
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

// for the moment commented out as omnet cannot instatiate it from a namespace
using namespace inet;
// namespace inet {

Register_Class(GaugeFigure);

#if OMNETPP_VERSION >= 0x500

#define M_PI 3.14159265358979323846

static const double START_ANGLE = -M_PI/2;
static const double END_ANGLE = M_PI;
static const char *LABEL_PROPERTY = "label";
static const char *VALUE_PROPERTY = "value";
static const char *MIN_VALUE_PROPERTY = "minValue";
static const char *MAX_VALUE_PROPERTY = "maxValue";
static const char *TICK_SIZE_PROPERTY = "tickSize";
static const char *COLOR_STRIP_PROPERTY = "colorStrip";

static const char *use_default(const char *value, const char *defValue)
{
    return value != nullptr ? value : defValue;
}

GaugeFigure::GaugeFigure(const char *name) : cOvalFigure(name)
{
}

void GaugeFigure::parse(cProperty *property)
{
    cOvalFigure::parse(property);
    label = use_default(property->getValue(LABEL_PROPERTY, 0), "");
    value = atof(use_default(property->getValue(VALUE_PROPERTY, 0), "0"));
    minValue = atof(use_default(property->getValue(MIN_VALUE_PROPERTY, 0), "0"));
    maxValue = atof(use_default(property->getValue(MAX_VALUE_PROPERTY, 0), "100"));
    tickSize = atof(use_default(property->getValue(TICK_SIZE_PROPERTY, 0), "20"));
    colorStrip = use_default(property->getValue(COLOR_STRIP_PROPERTY, 0), "");
    addChildren();
}

bool GaugeFigure::isAllowedPropertyKey(const char *key) const
{
    if (strcmp(key, LABEL_PROPERTY) == 0 ||
        strcmp(key, MIN_VALUE_PROPERTY) == 0 ||
        strcmp(key, MAX_VALUE_PROPERTY) == 0 ||
        strcmp(key, VALUE_PROPERTY) == 0 ||
        strcmp(key, TICK_SIZE_PROPERTY) == 0 ||
        strcmp(key, COLOR_STRIP_PROPERTY) == 0)
        return true;

    return cOvalFigure::isAllowedPropertyKey(key);
}

void GaugeFigure::addChildren()
{
    cFigure::Rectangle bounds = getBounds();
    cFigure::Color color("#b8afa6");
    setFilled(true);
    setFillColor(color);
    setLineWidth(bounds.width/80);

    // create color strips
    cStringTokenizer signalTokenizer(colorStrip, " ,");

    double lastStop = 0.0;
    double newStop = 0.0;
    while (signalTokenizer.hasMoreTokens()) {
        const char *token = signalTokenizer.nextToken();
        newStop = atof(token);
        if (newStop > lastStop) {
            addColorCurve(color, M_PI - (M_PI * 1.5 * newStop), M_PI - (M_PI * 1.5 * lastStop));
            lastStop = newStop;
        }
        if (newStop == 0.0)
            color = cFigure::Color(token);
    }
    if (lastStop < 1.0)
        addColorCurve(color, M_PI - (M_PI * 1.5), M_PI - (M_PI * 1.5 * lastStop));


    int numTicks = maxValue / tickSize + 1;

    // create ticks
    for(int i = 0; i < numTicks; ++i) {
        // create line
        cLineFigure *line = new cLineFigure();
        line->setStart(cFigure::Point(bounds.x, bounds.getCenter().y));
        cFigure::Point endPos = bounds.getCenter();

        if (!(i % 3))
            endPos.x -= bounds.width / 2.3;
        else
            endPos.x -= bounds.width / 2.15;
        line->setEnd(endPos);
        line->setLineWidth(bounds.width/120);

        line->rotate(i*(M_PI + M_PI/2)/(numTicks-1), bounds.getCenter());

        addFigure(line);

        // create numbers
        cTextFigure *number = new cTextFigure();
        char buf[32];
        sprintf(buf, "%g", minValue + i*tickSize);
        number->setText(buf);
        number->setAnchor(cFigure::ANCHOR_CENTER);
        number->setFont(cFigure::Font("", bounds.width / 15, 0));

        cFigure::Point textPos = cFigure::Point(bounds.x + bounds.width/9, bounds.getCenter().y);
        number->setPosition(textPos);
        number->rotate(-i*(M_PI + M_PI/2)/(numTicks-1), textPos);
        number->rotate(M_PI/4, textPos);
        number->rotate(i*(M_PI + M_PI/2)/(numTicks-1), bounds.getCenter());
        addFigure(number);
    }

    // create needle
    needle = new cPathFigure();
    needle->setFilled(true);
    needle->setFillColor(cFigure::Color("#dba672"));
    double cx = bounds.getCenter().x;
    double cy = bounds.getCenter().y;
    double w = bounds.width;
    needle->addMoveTo(cx, cy);
    needle->addLineTo(cx - (w / 30), cy - (w / 15));
    needle->addLineTo(cx - (w / 2.5), cy);
    needle->addLineTo(cx - (w / 30), cy + (w / 15));
    needle->addClosePath();
    addFigure(needle);

    rotate(-M_PI/4, bounds.getCenter());

    valueFigure = new cTextFigure();
    valueFigure->setAnchor(cFigure::ANCHOR_CENTER);
    valueFigure->setFont(cFigure::Font("", bounds.width / 15, 0));
    cFigure::Point valuePos = cFigure::Point(bounds.getCenter().x, bounds.y + bounds.height * 0.9f);
    valueFigure->setPosition(valuePos);
    valueFigure->rotate(M_PI/4, bounds.getCenter());
    addFigure(valueFigure);

    // show label
    labelFigure = new cTextFigure();
    labelFigure->setAnchor(cFigure::ANCHOR_CENTER);
    labelFigure->setFont(cFigure::Font("", bounds.width / 15, 0));
    cFigure::Point labelPos = cFigure::Point(bounds.getCenter().x, bounds.y + bounds.height * 1.05f);
    labelFigure->setPosition(labelPos);
    labelFigure->rotate(M_PI/4, bounds.getCenter());
    addFigure(labelFigure);

    setLabel(label);
    setValue(0, simTime(), value);
}

void GaugeFigure::addColorCurve(const cFigure::Color& curveColor, double startAngle, double endAngle)
{
    cArcFigure *arc = new cArcFigure();
    double lineWidth = getBounds().width / 40;
    cFigure::Rectangle arcBounds = getBounds();
    double offset = lineWidth + getLineWidth();
    arcBounds.height -= offset;
    arcBounds.width -= offset;
    arcBounds.x += offset/2;
    arcBounds.y += offset/2;

    arc->setBounds(arcBounds);
    arc->setLineColor(curveColor);
    arc->setStartAngle(startAngle);
    arc->setEndAngle(endAngle);
    arc->setLineWidth(lineWidth);
    addFigure(arc);
}

void GaugeFigure::setLabel(const char *newLabel)
{
    label = newLabel;
    labelFigure->setText(label);
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

void GaugeFigure::refresh()
{
    // adjust needle
    double needleValue = std::max(minValue, std::min(maxValue, value));
    double angle = (needleValue - minValue)/(maxValue - minValue)*(END_ANGLE - START_ANGLE);
    cFigure::Transform t;
    t.rotate(angle, getBounds().getCenter());
    needle->setTransform(t);

    // update displayed number
    char buf[32];
    sprintf(buf, "%g", value);
    valueFigure->setText(buf);
}

#endif // omnetpp 5

// } // namespace inet
