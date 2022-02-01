//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LINKBREAKVISUALIZERBASE_H
#define __INET_LINKBREAKVISUALIZERBASE_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/AnimationPosition.h"
#include "inet/visualizer/util/InterfaceFilter.h"
#include "inet/visualizer/util/LineManager.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"

namespace inet {

namespace visualizer {

class INET_API LinkBreakVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API LinkBreakVisualization {
      public:
        mutable AnimationPosition linkBreakAnimationPosition;
        const int transmitterModuleId = -1;
        const int receiverModuleId = -1;

      public:
        LinkBreakVisualization(int transmitterModuleId, int receiverModuleId);
        virtual ~LinkBreakVisualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayLinkBreaks = false;
    NetworkNodeFilter nodeFilter;
    InterfaceFilter interfaceFilter;
    PacketFilter packetFilter;
    const char *icon = nullptr;
    double iconTintAmount = NaN;
    cFigure::Color iconTintColor;
    const char *fadeOutMode = nullptr;
    double fadeOutTime = NaN;
    double fadeOutAnimationSpeed = NaN;
    //@}

    std::map<std::pair<int, int>, const LinkBreakVisualization *> linkBreakVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshDisplay() const override;
    virtual void preDelete(cComponent *root) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual const LinkBreakVisualization *createLinkBreakVisualization(cModule *transmitter, cModule *receiver) const = 0;
    virtual void addLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization);
    virtual void removeLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization);
    virtual void removeAllLinkBreakVisualizations();
    virtual void setAlpha(const LinkBreakVisualization *linkBreakVisualization, double alpha) const = 0;

    virtual cModule *findNode(MacAddress address);

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif

