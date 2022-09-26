//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LINKVISUALIZERBASE_H
#define __INET_LINKVISUALIZERBASE_H

#include "inet/common/StringFormat.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/AnimationPosition.h"
#include "inet/visualizer/util/InterfaceFilter.h"
#include "inet/visualizer/util/LineManager.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"

namespace inet {

namespace visualizer {

class INET_API LinkVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    enum ActivityLevel {
        ACTIVITY_LEVEL_SERVICE,
        ACTIVITY_LEVEL_PEER,
        ACTIVITY_LEVEL_PROTOCOL,
    };

    class INET_API LinkVisualization : public LineManager::ModuleLine {
      public:
        mutable AnimationPosition lastUsageAnimationPosition;

      public:
        LinkVisualization(int sourceModuleId, int destinationModuleId);
        virtual ~LinkVisualization() {}
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
    bool displayLinks = false;
    ActivityLevel activityLevel = static_cast<ActivityLevel>(-1);
    NetworkNodeFilter nodeFilter;
    InterfaceFilter interfaceFilter;
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
     * Maps packet to last module.
     */
    std::map<int, int> lastModules;
    /**
     * Maps source/destination module ids to link visualizations.
     */
    std::map<std::pair<int, int>, const LinkVisualization *> linkVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshDisplay() const override;
    virtual void preDelete(cComponent *root) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual bool isLinkStart(cModule *module) const = 0;
    virtual bool isLinkEnd(cModule *module) const = 0;

    virtual const LinkVisualization *createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const = 0;
    virtual const LinkVisualization *getLinkVisualization(std::pair<int, int> linkVisualization);
    virtual void addLinkVisualization(std::pair<int, int> sourceAndDestination, const LinkVisualization *linkVisualization);
    virtual void removeLinkVisualization(const LinkVisualization *linkVisualization);
    virtual void removeAllLinkVisualizations();
    virtual void setAlpha(const LinkVisualization *linkVisualization, double alpha) const = 0;

    virtual cModule *getLastModule(int treeId);
    virtual void setLastModule(int treeId, cModule *lastModule);
    virtual void removeLastModule(int treeId);

    virtual std::string getLinkVisualizationText(cPacket *packet) const;
    virtual void refreshLinkVisualization(const LinkVisualization *linkVisualization, cPacket *packet);
    virtual void updateLinkVisualization(cModule *source, cModule *destination, cPacket *packet);

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif

