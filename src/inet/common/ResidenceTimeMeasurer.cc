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

#include "inet/common/ResidenceTimeMeasurer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/Simsignals.h"
#include "inet/common/TimeTag_m.h"

namespace inet {
namespace queueing {

simsignal_t ResidenceTimeMeasurer::packetStayedSignal = cComponent::registerSignal("packetStayed");

Define_Module(ResidenceTimeMeasurer);

void ResidenceTimeMeasurer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        auto networkNode = getContainingNode(this);
        auto measurementStartSignal = registerSignal(par("measurementStartSignal"));
        auto measurementEndSignal = registerSignal(par("measurementEndSignal"));
        networkNode->subscribe(measurementStartSignal, this);
        networkNode->subscribe(measurementEndSignal, this);
    }
}

void ResidenceTimeMeasurer::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    auto physicalSignal = check_and_cast<cPacket *>(object);
    auto packet = check_and_cast<Packet *>(physicalSignal->getEncapsulatedPacket());
    b offset = b(0);
    b length = packet->getDataLength();
    packet->addRegionTagsWhereAbsent<ResidenceTimeTag>(offset, length);
    if (signal == receptionStartedSignal) {
        packet->mapAllRegionTagsForUpdate<ResidenceTimeTag>(offset, length, [&] (b o, b l, const Ptr<ResidenceTimeTag>& tag) {
            tag->setReceptionStartTime(simTime());
        });
    }
    else if (signal == receptionEndedSignal) {
        packet->mapAllRegionTagsForUpdate<ResidenceTimeTag>(offset, length, [&] (b o, b l, const Ptr<ResidenceTimeTag>& tag) {
            tag->setReceptionEndTime(simTime());
        });
    }
    else if (signal == transmissionStartedSignal) {
        packet->mapAllRegionTagsForUpdate<ResidenceTimeTag>(offset, length, [&] (b o, b l, const Ptr<ResidenceTimeTag>& tag) {
            tag->setTransmissionStartTime(simTime());
        });
        emit(packetStayedSignal, packet);
        packet->removeRegionTagIfPresent<ResidenceTimeTag>(offset, length);
    }
    else if (signal == transmissionEndedSignal) {
        packet->mapAllRegionTagsForUpdate<ResidenceTimeTag>(offset, length, [&] (b o, b l, const Ptr<ResidenceTimeTag>& tag) {
            tag->setTransmissionEndTime(simTime());
        });
        emit(packetStayedSignal, packet);
        packet->removeRegionTagIfPresent<ResidenceTimeTag>(offset, length);
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace queueing
} // namespace inet

