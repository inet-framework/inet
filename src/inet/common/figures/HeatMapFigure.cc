//
// Copyright (C) 2016 OpenSim Ltd.
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

#include "inet/common/figures/HeatMapFigure.h"

namespace inet {

HeatMapFigure::HeatMapFigure(int size, const char *name) :
    cPixmapFigure(name)
{
    setPixmap(cFigure::Pixmap(size, size));
    fillPixmap(fromColor, 0);
}

double HeatMapFigure::getHeat(int x, int y)
{
    return getPixelOpacity(x, y);
}

void HeatMapFigure::setHeat(int x, int y, double value)
{
    cFigure::Color color(
            (1 - value) * fromColor.red + value * toColor.red,
            (1 - value) * fromColor.green + value * toColor.green,
            (1 - value) * fromColor.blue + value * toColor.blue);
    setPixel(x, y, color, value);
}

void HeatMapFigure::heatPoint(int x, int y)
{
    double value = alpha + (1 - alpha) * getHeat(x, y);
    setHeat(x, y, value);
}

void HeatMapFigure::heatLine(int x1, int y1, int x2, int y2)
{
    // NOTE: http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    bool steep = (std::abs(y2 - y1) > std::abs(x2 - x1));
    if (steep) {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }
    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    double dx = x2 - x1;
    double dy = std::abs(y2 - y1);
    double error = dx / 2.0f;
    int yStep = (y1 < y2) ? 1 : -1;
    int y = y1;
    for (int x = x1; x <= x2; x++) {
        if (steep)
            heatPoint(y, x);
        else
            heatPoint(x, y);
        error -= dy;
        if (error < 0) {
            y += yStep;
            error += dx;
        }
    }
}

void HeatMapFigure::coolDown()
{
    double factor = exp(coolingSpeed * (lastCoolDown - simTime()).dbl());
    if (factor < 0.9) {
        lastCoolDown = simTime();
        for (int x = 0; x < getPixmapWidth(); x++)
            for (int y = 0; y < getPixmapHeight(); y++)
                setHeat(x, y, getHeat(x, y) * factor);
    }
}

} // namespace inet

