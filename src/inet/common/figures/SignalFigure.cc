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

#include "inet/common/figures/SignalFigure.h"

namespace inet {

SignalFigure::SignalFigure(const char *name) :
    cGroupFigure(name)
{
}

void SignalFigure::setColor(const cFigure::Color& color)
{
    for (auto ring : rings)
        ring->setFillColor(color);
    for (auto wave : waves)
        wave->setLineColor(color);
}

void SignalFigure::setRingCount(int count)
{
    for (int i = 0; i < count; i++) {
        auto ring = new cRingFigure("wave power");
        ring->setFilled(true);
        ring->setOutlined(false);
        rings.push_back(ring);
        addFigure(ring);
    }
}

void SignalFigure::setWaveCount(int count)
{
    for (int i = 0; i < count; i++) {
        auto wave = new cOvalFigure("wave crest");
        wave->setFilled(false);
        wave->setOutlined(true);
        waves.push_back(wave);
        addFigure(wave);
    }
}

void SignalFigure::refresh()
{
    auto center = bounds.getCenter();
    auto size = bounds.getSize() / 2;
    for (size_t i = 0; i < rings.size(); i++) {
        auto ring = rings[i];
        bool isLast = i == rings.size() - 1;
        double minR = i * ringSize;
        double maxRx = isLast ? DBL_MAX : minR + ringSize;
        double maxRy = isLast ? DBL_MAX : minR + ringSize;
        double innerRx = std::max(minR, std::min(maxRx, this->innerRx));
        double innerRy = std::max(minR, std::min(maxRy, this->innerRy));
        ring->setInnerRx(innerRx);
        ring->setInnerRy(innerRy);
        double outerRx = isLast ? size.x : std::min(size.x, maxRx);
        double outerRy = isLast ? size.y : std::min(size.y, maxRy);
        ring->setBounds(cFigure::Rectangle(center.x - outerRx, center.y - outerRy, 2 * outerRx, 2 * outerRy));
        ring->setVisible(innerRx < outerRx && innerRy < outerRy);
    }
    for (size_t i = 0; i < waves.size(); i++) {
        auto wave = waves[i];
        bool isLast = i == waves.size() - 1;
        double radius = waveOffset + i * waveLength;
        double outerRx = isLast ? size.x : std::min(size.x, radius);
        double outerRy = isLast ? size.y : std::min(size.y, radius);
        wave->setLineWidth(waveWidth);
        wave->setBounds(cFigure::Rectangle(center.x - outerRx, center.y - outerRy, 2 * outerRx, 2 * outerRy));
        wave->setVisible(innerRx < radius && innerRy < radius && radius < size.x && radius < size.y);
    }
    for (size_t i = 0; i < rings.size(); i++)
        rings[i]->setFillOpacity(opacity / pow(fadingFactor, i * ringSize / fadingDistance));
    for (size_t i = 0; i < waves.size(); i++)
        waves[i]->setLineOpacity(waveOpacityFactor * opacity / pow(fadingFactor, i * waveLength / fadingDistance));
}

} // namespace inet

