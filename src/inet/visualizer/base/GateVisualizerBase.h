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

#ifndef __INET_GATEVISUALIZERBASE_H
#define __INET_GATEVISUALIZERBASE_H

#include "inet/clock/common/ClockTime.h" // TODO
#include "inet/clock/contract/ClockTime.h"
#include "inet/queueing/contract/IPacketGate.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/GateFilter.h"
#include "inet/visualizer/util/Placement.h"

namespace inet {

namespace visualizer {

class INET_API GateVisualizerBase : public VisualizerBase
{
  protected:
    class INET_API GateVisitor : public cVisitor {
      public:
        std::vector<queueing::IPacketGate *> gates;

      public:
        virtual bool visit(cObject *object) override;
    };

    class INET_API GateVisualization {
      public:
        queueing::IPacketGate *gate = nullptr;

      public:
        GateVisualization(queueing::IPacketGate *gate);
        virtual ~GateVisualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayGates = false;
    GateFilter gateFilter;
    double width;
    double height;
    double spacing;
    Placement placementHint;
    double placementPriority;
    clocktime_t displayDuration;
    double currentTimePosition;
    //@}

    mutable simtime_t lastRefreshTime = -1;
    std::vector<const GateVisualization *> gateVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshDisplay() const override;
    virtual void preDelete(cComponent *root) override;

    virtual GateVisualization *createGateVisualization(queueing::IPacketGate *gate) const = 0;
    virtual void addGateVisualization(const GateVisualization *gateVisualization);
    virtual void addGateVisualizations();
    virtual void removeGateVisualization(const GateVisualization *gateVisualization);
    virtual void refreshGateVisualization(const GateVisualization *gateVisualization) const = 0;
    virtual void removeAllGateVisualizations();
};

} // namespace visualizer

} // namespace inet

#endif

