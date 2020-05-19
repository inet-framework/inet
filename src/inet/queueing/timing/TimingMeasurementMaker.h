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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_TIMINGMEASUREMENTMAKER_H
#define __INET_TIMINGMEASUREMENTMAKER_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/common/TimeTag_m.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {
namespace queueing {

class INET_API TimingMeasurementMaker : public PacketFlowBase
{
  public:
    static simsignal_t lifeTimeSignal;
    static simsignal_t elapsedTimeSignal;
    static simsignal_t delayingTimeSignal;
    static simsignal_t queueingTimeSignal;
    static simsignal_t processingTimeSignal;
    static simsignal_t transmissionTimeSignal;
    static simsignal_t propagationTimeSignal;

  protected:
    PacketFilter packetFilter;
    b offset = b(0);
    b length = b(-1);
    bool endMeasurement_ = false;
    const char* flowName = nullptr;
    cMatchExpression flowNameMatcher;
    bool measureLifeTime = false;
    bool measureElapsedTime = false;
    bool measureDelayingTime = false;
    bool measureQueueingTime = false;
    bool measureProcessingTime = false;
    bool measureTransmissionTime = false;
    bool measurePropagationTime = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void makeMeasurement(Packet *packet, b offset, b length, const char *flowName, simsignal_t signal, simtime_t value);
    virtual void makeMeasurements(Packet *packet);
    virtual void endMeasurements(Packet *packet);

    template <typename T>
    void makeMeasurement(Packet *packet, b offset, b length, simsignal_t signal) {
        packet->mapAllRegionTags<T>(offset, length, [&] (b o, b l, const Ptr<const T>& timeTag) {
            for (int i = 0; i < (int)timeTag->getTotalTimesArraySize(); i++) {
                auto flowName = timeTag->getFlowNames(i);
                cMatchableString matchableFlowName(flowName);
                if (flowNameMatcher.matches(&matchableFlowName))
                    makeMeasurement(packet, o, l, flowName, signal, timeTag->getTotalTimes(i));
            }
        });
    }

    template <typename T>
    void endMeasurement(Packet *packet, b offset, b length) {
        packet->mapAllRegionTagsForUpdate<T>(offset, length, [&] (b o, b l, const Ptr<T>& timeTag) {
            for (int i = 0; i < (int)timeTag->getTotalTimesArraySize(); i++) {
                auto flowName = timeTag->getFlowNames(i);
                cMatchableString matchableFlowName(flowName);
                if (flowNameMatcher.matches(&matchableFlowName)) {
                    EV_INFO << "Stopping measurement on packet " << packet->getName() << ": "
                            << "range (" << offset << ", " << offset + length << "), ";
                    if (flowName != nullptr && *flowName != '\0')
                        EV_INFO << "flowName = " << flowName;
                    EV_INFO << std::endl;
                    timeTag->eraseFlowNames(i);
                    timeTag->eraseTotalTimes(i);
                    i--;
                    break;
                }
            }
        });
    }

  public:
    virtual void processPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_TIMINGMEASUREMENTMAKER_H

