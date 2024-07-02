//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CHANNELVISUALIZERBASE_H
#define __INET_CHANNELVISUALIZERBASE_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/AnimationPosition.h"
#include "inet/visualizer/util/LineManager.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"

namespace inet {

namespace visualizer {

class INET_API ChannelVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API ChannelVisualization : public LineManager::ModuleLine {
      public:
        mutable AnimationPosition lastUsageAnimationPosition;

      public:
        ChannelVisualization(int sourceModuleId, int destinationModuleId);
        virtual ~ChannelVisualization() {}
    };

    class INET_API DirectiveResolver : public StringFormat::IDirectiveResolver {
      protected:
        const cPacket *packet = nullptr;

      public:
        DirectiveResolver(const cPacket *packet) : packet(packet) {}

        virtual std::string resolveDirective(char directive) const override;
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayChannelActivity = false;
    NetworkNodeFilter nodeFilter;
    PacketFilter packetFilter;
    cFigure::Color lineColor;
    cFigure::LineStyle lineStyle;
    double lineWidth = NaN;
    double lineShift = NaN;
    const char *lineShiftMode = nullptr;
    double lineContactSpacing = NaN;
    const char *lineContactMode = nullptr;
    StringFormat labelFormat;
    cFigure::Font labelFont;
    cFigure::Color labelColor;
    const char *fadeOutMode = nullptr;
    double fadeOutTime = NaN;
    double fadeOutAnimationSpeed = NaN;
    double holdAnimationTime = NaN;
    //@}

    LineManager *lineManager = nullptr;
    /**
     * Maps source/destination module ids to channel activity visualizations.
     */
    std::map<std::pair<int, int>, const ChannelVisualization *> channelVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshDisplay() const override;
    virtual void preDelete(cComponent *root) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual const ChannelVisualization *createChannelVisualization(cModule *source, cModule *destination, cPacket *packet) const = 0;
    virtual const ChannelVisualization *getChannelVisualization(std::pair<int, int> channelVisualization);
    virtual void addChannelVisualization(std::pair<int, int> sourceAndDestination, const ChannelVisualization *channelVisualization);
    virtual void removeChannelVisualization(const ChannelVisualization *channelVisualization);
    virtual void removeAllChannelVisualizations();
    virtual void setAlpha(const ChannelVisualization *channelVisualization, double alpha) const = 0;

    virtual std::string getChannelVisualizationText(cPacket *packet) const;
    virtual void refreshChannelVisualization(const ChannelVisualization *channelVisualization, cPacket *packet);
    virtual void updateChannelVisualization(cModule *source, cModule *destination, cPacket *packet);

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif

