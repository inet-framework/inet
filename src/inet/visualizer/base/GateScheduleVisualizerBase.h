//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GATESCHEDULEVISUALIZERBASE_H
#define __INET_GATESCHEDULEVISUALIZERBASE_H

#include "inet/clock/common/ClockTime.h" // TODO
#include "inet/clock/contract/ClockTime.h"
#include "inet/queueing/contract/IPacketGate.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/GateFilter.h"
#include "inet/visualizer/util/Placement.h"
#include "inet/common/StringFormat.h"

namespace inet {

namespace visualizer {

class INET_API GateScheduleVisualizerBase : public VisualizerBase
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

    class INET_API DirectiveResolver : public StringFormat::IDirectiveResolver {
      protected:
        const cModule *module = nullptr;

      public:
        DirectiveResolver(const cModule *module) : module(module) {}

        virtual std::string resolveDirective(char directive) const override;
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayGateSchedules = false;
    StringFormat stringFormat;
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

    virtual std::string getGateScheduleVisualizationText(cModule *module) const;
};

} // namespace visualizer

} // namespace inet

#endif

