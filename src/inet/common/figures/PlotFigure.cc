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

#include "inet/common/figures/InstrumentUtil.h"
#include "inet/common/figures/PlotFigure.h"

namespace inet {

Register_Figure("plot", PlotFigure);

static const char *INIT_PLOT_COLOR = "blue";
static const char *INIT_BACKGROUND_COLOR = "white";
static const double TICK_LENGTH = 5;
static const double LABEL_Y_DISTANCE_FACTOR = 1.5;

static const char *PKEY_BACKGROUND_COLOR = "backgroundColor";
static const char *PKEY_LABEL = "label";
static const char *PKEY_LABEL_OFFSET = "labelOffset";
static const char *PKEY_LABEL_FONT = "labelFont";
static const char *PKEY_LABEL_COLOR = "labelColor";
static const char *PKEY_TIME_WINDOW = "timeWindow";
static const char *PKEY_X_TICK_SIZE = "xTickSize";
static const char *PKEY_Y_TICK_SIZE = "yTickSize";
static const char *PKEY_LINE_COLOR = "lineColor";
static const char *PKEY_MIN_X = "minX";
static const char *PKEY_MAX_X = "maxX";
static const char *PKEY_MIN_Y = "minY";
static const char *PKEY_MAX_Y = "maxY";
static const char *PKEY_POS = "pos";
static const char *PKEY_SIZE = "size";
static const char *PKEY_ANCHOR = "anchor";
static const char *PKEY_BOUNDS = "bounds";

PlotFigure::PlotFigure(const char *name) : cGroupFigure(name)
{
    addChildren();
    setNumSeries(1);
}

void PlotFigure::setNumSeries(int numSeries)
{
    this->numSeries = numSeries;
    seriesValues.resize(numSeries);
    for (auto plotFigure : seriesPlotFigures) {
        removeFigure(plotFigure);
        delete plotFigure;
    }
    seriesPlotFigures.clear();
    for (int i = 0; i < numSeries; i++) {
        auto plotFigure = new cPathFigure("plot");
        plotFigure->setLineColor(INIT_PLOT_COLOR);
        addFigure(plotFigure);
        seriesPlotFigures.push_back(plotFigure);
    }
}

void PlotFigure::setPlotSize(const Point& p)
{
    const auto& backgroundBounds = backgroundFigure->getBounds();
    backgroundFigure->setBounds(Rectangle(backgroundBounds.x, backgroundBounds.y, p.x, p.y));
    invalidLayout = true;
}

const cFigure::Rectangle& PlotFigure::getBounds() const
{
    if (invalidLayout)
        const_cast<PlotFigure *>(this)->layout();
    return bounds;
}

void PlotFigure::setBounds(const Rectangle& rect)
{
    const auto& backgroundBounds = backgroundFigure->getBounds();
    backgroundFigure->setBounds(Rectangle(backgroundBounds.x + rect.x - bounds.x, backgroundBounds.y + rect.y - bounds.y, rect.width - (bounds.width - backgroundBounds.width), rect.height - (bounds.height - backgroundBounds.height)));
    invalidLayout = true;
}

const cFigure::Color& PlotFigure::getBackgrouncColor() const
{
    return backgroundFigure->getFillColor();
}

void PlotFigure::setBackgroundColor(const Color& color)
{
    backgroundFigure->setFillColor(color);
}

double PlotFigure::getYTickSize() const
{
    return yTickSize;
}

void PlotFigure::setYTickSize(double size)
{
    if (yTickSize == size)
        return;

    yTickSize = size;
    invalidLayout = true;
}

void PlotFigure::setYTickCount(int count)
{
    if (count != 0 && std::isfinite(minY) && std::isfinite(maxY))
        setYTickSize((maxY - minY) / (count - 1));
    else
        setYTickSize(INFINITY);
}

double PlotFigure::getTimeWindow() const
{
    return timeWindow;
}

void PlotFigure::setTimeWindow(double timeWindow)
{
    if (this->timeWindow == timeWindow)
        return;

    this->timeWindow = timeWindow;
}

double PlotFigure::getXTickSize() const
{
    return xTickSize;
}

void PlotFigure::setXTickSize(double size)
{
    if (xTickSize == size)
        return;

    xTickSize = size;
}

void PlotFigure::setXTickCount(int count)
{
    if (count != 0 && std::isfinite(minX) && std::isfinite(maxX))
        setXTickSize((maxX - minX) / (count - 1));
    else
        setXTickSize(INFINITY);
}

const cFigure::Color& PlotFigure::getLineColor(int series) const
{
    return seriesPlotFigures[series]->getLineColor();
}

void PlotFigure::setLineColor(int series, const Color& color)
{
    seriesPlotFigures[series]->setLineColor(color);
}

void PlotFigure::setMinX(double value)
{
    if (minX == value)
        return;

    minX = value;
    invalidLayout = true;
}

void PlotFigure::setMaxX(double value)
{
    if (maxX == value)
        return;

    maxX = value;
    invalidLayout = true;
}

void PlotFigure::setMinY(double value)
{
    if (minY == value)
        return;

    minY = value;
    invalidLayout = true;
}

void PlotFigure::setMaxY(double value)
{
    if (maxY == value)
        return;

    maxY = value;
    invalidLayout = true;
}

int PlotFigure::getLabelOffset() const
{
    return labelOffset;
}

void PlotFigure::setLabelOffset(int offset)
{
    if (labelOffset == offset)
        return;

    labelOffset = offset;
    invalidLayout = true;
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

    setBounds(parseBounds(property, getBounds()));

    if ((s = property->getValue(PKEY_BACKGROUND_COLOR)) != nullptr)
        setBackgroundColor(parseColor(s));
    if ((s = property->getValue(PKEY_X_TICK_SIZE)) != nullptr)
        setXTickSize(atoi(s));
    if ((s = property->getValue(PKEY_Y_TICK_SIZE)) != nullptr)
        setYTickSize(atoi(s));
    if ((s = property->getValue(PKEY_TIME_WINDOW)) != nullptr)
        setTimeWindow(atoi(s));
    if ((s = property->getValue(PKEY_LINE_COLOR)) != nullptr) {
        for (int i = 0; i < numSeries; i++)
            setLineColor(i, parseColor(s));
    }
    if ((s = property->getValue(PKEY_MIN_X)) != nullptr)
        setMinX(atof(s));
    if ((s = property->getValue(PKEY_MAX_X)) != nullptr)
        setMaxX(atof(s));
    if ((s = property->getValue(PKEY_MIN_Y)) != nullptr)
        setMinY(atof(s));
    if ((s = property->getValue(PKEY_MAX_Y)) != nullptr)
        setMaxY(atof(s));
    if ((s = property->getValue(PKEY_LABEL)) != nullptr)
        setLabel(s);
    if ((s = property->getValue(PKEY_LABEL_OFFSET)) != nullptr)
        setLabelOffset(atoi(s));
    if ((s = property->getValue(PKEY_LABEL_COLOR)) != nullptr)
        setLabelColor(parseColor(s));
    if ((s = property->getValue(PKEY_LABEL_FONT)) != nullptr)
        setLabelFont(parseFont(s));
    refreshDisplay();
}

const char **PlotFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {
            PKEY_Y_TICK_SIZE, PKEY_TIME_WINDOW, PKEY_X_TICK_SIZE,
            PKEY_LINE_COLOR, PKEY_MIN_X, PKEY_MAX_X, PKEY_MIN_Y, PKEY_MAX_Y, PKEY_BACKGROUND_COLOR,
            PKEY_LABEL, PKEY_LABEL_OFFSET, PKEY_LABEL_COLOR, PKEY_LABEL_FONT, PKEY_POS,
            PKEY_SIZE, PKEY_ANCHOR, PKEY_BOUNDS, nullptr
        };
        concatArrays(keys, cGroupFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void PlotFigure::addChildren()
{
    labelFigure = new cLabelFigure("label");
    labelFigure->setAnchor(ANCHOR_N);
    xAxisLabelFigure = new cLabelFigure("X axis label");
    xAxisLabelFigure->setAnchor(ANCHOR_S);
    yAxisLabelFigure = new cLabelFigure("Y axis label");
    yAxisLabelFigure->setAnchor(ANCHOR_S);
#if OMNETPP_BUILDNUM > 1500
    yAxisLabelFigure->setAngle(-90);
#endif
    backgroundFigure = new cRectangleFigure("bounds");

    backgroundFigure->setFilled(true);
    backgroundFigure->setFillColor(INIT_BACKGROUND_COLOR);

    addFigure(backgroundFigure);
    addFigure(labelFigure);
    addFigure(xAxisLabelFigure);
    addFigure(yAxisLabelFigure);
}

void PlotFigure::setValue(int series, double x, double y)
{
    seriesValues[series].push_front({x, y});
    invalidPlot = true;
}

static cFigure::Rectangle rectangleUnion(const cFigure::Rectangle& r1, const cFigure::Rectangle& r2)
{
    auto x1 = std::min(r1.x, r2.x);
    auto y1 = std::min(r1.y, r2.y);
    auto x2 = std::max(r1.x + r1.width, r2.x + r2.width);
    auto y2 = std::max(r1.y + r1.height, r2.y + r2.height);
    return cFigure::Rectangle(x1, y1, x2 - x1, y2 - y1);
}

void PlotFigure::layout()
{
    redrawXTicks();
    redrawYTicks();

    Rectangle b = backgroundFigure->getBounds();
    double fontSize = xTicks.size() > 0 && xTicks[0].number ? xTicks[0].number->getFont().pointSize : 12;
    labelFigure->setPosition(Point(b.getCenter().x, b.y + b.height + fontSize * LABEL_Y_DISTANCE_FACTOR + labelOffset));
    xAxisLabelFigure->setPosition(Point(b.x + b.width / 2, b.y - 5));
    yAxisLabelFigure->setPosition(Point(-5, b.height / 2));

    bounds = backgroundFigure->getBounds();
    bounds = rectangleUnion(bounds, labelFigure->getBounds());
    bounds.x -= fontSize;
    bounds.y -= fontSize;
    bounds.width +=  2 * fontSize;
    bounds.height += 2 * fontSize;
    invalidLayout = false;
}

void PlotFigure::redrawYTicks()
{
    Rectangle bounds = backgroundFigure->getBounds();
    int numTicks = std::isfinite(yTickSize) ? std::abs(maxY - minY) / yTickSize + 1 : 0;

    double valueTickYposAdjust[2] = { 0, 0 };
    int fontSize = labelFigure->getFont().pointSize;
    if (yTicks.size() == 1) {
        valueTickYposAdjust[0] = - (fontSize / 2);
        valueTickYposAdjust[1] = fontSize / 2;
    }

    // Allocate ticks and numbers if needed
    if ((size_t)numTicks > yTicks.size())
        while ((size_t)numTicks > yTicks.size()) {
            cLineFigure *tick = new cLineFigure("yTick");
            cLineFigure *dashLine = new cLineFigure("yDashLine");
            cLabelFigure *number = new cLabelFigure("yNumber");

            dashLine->setLineStyle(LINE_DASHED);

            number->setAnchor(ANCHOR_W);
            auto plotFigure = seriesPlotFigures[seriesPlotFigures.size() - 1];
            tick->insertBelow(plotFigure);
            dashLine->insertBelow(plotFigure);
            number->insertBelow(plotFigure);
            yTicks.push_back(Tick(tick, dashLine, number));
        }
    else
        // Add or remove figures from canvas according to previous number of ticks
        for (int i = yTicks.size() - 1; i >= numTicks; --i) {
            delete removeFigure(yTicks[i].number);
            delete removeFigure(yTicks[i].dashLine);
            delete removeFigure(yTicks[i].tick);
            yTicks.pop_back();
        }

    for (size_t i = 0; i < yTicks.size(); ++i) {
        double x = bounds.x + bounds.width;
        double y = bounds.y + bounds.height - bounds.height * (i * yTickSize) / std::abs(maxY - minY);
        if (y > bounds.y && y < bounds.y + bounds.height) {
            yTicks[i].tick->setVisible(true);
            yTicks[i].tick->setStart(Point(x, y));
            yTicks[i].tick->setEnd(Point(x - TICK_LENGTH, y));

            yTicks[i].dashLine->setVisible(true);
            yTicks[i].dashLine->setStart(Point(x - TICK_LENGTH, y));
            yTicks[i].dashLine->setEnd(Point(bounds.x, y));
        }
        else {
            yTicks[i].tick->setVisible(false);
            yTicks[i].dashLine->setVisible(false);
        }

        char buf[32];
        sprintf(buf, yValueFormat, minY + i * yTickSize);
        yTicks[i].number->setText(buf);
        yTicks[i].number->setPosition(Point(x + 5, y + valueTickYposAdjust[i % 2]));
    }
}

void PlotFigure::redrawXTicks()
{
    Rectangle bounds = backgroundFigure->getBounds();
    double minX = std::isnan(timeWindow) ? this->minX : simTime().dbl() - timeWindow;
    double maxX = std::isnan(timeWindow) ? this->maxX : simTime().dbl();

    double shifting = 0;
    if (!std::isnan(timeWindow)) {
        double fraction = std::abs(fmod((minX / xTickSize), 1));
        shifting = xTickSize * (minX < 0 ? fraction : 1 - fraction);
        // if fraction == 0 then shifting == xTickSize therefore don't have to shift the X ticks
        if (shifting == xTickSize)
            shifting = 0;
    }

    int numTicks = std::isfinite(xTickSize) ? ((maxX - minX) - shifting) / xTickSize + 1 : 0;

    // Allocate ticks and numbers if needed
    if ((size_t)numTicks > xTicks.size())
        while ((size_t)numTicks > xTicks.size()) {
            cLineFigure *tick = new cLineFigure("xTick");
            cLineFigure *dashLine = new cLineFigure("xDashLine");
            cLabelFigure *number = new cLabelFigure("xNumber");

            dashLine->setLineStyle(LINE_DASHED);

            number->setAnchor(ANCHOR_N);
            auto plotFigure = seriesPlotFigures[seriesPlotFigures.size() - 1];
            tick->insertBelow(plotFigure);
            dashLine->insertBelow(plotFigure);
            number->insertBelow(plotFigure);
            xTicks.push_back(Tick(tick, dashLine, number));
        }
    else
        // Add or remove figures from canvas according to previous number of ticks
        for (int i = xTicks.size() - 1; i >= numTicks; --i) {
            delete removeFigure(xTicks[i].number);
            delete removeFigure(xTicks[i].dashLine);
            delete removeFigure(xTicks[i].tick);
            xTicks.pop_back();
        }

    for (uint32 i = 0; i < xTicks.size(); ++i) {
        double x = bounds.x + bounds.width * (i * xTickSize + shifting) / (maxX - minX);
        double y = bounds.y + bounds.height;
        if (x > bounds.x && x < bounds.x + bounds.width) {
            xTicks[i].tick->setVisible(true);
            xTicks[i].tick->setStart(Point(x, y));
            xTicks[i].tick->setEnd(Point(x, y - TICK_LENGTH));

            xTicks[i].dashLine->setVisible(true);
            xTicks[i].dashLine->setStart(Point(x, y - TICK_LENGTH));
            xTicks[i].dashLine->setEnd(Point(x, bounds.y));
        }
        else {
            xTicks[i].tick->setVisible(false);
            xTicks[i].dashLine->setVisible(false);
        }

        char buf[32];
        double number = minX + i * xTickSize + shifting;

        sprintf(buf, xValueFormat, number);
        xTicks[i].number->setText(buf);
        xTicks[i].number->setPosition(Point(x, y + 5));
    }
}

void PlotFigure::plot()
{
    double minX = std::isnan(timeWindow) ? this->minX : simTime().dbl() - timeWindow;
    double maxX = std::isnan(timeWindow) ? this->maxX : simTime().dbl();

    for (int i = 0; i < numSeries; i++) {
        auto values = seriesValues[i];
        auto plotFigure = seriesPlotFigures[i];
        plotFigure->clearPath();

        if (values.size() < 2)
            continue;
        if (minX > values.front().first) {
            values.clear();
            continue;
        }

        auto r = backgroundFigure->getBounds();
        auto it = values.begin();
        double startX = r.x + r.width - (maxX - it->first) / (maxX - minX) * r.width;
        double startY = std::min(r.y + 100 * r.height, r.y + (maxY - it->second) / std::abs(maxY - minY) * r.height);
        plotFigure->addMoveTo(startX, startY);

        ++it;
        do {
            double endX = r.x + r.width - (maxX - it->first) / (maxX - minX) * r.width;
            double endY = std::min(r.y + 100 * r.height, r.y + (maxY - it->second) / std::abs(maxY - minY) * r.height);

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
        if (!std::isnan(timeWindow) && it != values.end())
            values.erase(++it, values.end());
    }
}

void PlotFigure::refreshDisplay()
{
    if (invalidLayout)
        layout();
    else if (!std::isnan(timeWindow))
        redrawXTicks();
    if (invalidPlot)
        plot();
}

} // namespace inet

