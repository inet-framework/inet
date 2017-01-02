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

#ifndef __INET_SIGNALFIGURE_H
#define __INET_SIGNALFIGURE_H

#include "inet/common/INETMath.h"

namespace inet {

class INET_API SignalFigure : public cGroupFigure
{
  protected:
    double opacity = NaN;
    double ringSize = NaN;
    double fadingDistance = NaN;
    double fadingFactor = NaN;
    double waveLength = NaN;
    double waveWidth = NaN;
    double waveOffset = NaN;
    double waveOpacityFactor = NaN;
    std::vector<cRingFigure *> rings;
    std::vector<cOvalFigure *> waves;

    Rectangle bounds;
    double innerRx = NaN;
    double innerRy = NaN;

  public:
    SignalFigure(const char *name = nullptr);

    void setColor(const cFigure::Color& color);
    void setOpacity(double opacity) { this->opacity = opacity; }
    void setBounds(const cFigure::Rectangle& bounds) { this->bounds = bounds; }
    void setInnerRx(double innerRx) { this->innerRx = innerRx; }
    void setInnerRy(double innerRy) { this->innerRy = innerRy; }

    int getRingCount() const { return rings.size(); }
    void setRingCount(int count);

    double getRingSize() const { return ringSize; }
    void setRingSize(double ringSize) { this->ringSize = ringSize; }

    double getFadingDistance() const { return fadingDistance; }
    void setFadingDistance(double fadingDistance) { this->fadingDistance = fadingDistance; }

    double getFadingFactor() const { return fadingFactor; }
    void setFadingFactor(double fadingFactor) { this->fadingFactor = fadingFactor; }

    int getWaveCount() const { return waves.size(); }
    void setWaveCount(int count);

    double getWaveLength() const { return waveLength; }
    void setWaveLength(double waveLength) { this->waveLength = waveLength; }

    double getWaveWidth() const { return waveWidth; }
    void setWaveWidth(double waveWidth) { this->waveWidth = waveWidth; }

    double getWaveOffset() const { return waveOffset; }
    void setWaveOffset(double waveOffset) { this->waveOffset = waveOffset; }

    double getWaveOpacityFactor() const { return waveOpacityFactor; }
    void setWaveOpacityFactor(double waveOpacityFactor) { this->waveOpacityFactor = waveOpacityFactor; }

    void refresh();
};

} // namespace inet

#endif // ifndef __INET_SIGNALFIGURE_H

