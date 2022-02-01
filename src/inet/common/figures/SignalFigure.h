//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#endif

