//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FLOWMEASUREMENTRECORDER_H
#define __INET_FLOWMEASUREMENTRECORDER_H

#include "inet/common/JsonWriter.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/common/TimeTag_m.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {
namespace queueing {

class INET_API FlowMeasurementRecorder : public PacketFlowBase
{
  public:
    static simsignal_t packetFlowMeasuredSignal;

  protected:
    PacketFilter packetFilter;
    b offset = b(0);
    b length = b(-1);
    bool endMeasurement_ = false;
    const char *flowName = nullptr;
    cMatchExpression flowNameMatcher;
    bool measureLifeTime = false;
    bool measureElapsedTime = false;
    bool measureDelayingTime = false;
    bool measureQueueingTime = false;
    bool measureProcessingTime = false;
    bool measureTransmissionTime = false;
    bool measurePropagationTime = false;
    bool measurePacketEvent = false;
    JsonWriter packetEventFile;

  protected:
    virtual void initialize(int stage) override;
    virtual void makeMeasurements(Packet *packet);
    virtual void endMeasurements(Packet *packet);

    template<typename T>
    void endMeasurement(Packet *packet, b offset, b length) {
        packet->mapAllRegionTagsForUpdate<T>(offset, length, [&] (b o, b l, const Ptr<T>& timeTag) {
            for (size_t i = 0; i < timeTag->getBitTotalTimesArraySize(); i++) {
                auto flowName = timeTag->getFlowNames(i);
                cMatchableString matchableFlowName(flowName);
                if (flowNameMatcher.matches(&matchableFlowName)) {
                    EV_INFO << "Stopping measurement on packet" << EV_FIELD(offset) << EV_FIELD(length);
                    if (flowName != nullptr && *flowName != '\0')
                        EV_INFO << EV_FIELD(flowName);
                    EV_INFO << EV_FIELD(packet) << EV_ENDL;
                    timeTag->eraseFlowNames(i);
                    timeTag->eraseBitTotalTimes(i);
                    timeTag->erasePacketTotalTimes(i);
                    i--;
                    break;
                }
            }
        });
    }

  public:
    virtual ~FlowMeasurementRecorder();
    virtual void processPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

