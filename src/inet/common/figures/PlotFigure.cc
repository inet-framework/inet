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

#include "PlotFigure.h"
#include "InstrumentUtil.h"

// for the moment commented out as omnet cannot instatiate it from a namespace
using namespace inet;
// namespace inet {

Register_Figure("plot", PlotFigure);

#define M_PI    3.14159265358979323846
static const char *INIT_PLOT_COLOR = "blue";
static const char *INIT_BACKGROUND_COLOR = "white";
static const double TICK_LENGTH = 5;
static const double NUMBER_SIZE_PERCENT = 0.1;
static const double NUMBER_DISTANCE_FROM_TICK = 0.04;
static const double LABEL_Y_DISTANCE_FACTOR = 1.5;

static const char *PKEY_BACKGROUND_COLOR = "backgroundColor";
static const char *PKEY_LABEL = "label";
static const char *PKEY_LABEL_OFFSET = "labelOffset";
static const char *PKEY_LABEL_FONT = "labelFont";
static const char *PKEY_LABEL_COLOR = "labelColor";
static const char *PKEY_NUMBER_SIZE_FACTOR = "numberSizeFactor";
static const char *PKEY_VALUE_TICK_SIZE = "valueTickSize";
static const char *PKEY_TIME_WINDOW = "timeWindow";
static const char *PKEY_TIME_TICK_SIZE = "timeTickSize";
static const char *PKEY_LINE_COLOR = "lineColor";
static const char *PKEY_MIN_VALUE = "minValue";
static const char *PKEY_MAX_VALUE = "maxValue";
static const char *PKEY_POS = "pos";
static const char *PKEY_SIZE = "size";
static const char *PKEY_ANCHOR = "anchor";
static const char *PKEY_BOUNDS = "bounds";

PlotFigure::PlotFigure(const char *name) : cGroupFigure(name)
{
    addChildren();
}

const cFigure::Rectangle& PlotFigure::getBounds() const
{
    return backgroundFigure->getBounds();
}

void PlotFigure::setBounds(const Rectangle& rect)
{
    if (backgroundFigure->getBounds() == rect)
        return;

    backgroundFigure->setBounds(rect);
    layout();
}

const cFigure::Color& PlotFigure::getBackgrouncColor() const
{
    return backgroundFigure->getFillColor();
}

void PlotFigure::setBackgroundColor(const Color& color)
{
    backgroundFigure->setFillColor(color);
}

double PlotFigure::getValueTickSize() const
{
    return valueTickSize;
}

void PlotFigure::setValueTickSize(double size)
{
    if (valueTickSize == size)
        return;

    valueTickSize = size;
    layout();
}

simtime_t PlotFigure::getTimeWindow() const
{
    return timeWindow;
}

void PlotFigure::setTimeWindow(simtime_t timeWindow)
{
    if (this->timeWindow == timeWindow)
        return;

    this->timeWindow = timeWindow;
    refresh();
}

simtime_t PlotFigure::getTimeTickSize() const
{
    return timeTickSize;
}

void PlotFigure::setTimeTickSize(simtime_t size)
{
    if (timeTickSize == size)
        return;

    timeTickSize = size;
    refresh();
}

const cFigure::Color& PlotFigure::getLineColor() const
{
    return plotFigure->getLineColor();
}

void PlotFigure::setLineColor(const Color& color)
{
    plotFigure->setLineColor(color);
}

double PlotFigure::getMinValue() const
{
    return min;
}

void PlotFigure::setMinValue(double value)
{
    if (min == value)
        return;

    min = value;
    layout();
}

double PlotFigure::getMaxValue() const
{
    return max;
}

void PlotFigure::setMaxValue(double value)
{
    if (max == value)
        return;

    max = value;
    layout();
}

const char *PlotFigure::getLabel() const
{
    return labelFigure->getText();
}

void PlotFigure::setLabel(const char *text)
{
    labelFigure->setText(text);
}

const int PlotFigure::getLabelOffset() const
{
    return labelOffset;
}

void PlotFigure::setLabelOffset(int offset)
{
    if(labelOffset != offset)   {
        labelOffset = offset;
        layout();
    }
}

const cFigure::Font& PlotFigure::getLabelFont() const
{
    return labelFigure->getFont();
}

void PlotFigure::setLabelFont(const Font& font)
{
    labelFigure->setFont(font);
}

const cFigure::Color& PlotFigure::getLabelColor() const
{
    return labelFigure->getColor();
}

void PlotFigure::setLabelColor(const Color& color)
{
    labelFigure->setColor(color);
}

void PlotFigure::parse(cProperty *property)
{
    cGroupFigure::parse(property);

    const char *s;
    if ((s = property->getValue(PKEY_NUMBER_SIZE_FACTOR)) != nullptr)
            numberSizeFactor = atof(s);

    setBounds(parseBounds(property, getBounds()));

    if ((s = property->getValue(PKEY_BACKGROUND_COLOR)) != nullptr)
        setBackgroundColor(parseColor(s));
    if ((s = property->getValue(PKEY_VALUE_TICK_SIZE)) != nullptr)
        setValueTickSize(atoi(s));
    if ((s = property->getValue(PKEY_TIME_WINDOW)) != nullptr)
        setTimeWindow(atoi(s));
    if ((s = property->getValue(PKEY_LINE_COLOR)) != nullptr)
        setLineColor(parseColor(s));
    if ((s = property->getValue(PKEY_MIN_VALUE)) != nullptr)
        setMinValue(atof(s));
    if ((s = property->getValue(PKEY_MAX_VALUE)) != nullptr)
        setMaxValue(atof(s));
    if ((s = property->getValue(PKEY_LABEL)) != nullptr)
        setLabel(s);
    if ((s = property->getValue(PKEY_LABEL_OFFSET)) != nullptr)
                setLabelOffset(atoi(s));
    if ((s = property->getValue(PKEY_LABEL_COLOR)) != nullptr)
        setLabelColor(parseColor(s));
    if ((s = property->getValue(PKEY_LABEL_FONT)) != nullptr)
        setLabelFont(parseFont(s));
    if ((s = property->getValue(PKEY_TIME_TICK_SIZE)) != nullptr)
        setTimeTickSize(atoi(s));
    else
        refresh();
}

const char **PlotFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {
            PKEY_VALUE_TICK_SIZE, PKEY_TIME_WINDOW, PKEY_TIME_TICK_SIZE,
            PKEY_LINE_COLOR, PKEY_MIN_VALUE, PKEY_MAX_VALUE, PKEY_BACKGROUND_COLOR,
            PKEY_LABEL, PKEY_LABEL_OFFSET, PKEY_LABEL_COLOR, PKEY_LABEL_FONT,
            PKEY_NUMBER_SIZE_FACTOR, PKEY_POS,
            PKEY_SIZE, PKEY_ANCHOR, PKEY_BOUNDS, nullptr
        };
        concatArrays(keys, cGroupFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void PlotFigure::addChildren()
{
    plotFigure = new cPathFigure("plot");
    labelFigure = new cTextFigure("label");
    backgroundFigure = new cRectangleFigure("bounds");

    backgroundFigure->setFilled(true);
    backgroundFigure->setFillColor(INIT_BACKGROUND_COLOR);
    plotFigure->setLineColor(INIT_PLOT_COLOR);
    labelFigure->setAnchor(ANCHOR_N);

    addFigure(backgroundFigure);
    addFigure(plotFigure);
    addFigure(labelFigure);
}

void PlotFigure::setValue(int series, simtime_t timestamp, double value)
{
    ASSERT(series == 0);

    values.push_front(std::pair<simtime_t, double>(timestamp, value));
    refresh();
}

void PlotFigure::layout()
{
    redrawValueTicks();

    Rectangle b = getBounds();
    double fontSize = timeTicks.size() > 0 && timeTicks[0].number ? timeTicks[0].number->getFont().pointSize : 12;
    labelFigure->setPosition(Point(b.getCenter().x, b.y + b.height + fontSize * LABEL_Y_DISTANCE_FACTOR + labelOffset));
}

void PlotFigure::redrawValueTicks()
{
    Rectangle bounds = getBounds();
    int numTicks = std::abs(max - min) / valueTickSize + 1;

    int fontSize = bounds.height * NUMBER_SIZE_PERCENT * numberSizeFactor;

    double valueTickYposAdjust[2] = { 0, 0 };

    if(valueTicks.size() == 1)
    {
        valueTickYposAdjust[0] = - (fontSize / 2);
        valueTickYposAdjust[1] = fontSize / 2;
    }

    // Allocate ticks and numbers if needed
    if (numTicks > valueTicks.size())
        while (numTicks > valueTicks.size()) {
            cLineFigure *tick = new cLineFigure("valueTick");
            cLineFigure *dashLine = new cLineFigure("valueDashLine");
            cTextFigure *number = new cTextFigure("valueNumber");

            dashLine->setLineStyle(LINE_DASHED);

            number->setAnchor(ANCHOR_W);
            number->setFont(Font("", bounds.height * NUMBER_SIZE_PERCENT * numberSizeFactor));
            tick->insertBelow(plotFigure);
            dashLine->insertBelow(plotFigure);
            number->insertBelow(plotFigure);
            valueTicks.push_back(Tick(tick, dashLine, number));
        }
    else
        // Add or remove figures from canvas according to previous number of ticks
        for (int i = valueTicks.size() - 1; i >= numTicks; --i) {
            delete removeFigure(valueTicks[i].number);
            delete removeFigure(valueTicks[i].dashLine);
            delete removeFigure(valueTicks[i].tick);
            valueTicks.pop_back();
        }

    for (int i = 0; i < valueTicks.size(); ++i) {
        double x = bounds.x + bounds.width;
        double y = bounds.y + bounds.height - bounds.height * (i * valueTickSize) / std::abs(max - min);
        if (y > bounds.y && y < bounds.y + bounds.height) {
            valueTicks[i].tick->setVisible(true);
            valueTicks[i].tick->setStart(Point(x, y));
            valueTicks[i].tick->setEnd(Point(x - TICK_LENGTH, y));

            valueTicks[i].dashLine->setVisible(true);
            valueTicks[i].dashLine->setStart(Point(x - TICK_LENGTH, y));
            valueTicks[i].dashLine->setEnd(Point(bounds.x, y));
        }
        else {
            valueTicks[i].tick->setVisible(false);
            valueTicks[i].dashLine->setVisible(false);
        }

        char buf[32];
        sprintf(buf, "%g", min + i * valueTickSize);
        valueTicks[i].number->setText(buf);
        valueTicks[i].number->setPosition(Point(x + bounds.height * NUMBER_DISTANCE_FROM_TICK, y + valueTickYposAdjust[i % 2]));
    }
}

void PlotFigure::refreshDisplay()
{
    refresh();
}

void PlotFigure::redrawTimeTicks()
{
    Rectangle bounds = getBounds();
    simtime_t minX = simTime() - timeWindow;

    double fraction = std::abs(fmod((minX / timeTickSize), 1));
    simtime_t shifting = timeTickSize * (minX < 0 ? fraction : 1 - fraction);
    // if fraction == 0 then shifting == timeTickSize therefore don't have to shift the time ticks
    if (shifting == timeTickSize)
        shifting = 0;

    int numTimeTicks = (timeWindow - shifting) / timeTickSize + 1;

    // Allocate ticks and numbers if needed
    if (numTimeTicks > timeTicks.size())
        while (numTimeTicks > timeTicks.size()) {
            cLineFigure *tick = new cLineFigure("timeTick");
            cLineFigure *dashLine = new cLineFigure("timeDashLine");
            cTextFigure *number = new cTextFigure("timeNumber");

            dashLine->setLineStyle(LINE_DASHED);

            number->setAnchor(ANCHOR_N);
            number->setFont(Font("", bounds.height * NUMBER_SIZE_PERCENT * numberSizeFactor));
            tick->insertBelow(plotFigure);
            dashLine->insertBelow(plotFigure);
            number->insertBelow(plotFigure);
            timeTicks.push_back(Tick(tick, dashLine, number));
        }
    else
        // Add or remove figures from canvas according to previous number of ticks
        for (int i = timeTicks.size() - 1; i >= numTimeTicks; --i) {
            delete removeFigure(timeTicks[i].number);
            delete removeFigure(timeTicks[i].dashLine);
            delete removeFigure(timeTicks[i].tick);
            timeTicks.pop_back();
        }

    for (int i = 0; i < timeTicks.size(); ++i) {
        double x = bounds.x + bounds.width * (i * timeTickSize + shifting) / timeWindow;
        double y = bounds.y + bounds.height;
        if (x > bounds.x && x < bounds.x + bounds.width) {
            timeTicks[i].tick->setVisible(true);
            timeTicks[i].tick->setStart(Point(x, y));
            timeTicks[i].tick->setEnd(Point(x, y - TICK_LENGTH));

            timeTicks[i].dashLine->setVisible(true);
            timeTicks[i].dashLine->setStart(Point(x, y - TICK_LENGTH));
            timeTicks[i].dashLine->setEnd(Point(x, bounds.y));
        }
        else {
            timeTicks[i].tick->setVisible(false);
            timeTicks[i].dashLine->setVisible(false);
        }

        char buf[32];
        simtime_t number = minX.dbl() + i * timeTickSize + shifting;

        sprintf(buf, "%g", number.dbl());
        timeTicks[i].number->setText(buf);
        timeTicks[i].number->setPosition(Point(x, y + bounds.height * NUMBER_DISTANCE_FROM_TICK));
    }
}

void PlotFigure::refresh()
{
    // timeTicks + timeNumbers
    redrawTimeTicks();

    // plot
    simtime_t minX = simTime() - timeWindow;
    simtime_t maxX = simTime();

    plotFigure->clearPath();

    if (values.size() < 2)
        return;
    if (minX > values.front().first) {
        values.clear();
        return;
    }

    auto r = getBounds();
    auto it = values.begin();
    double startX = r.x + r.width - (maxX - it->first).dbl() / timeWindow * r.width;
    double startY = r.y + (max - it->second) / std::abs(max - min) * r.height;
    plotFigure->addMoveTo(startX, startY);

    ++it;
    do {
        double endX = r.x + r.width - (maxX - it->first).dbl() / timeWindow * r.width;
        double endY = r.y + (max - it->second) / std::abs(max - min) * r.height;

        double originalStartX = startX;
        double originalStartY = startY;
        double originalEndX = endX;
        double originalEndY = endY;
        if (InstrumentUtil::CohenSutherlandLineClip(startX, startY, endX, endY, r.x, r.x + r.width, r.y, r.y + r.height)) {
            if (originalStartX != startX || originalStartY != startY)
                plotFigure->addMoveTo(startX, startY);

            plotFigure->addLineTo(endX, endY);
        }

        if (minX > it->first)
            break;

        startX = originalEndX;
        startY = originalEndY;
        ++it;
    } while (it != values.end());

    // Delete old elements
    if (it != values.end())
        values.erase(++it, values.end());
}

// } // namespace inet
