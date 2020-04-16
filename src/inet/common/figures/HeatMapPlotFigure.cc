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

#include "inet/common/figures/HeatMapPlotFigure.h"

namespace inet {

static const char *INIT_BACKGROUND_COLOR = "white";
static const double TICK_LENGTH = 5;
static const double LABEL_Y_DISTANCE_FACTOR = 1.5;

static const char *PKEY_LABEL = "label";
static const char *PKEY_LABEL_OFFSET = "labelOffset";
static const char *PKEY_LABEL_FONT = "labelFont";
static const char *PKEY_LABEL_COLOR = "labelColor";
static const char *PKEY_X_TICK_SIZE = "xTickSize";
static const char *PKEY_Y_TICK_SIZE = "yTickSize";
static const char *PKEY_MIN_X = "minX";
static const char *PKEY_MAX_X = "maxX";
static const char *PKEY_MIN_Y = "minY";
static const char *PKEY_MAX_Y = "maxY";
static const char *PKEY_POS = "pos";
static const char *PKEY_SIZE = "size";
static const char *PKEY_ANCHOR = "anchor";
static const char *PKEY_BOUNDS = "bounds";

HeatMapPlotFigure::HeatMapPlotFigure(const char *name) : cGroupFigure(name)
{
    addChildren();
}

cFigure::Color HeatMapPlotFigure::getHeatColor(int x, int y, double c, int channel)
{
    Color oldColor = pixmapFigure->getPixelColor(x, y);
    switch (channel) {
        case 0: return Color(255 * c, oldColor.green, oldColor.blue); break;
        case 1: return Color(oldColor.red, 255 * c, oldColor.blue); break;
        case 2: return Color(oldColor.red, oldColor.green, 255 * c); break;
        default: throw cRuntimeError("Unknown channel");
    }
}

void HeatMapPlotFigure::setPixelSafe(int x, int y, double c, int channel)
{
    if (0 <= x && x < pixmapFigure->getPixmapWidth() && 0 <= y && y < pixmapFigure->getPixmapHeight())
        pixmapFigure->setPixel(x, y, getHeatColor(x, y, std::max(0.0, std::min(1.0, c)), channel));
}

void HeatMapPlotFigure::clearValues()
{
    pixmapFigure->fillPixmap(cFigure::BLACK, 0);
}

void HeatMapPlotFigure::setValue(int x, int y, double v, int channel)
{
    double c = (v - minValue) / (maxValue - minValue);
    setPixelSafe(x, y, c, channel);
}

void HeatMapPlotFigure::setValue(double x, double y, double v, int channel)
{
//    ASSERT(minX <= x && x <= maxX);
//    ASSERT(minY <= y && y <= maxY);
//    ASSERT(minValue <= v && v <= maxValue);
    int xp = std::floor(pixmapFigure->getPixmapWidth() * (x - minX) / (maxX - minX));
    int yp = std::floor(pixmapFigure->getPixmapHeight() * (y - minY) / (maxY - minY));
    double c = (v - minValue) / (maxValue - minValue);
    setPixelSafe(xp, yp, c, channel);
}

void HeatMapPlotFigure::setConstantValue(double x1, double x2, double y1, double y2, double v, int channel)
{
//    ASSERT(minX <= x1 && x1 <= maxX);
//    ASSERT(minX <= x2 && x2 <= maxX);
//    ASSERT(minY <= y1 && y1 <= maxY);
//    ASSERT(minY <= y2 && y2 <= maxY);
//    ASSERT(minValue <= v && v <= maxValue);
    int x1p = std::floor(pixmapFigure->getPixmapWidth() * (x1 - minX) / (maxX - minX));
    int x2p = std::floor(pixmapFigure->getPixmapWidth() * (x2 - minX) / (maxX - minX));
    int y1p = std::floor(pixmapFigure->getPixmapHeight() * (y1 - minY) / (maxY - minY));
    int y2p = std::floor(pixmapFigure->getPixmapHeight() * (y2 - minY) / (maxY - minY));
    double c = (v - minValue) / (maxValue - minValue);
    for (int xp = x1p; xp < x2p; xp++)
        for (int yp = y1p; yp < y2p; yp++)
            setPixelSafe(xp, yp, c, channel);
}

void HeatMapPlotFigure::setLinearValue(double x1, double x2, double y1, double y2, double vl, double vu, int axis, int channel)
{
//    ASSERT(minX <= x1 && x1 <= maxX);
//    ASSERT(minX <= x2 && x2 <= maxX);
//    ASSERT(minY <= y1 && y1 <= maxY);
//    ASSERT(minY <= y2 && y2 <= maxY);
//    ASSERT(minValue <= vl && vl <= maxValue);
//    ASSERT(minValue <= vu && vu <= maxValue);
    int x1p = std::floor(pixmapFigure->getPixmapWidth() * (x1 - minX) / (maxX - minX));
    int x2p = std::floor(pixmapFigure->getPixmapWidth() * (x2 - minX) / (maxX - minX));
    int y1p = std::floor(pixmapFigure->getPixmapHeight() * (y1 - minY) / (maxY - minY));
    int y2p = std::floor(pixmapFigure->getPixmapHeight() * (y2 - minY) / (maxY - minY));
    double v;
    for (int xp = x1p; xp < x2p; xp++) {
        if (axis == 0) {
            double alpha = (double)(xp - x1p) / (x2p - x1p);
            v = vl * (1 - alpha) + vu * alpha;
        }
        for (int yp = y1p; yp < y2p; yp++) {
            if (axis == 1) {
                double alpha = (double)(yp - y1p) / (y2p - y1p);
                v = vl * (1 - alpha) + vu * alpha;
            }
            double c = (v - minValue) / (maxValue - minValue);
            setPixelSafe(xp, yp, c, channel);
        }
    }
}

void HeatMapPlotFigure::setBilinearValue(double x1, double x2, double y1, double y2, double v11, double v21, double v12, double v22, int channel)
{
//    ASSERT(minX <= x1 && x1 <= maxX);
//    ASSERT(minX <= x2 && x2 <= maxX);
//    ASSERT(minY <= y1 && y1 <= maxY);
//    ASSERT(minY <= y2 && y2 <= maxY);
//    ASSERT(minValue <= v11 && v11 <= maxValue);
//    ASSERT(minValue <= v21 && v21 <= maxValue);
//    ASSERT(minValue <= v12 && v12 <= maxValue);
//    ASSERT(minValue <= v22 && v22 <= maxValue);
    int x1p = std::floor(pixmapFigure->getPixmapWidth() * (x1 - minX) / (maxX - minX));
    int x2p = std::floor(pixmapFigure->getPixmapWidth() * (x2 - minX) / (maxX - minX));
    int y1p = std::floor(pixmapFigure->getPixmapHeight() * (y1 - minY) / (maxY - minY));
    int y2p = std::floor(pixmapFigure->getPixmapHeight() * (y2 - minY) / (maxY - minY));
    for (int xp = x1p; xp < x2p; xp++) {
        double ax = (double)(xp - x1p) / (x2p - x1p);
        for (int yp = y1p; yp < y2p; yp++) {
            double ay = (double)(yp - y1p) / (y2p - y1p);
            double vx1 = v11 * (1 - ax) + v21 * ax;
            double vx2 = v12 * (1 - ax) + v22 * ax;
            double vxy = vx1 * (1 - ay) + vx2 * ay;
            double c = (vxy - minValue) / (maxValue - minValue);
            setPixelSafe(xp, yp, c, channel);
        }
    }
}

void HeatMapPlotFigure::bakeValues()
{
    for (int x = 0; x < pixmapFigure->getPixmapWidth(); x++) {
        for (int y = 0; y < pixmapFigure->getPixmapHeight(); y++) {
            auto color = pixmapFigure->getPixelColor(x, y);
            auto c = std::max(std::max(color.red, color.green), color.blue) / 255.0;
            pixmapFigure->setPixelColor(x, y, Color(255 * (1 - c) + c * color.red, 255 * (1 - c) + c * color.green, 255 * (1 - c) + c * color.blue));
        }
    }
}

void HeatMapPlotFigure::setPlotSize(const Point& figureSize, const Point& pixmapSize)
{
    const auto& backgroundBounds = backgroundFigure->getBounds();
    backgroundFigure->setBounds(Rectangle(backgroundBounds.x, backgroundBounds.y, figureSize.x, figureSize.y));
    pixmapFigure->setSize(figureSize.x, figureSize.y);
    pixmapFigure->setPixmapSize(pixmapSize.x, pixmapSize.y, INIT_BACKGROUND_COLOR, 1);
    invalidLayout = true;
}

const cFigure::Rectangle& HeatMapPlotFigure::getBounds() const
{
    if (invalidLayout)
        const_cast<HeatMapPlotFigure *>(this)->layout();
    return bounds;
}

void HeatMapPlotFigure::setBounds(const Rectangle& rect)
{
    const auto& backgroundBounds = backgroundFigure->getBounds();
    backgroundFigure->setBounds(Rectangle(backgroundBounds.x + rect.x - bounds.x, backgroundBounds.y + rect.y - bounds.y, rect.width - (bounds.width - backgroundBounds.width), rect.height - (bounds.height - backgroundBounds.height)));
    pixmapFigure->setPosition(Point(rect.x, rect.y));
    pixmapFigure->setSize(rect.width, rect.height);
    pixmapFigure->setPixmapSize(rect.width, rect.height, INIT_BACKGROUND_COLOR, 1);
    invalidLayout = true;
}

double HeatMapPlotFigure::getYTickSize() const
{
    return yTickSize;
}

void HeatMapPlotFigure::setYTickSize(double size)
{
    if (yTickSize == size)
        return;

    yTickSize = size;
    invalidLayout = true;
}

void HeatMapPlotFigure::setYTickCount(int count)
{
    if (count != 0 && std::isfinite(minY) && std::isfinite(maxY))
        setYTickSize((maxY - minY) / (count - 1));
    else
        setYTickSize(INFINITY);
}

double HeatMapPlotFigure::getXTickSize() const
{
    return xTickSize;
}

void HeatMapPlotFigure::setXTickSize(double size)
{
    if (xTickSize == size)
        return;

    xTickSize = size;
}

void HeatMapPlotFigure::setXTickCount(int count)
{
    if (count != 0 && std::isfinite(minX) && std::isfinite(maxX))
        setXTickSize((maxX - minX) / (count - 1));
    else
        setXTickSize(INFINITY);
}

void HeatMapPlotFigure::setMinX(double value)
{
    if (minX == value)
        return;

    minX = value;
    invalidLayout = true;
}

void HeatMapPlotFigure::setMaxX(double value)
{
    if (maxX == value)
        return;

    maxX = value;
    invalidLayout = true;
}

void HeatMapPlotFigure::setMinY(double value)
{
    if (minY == value)
        return;

    minY = value;
    invalidLayout = true;
}

void HeatMapPlotFigure::setMaxY(double value)
{
    if (maxY == value)
        return;

    maxY = value;
    invalidLayout = true;
}

void HeatMapPlotFigure::setMinValue(double value)
{
    if (minValue == value)
        return;

    minValue = value;
    invalidLayout = true;
}

void HeatMapPlotFigure::setMaxValue(double value)
{
    if (maxValue == value)
        return;

    maxValue = value;
    invalidLayout = true;
}

int HeatMapPlotFigure::getLabelOffset() const
{
    return labelOffset;
}

void HeatMapPlotFigure::setLabelOffset(int offset)
{
    if (labelOffset == offset)
        return;

    labelOffset = offset;
    invalidLayout = true;
}

const cFigure::Font& HeatMapPlotFigure::getLabelFont() const
{
    return labelFigure->getFont();
}

void HeatMapPlotFigure::setLabelFont(const Font& font)
{
    labelFigure->setFont(font);
}

const cFigure::Color& HeatMapPlotFigure::getLabelColor() const
{
    return labelFigure->getColor();
}

void HeatMapPlotFigure::setLabelColor(const Color& color)
{
    labelFigure->setColor(color);
}

void HeatMapPlotFigure::parse(cProperty *property)
{
    cGroupFigure::parse(property);

    const char *s;

    setBounds(parseBounds(property, getBounds()));

    if ((s = property->getValue(PKEY_X_TICK_SIZE)) != nullptr)
        setXTickSize(atoi(s));
    if ((s = property->getValue(PKEY_Y_TICK_SIZE)) != nullptr)
        setYTickSize(atoi(s));
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

const char **HeatMapPlotFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {
            PKEY_Y_TICK_SIZE, PKEY_X_TICK_SIZE,
            PKEY_MIN_X, PKEY_MAX_X, PKEY_MIN_Y, PKEY_MAX_Y,
            PKEY_LABEL, PKEY_LABEL_OFFSET, PKEY_LABEL_COLOR, PKEY_LABEL_FONT, PKEY_POS,
            PKEY_SIZE, PKEY_ANCHOR, PKEY_BOUNDS, nullptr
        };
        concatArrays(keys, cGroupFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void HeatMapPlotFigure::addChildren()
{
    pixmapFigure = new cPixmapFigure("pixmap");
    pixmapFigure->fillPixmap(INIT_BACKGROUND_COLOR, 0);
    pixmapFigure->setAnchor(ANCHOR_NW);
    backgroundFigure = new cRectangleFigure("bounds");
    labelFigure = new cLabelFigure("label");
    labelFigure->setAnchor(ANCHOR_N);
    xAxisLabelFigure = new cLabelFigure("X axis label");
    xAxisLabelFigure->setAnchor(ANCHOR_S);
    yAxisLabelFigure = new cLabelFigure("Y axis label");
    yAxisLabelFigure->setAnchor(ANCHOR_S);
#if OMNETPP_BUILDNUM > 1500
    yAxisLabelFigure->setAngle(-90);
#endif

    addFigure(pixmapFigure);
    addFigure(backgroundFigure);
    addFigure(labelFigure);
    addFigure(xAxisLabelFigure);
    addFigure(yAxisLabelFigure);
}

static cFigure::Rectangle rectangleUnion(const cFigure::Rectangle& r1, const cFigure::Rectangle& r2)
{
    auto x1 = std::min(r1.x, r2.x);
    auto y1 = std::min(r1.y, r2.y);
    auto x2 = std::max(r1.x + r1.width, r2.x + r2.width);
    auto y2 = std::max(r1.y + r1.height, r2.y + r2.height);
    return cFigure::Rectangle(x1, y1, x2 - x1, y2 - y1);
}

void HeatMapPlotFigure::layout()
{
    redrawYTicks();
    redrawXTicks();

    Rectangle b = backgroundFigure->getBounds();
    double fontSize = xTicks.size() > 0 && xTicks[0].number ? xTicks[0].number->getFont().pointSize : 12;
    labelFigure->setPosition(Point(b.getCenter().x, b.y + b.height + fontSize * LABEL_Y_DISTANCE_FACTOR + labelOffset));
    xAxisLabelFigure->setPosition(Point(b.x + b.width / 2, b.y - 5));
    yAxisLabelFigure->setPosition(Point(-5, b.height / 2));

    bounds = backgroundFigure->getBounds();
    bounds = rectangleUnion(bounds, labelFigure->getBounds());
    bounds.x -= fontSize;
    bounds.y -= fontSize;
    bounds.width += 2 * fontSize;
    bounds.height += 2 * fontSize;
    invalidLayout = false;
}

void HeatMapPlotFigure::redrawYTicks()
{
    Rectangle bounds = pixmapFigure->getBounds();
    int numTicks = std::isfinite(yTickSize) ? std::abs(maxY - minY) / yTickSize + 1 : 0;

    double valueTickYposAdjust[2] = { 0, 0 };
    int fontSize = labelFigure->getFont().pointSize;
    if (yTicks.size() == 1) {
        valueTickYposAdjust[0] = - (fontSize / 2);
        valueTickYposAdjust[1] = fontSize / 2;
    }

    // Allocate ticks and numbers if needed
    if ((size_t)numTicks > yTicks.size()) {
        while ((size_t)numTicks > yTicks.size()) {
            cLineFigure *tick = new cLineFigure("yTick");
            cLineFigure *dashLine = new cLineFigure("yDashLine");
            cLabelFigure *number = new cLabelFigure("yNumber");

            dashLine->setLineStyle(LINE_DASHED);

            number->setAnchor(ANCHOR_W);
            tick->insertAbove(pixmapFigure);
            dashLine->insertAbove(pixmapFigure);
            number->insertAbove(pixmapFigure);
            yTicks.push_back(Tick(tick, dashLine, number));
        }
    }
    else {
        // Add or remove figures from canvas according to previous number of ticks
        for (int i = yTicks.size() - 1; i >= numTicks; --i) {
            delete removeFigure(yTicks[i].number);
            delete removeFigure(yTicks[i].dashLine);
            delete removeFigure(yTicks[i].tick);
            yTicks.pop_back();
        }
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
        double number = invertedYAxis ? maxY - i * yTickSize : minY + i * yTickSize;
        sprintf(buf, yValueFormat, number);
        yTicks[i].number->setText(buf);
        yTicks[i].number->setPosition(Point(x + 5, y + valueTickYposAdjust[i % 2]));
    }
}

void HeatMapPlotFigure::redrawXTicks()
{
    Rectangle bounds = pixmapFigure->getBounds();

    int numTicks = std::isfinite(xTickSize) ? (maxX - minX) / xTickSize + 1 : 0;

    // Allocate ticks and numbers if needed
    if ((size_t)numTicks > xTicks.size()) {
        while ((size_t)numTicks > xTicks.size()) {
            cLineFigure *tick = new cLineFigure("xTick");
            cLineFigure *dashLine = new cLineFigure("xDashLine");
            cLabelFigure *number = new cLabelFigure("xNumber");

            dashLine->setLineStyle(LINE_DASHED);

            number->setAnchor(ANCHOR_N);
            tick->insertAbove(pixmapFigure);
            dashLine->insertAbove(pixmapFigure);
            number->insertAbove(pixmapFigure);
            xTicks.push_back(Tick(tick, dashLine, number));
        }
    }
    else {
        // Add or remove figures from canvas according to previous number of ticks
        for (int i = xTicks.size() - 1; i >= numTicks; --i) {
            delete removeFigure(xTicks[i].number);
            delete removeFigure(xTicks[i].dashLine);
            delete removeFigure(xTicks[i].tick);
            xTicks.pop_back();
        }
    }

    for (uint32 i = 0; i < xTicks.size(); ++i) {
        double x = bounds.x + bounds.width * (i * xTickSize) / (maxX - minX);
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
        double number = invertedXAxis ? maxX - i * xTickSize : minX + i * xTickSize;
        sprintf(buf, xValueFormat, number);
        xTicks[i].number->setText(buf);
        xTicks[i].number->setPosition(Point(x, y + 5));
    }
}

void HeatMapPlotFigure::refreshDisplay()
{
    if (invalidLayout)
        layout();
}

} // namespace inet

