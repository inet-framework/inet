//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_TRACINGOBSTACLELOSSVISUALIZERBASE_H
#define __INET_TRACINGOBSTACLELOSSVISUALIZERBASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/ITracingObstacleLoss.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/AnimationPosition.h"

namespace inet {

namespace visualizer {

class INET_API TracingObstacleLossVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API ObstacleLossVisualization {
      public:
        mutable AnimationPosition obstacleLossAnimationPosition;

      public:
        ObstacleLossVisualization() { }
        virtual ~ObstacleLossVisualization() { }
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayIntersections = false;
    cFigure::Color intersectionLineColor;
    cFigure::LineStyle intersectionLineStyle;
    double intersectionLineWidth = NaN;
    bool displayFaceNormalVectors = false;
    cFigure::Color faceNormalLineColor;
    cFigure::LineStyle faceNormalLineStyle;
    double faceNormalLineWidth = NaN;
    const char *fadeOutMode = nullptr;
    double fadeOutTime = NaN;
    double fadeOutAnimationSpeed = NaN;
    //@}

    std::vector<const ObstacleLossVisualization *> obstacleLossVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual void preDelete(cComponent *root) override;

    virtual void subscribe();
    virtual void unsubscribe();

    // TODO: use ITransmission for identification?
    virtual const ObstacleLossVisualization *createObstacleLossVisualization(const physicallayer::ITracingObstacleLoss::ObstaclePenetratedEvent *obstaclePenetratedEvent) const = 0;
    virtual void addObstacleLossVisualization(const ObstacleLossVisualization *obstacleLossVisualization);
    virtual void removeObstacleLossVisualization(const ObstacleLossVisualization *obstacleLossVisualization);
    virtual void removeAllObstacleLossVisualizations();
    virtual void setAlpha(const ObstacleLossVisualization *obstacleLossVisualization, double alpha) const = 0;

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif

