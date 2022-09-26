//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDROPVISUALIZERBASE_H
#define __INET_PACKETDROPVISUALIZERBASE_H

#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/AnimationPosition.h"
#include "inet/visualizer/util/ColorSet.h"
#include "inet/visualizer/util/InterfaceFilter.h"
#include "inet/visualizer/util/LineManager.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"

namespace inet {

namespace visualizer {

class INET_API PacketDrop : public PacketDropDetails
{
  protected:
    const cPacket *packet = nullptr;
    const int moduleId = -1;
    const Coord position;

  public:
    PacketDrop(PacketDropReason reason, const cPacket *packet, const int moduleId, const Coord& position);
    virtual ~PacketDrop();

    const cPacket *getPacket_() const { return packet; }
    const cModule *getModule() const;
    const cModule *getNetworkNode() const;
    const NetworkInterface *getNetworkInterface() const;
    const Coord& getPosition() const { return position; }
};

class INET_API PacketDropVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API PacketDropVisualization {
      public:
        mutable AnimationPosition packetDropAnimationPosition;
        const PacketDrop *packetDrop = nullptr;

      public:
        PacketDropVisualization(const PacketDrop *packetDrop);
        virtual ~PacketDropVisualization();
    };

    class INET_API DirectiveResolver : public StringFormat::IDirectiveResolver {
      protected:
        const PacketDrop *packetDrop = nullptr;

      public:
        DirectiveResolver(const PacketDrop *packetDrop);

        virtual std::string resolveDirective(char directive) const override;
    };

    class INET_API DetailsFilter {
      protected:
        cMatchExpression matchExpression;

      public:
        void setPattern(const char *pattern);

        bool matches(const PacketDropDetails *details) const;
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayPacketDrops = false;
    NetworkNodeFilter nodeFilter; // TODO
    InterfaceFilter interfaceFilter; // TODO
    PacketFilter packetFilter;
    DetailsFilter detailsFilter;
    const char *icon = nullptr;
    ColorSet iconTintColorSet;
    double iconTintAmount = NaN;
    StringFormat labelFormat; // TODO
    cFigure::Font labelFont;
    cFigure::Color labelColor;
    const char *fadeOutMode = nullptr;
    double fadeOutTime = NaN;
    double fadeOutAnimationSpeed = NaN;
    //@}

    std::vector<const PacketDropVisualization *> packetDropVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshDisplay() const override;
    virtual void preDelete(cComponent *root) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual const PacketDropVisualization *createPacketDropVisualization(PacketDrop *packetDrop) const = 0;
    virtual void addPacketDropVisualization(const PacketDropVisualization *packetDropVisualization);
    virtual void removePacketDropVisualization(const PacketDropVisualization *packetDropVisualization);
    virtual void removeAllPacketDropVisualizations();
    virtual void setAlpha(const PacketDropVisualization *packetDropVisualization, double alpha) const = 0;

    virtual std::string getPacketDropVisualizationText(const PacketDrop *packetDrop) const;

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif

