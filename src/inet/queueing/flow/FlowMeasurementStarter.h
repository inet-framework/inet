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

#ifndef __INET_FLOWMEASUREMENTSTARTER_H
#define __INET_FLOWMEASUREMENTSTARTER_H

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {
namespace queueing {

class INET_API FlowMeasurementStarter : public PacketFlowBase
{
  protected:
    PacketFilter packetFilter;
    b offset = b(0);
    b length = b(-1);
    const char *flowName = nullptr;
    bool measureElapsedTime = false;
    bool measureDelayingTime = false;
    bool measureQueueingTime = false;
    bool measureProcessingTime = false;
    bool measureTransmissionTime = false;
    bool measurePropagationTime = false;
    bool measurePacketEvents = false;

  protected:
    virtual void initialize(int stage) override;

    template <typename T>
    void startMeasurement(Packet *packet, b offset, b length, simtime_t value) const {
        if (length == b(-1))
            length = packet->getTotalLength();
        EV_INFO << "Starting measurement on packet" << EV_FIELD(offset) << EV_FIELD(length);
        if (flowName != nullptr && *flowName != '\0')
            EV_INFO << EV_FIELD(flowName);
        EV_INFO << EV_FIELD(class, typeid(T).name()) << EV_ENDL;
        packet->addRegionTagsWhereAbsent<T>(offset, length);
        packet->mapAllRegionTagsForUpdate<T>(offset, length, [&] (b o, b l, const Ptr<T>& timeTag) {
            timeTag->insertFlowNames(flowName);
            timeTag->insertTotalTimes(value);
        });
    }

    virtual void startMeasurements(Packet *packet) const;

  public:
    virtual void processPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

