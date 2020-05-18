//
// Copyright (C) OpenSim Ltd.
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

#include "inet/common/FlowTag.h"

#ifdef WITH_ETHERNET
#include "inet/linklayer/ethernet/switch/MacRelayUnit.h"
#endif

#ifdef WITH_IEEE8021D
#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"
#endif

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4.h"
#endif

#include "inet/visualizer/flow/PacketFlowCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(PacketFlowCanvasVisualizer);

bool PacketFlowCanvasVisualizer::isPathStart(cModule *module) const
{
    return true;
}

bool PacketFlowCanvasVisualizer::isPathEnd(cModule *module) const
{
    return true;
}

bool PacketFlowCanvasVisualizer::isPathElement(cModule *module) const
{
#ifdef WITH_ETHERNET
    if (dynamic_cast<MacRelayUnit *>(module) != nullptr)
        return true;
#endif

#ifdef WITH_IEEE8021D
    if (dynamic_cast<Ieee8021dRelay *>(module) != nullptr)
        return true;
#endif

#ifdef WITH_IPv4
    if (dynamic_cast<Ipv4 *>(module) != nullptr)
        return true;
#endif

    return false;
}

const PathCanvasVisualizerBase::PathVisualization *PacketFlowCanvasVisualizer::createPathVisualization(const char *label, const std::vector<int>& path, cPacket *packet) const
{
    auto pathVisualization = static_cast<const PathCanvasVisualization *>(PathCanvasVisualizerBase::createPathVisualization(label, path, packet));
    pathVisualization->figure->setTags((std::string("transport_route ") + tags).c_str());
    pathVisualization->figure->setTooltip("This polyline arrow represents a recently active packet flow route between two network nodes");
    pathVisualization->shiftPriority = 4;
    return pathVisualization;
}

void PacketFlowCanvasVisualizer::processPathElement(cModule *networkNode, const char *label, Packet *packet)
{
    packet->mapAllRegionTags<FlowTag>(b(0), packet->getTotalLength(), [&] (b offset, b length, const Ptr<const FlowTag>& flowTag) {
        for (int i = 0; i < (int)flowTag->getNamesArraySize(); i++) {
            auto label = flowTag->getNames(i);
            mapChunks(packet->peekAt(b(0), packet->getTotalLength()), [&] (const Ptr<const Chunk>& chunk, int chunkId) {
                std::cout << "Adding " << networkNode->getFullPath() << " with label " << label << " and id " << chunkId << " to path.\n";
                auto path = getIncompletePath(label, chunkId);
                if (path != nullptr) {
                    addToIncompletePath(label, chunkId, networkNode);
                }
            });
        }
    });
}

} // namespace visualizer

} // namespace inet

