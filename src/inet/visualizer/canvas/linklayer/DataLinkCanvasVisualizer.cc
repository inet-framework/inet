//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/linklayer/DataLinkCanvasVisualizer.h"

#include "inet/linklayer/base/MacProtocolBase.h"

#ifdef INET_WITH_PROTOCOLELEMENT
#include "inet/protocolelement/common/PacketEmitter.h"
#endif // INET_WITH_PROTOCOLELEMENT

#ifdef INET_WITH_IEEE80211
#include "inet/linklayer/ieee80211/mac/contract/ICoordinationFunction.h"
#endif // INET_WITH_IEEE80211

namespace inet {

namespace visualizer {

Define_Module(DataLinkCanvasVisualizer);

bool DataLinkCanvasVisualizer::isLinkStart(cModule *module) const
{
    // KLUDGE: for visualizing when using the layered Ethernet model
    return false
#ifdef INET_WITH_PROTOCOLELEMENT
            || dynamic_cast<PacketEmitter *>(module) != nullptr
#endif // INET_WITH_PROTOCOLELEMENT
            || dynamic_cast<MacProtocolBase *>(module) != nullptr
#ifdef INET_WITH_IEEE80211
            || dynamic_cast<ieee80211::ICoordinationFunction *>(module) != nullptr
#endif // INET_WITH_IEEE80211
        ;
}

bool DataLinkCanvasVisualizer::isLinkEnd(cModule *module) const
{
    // KLUDGE: for visualizing when using the layered Ethernet model
    return false
#ifdef INET_WITH_PROTOCOLELEMENT
            || dynamic_cast<PacketEmitter *>(module) != nullptr
#endif // INET_WITH_PROTOCOLELEMENT
            || dynamic_cast<MacProtocolBase *>(module) != nullptr
#ifdef INET_WITH_IEEE80211
            || dynamic_cast<ieee80211::ICoordinationFunction *>(module) != nullptr
#endif // INET_WITH_IEEE80211
        ;
}

const LinkVisualizerBase::LinkVisualization *DataLinkCanvasVisualizer::createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const
{
    auto linkVisualization = static_cast<const LinkCanvasVisualization *>(LinkCanvasVisualizerBase::createLinkVisualization(source, destination, packet));
    linkVisualization->figure->setTags((std::string("data_link ") + tags).c_str());
    linkVisualization->figure->setTooltip("This arrow represents a data link between two network nodes");
    linkVisualization->shiftPriority = 2;
    return linkVisualization;
}

} // namespace visualizer

} // namespace inet

