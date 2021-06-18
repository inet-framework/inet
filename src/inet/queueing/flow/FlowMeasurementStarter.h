//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FLOWMEASUREMENTSTARTER_H
#define __INET_FLOWMEASUREMENTSTARTER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {
namespace queueing {

class INET_API FlowMeasurementStarter : public PacketFlowBase, public TransparentProtocolRegistrationListener
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

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

    template<typename T>
    void startMeasurement(Packet *packet, b offset, b length, simtime_t value) const {
        if (length == b(-1))
            length = packet->getTotalLength();
        EV_INFO << "Starting measurement on packet" << EV_FIELD(offset) << EV_FIELD(length);
        if (flowName != nullptr && *flowName != '\0')
            EV_INFO << EV_FIELD(flowName);
        EV_INFO << EV_FIELD(class, typeid(T).name()) << EV_ENDL;
        packet->addRegionTagsWhereAbsent<T>(offset, length);
        packet->mapAllRegionTagsForUpdate<T>(offset, length, [&] (b o, b l, const Ptr<T>& timeTag) {
            timeTag->appendFlowNames(flowName);
            timeTag->appendBitTotalTimes(value);
            timeTag->appendPacketTotalTimes(value);
        });
    }

    virtual void startMeasurements(Packet *packet) const;

  public:
    virtual void processPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

