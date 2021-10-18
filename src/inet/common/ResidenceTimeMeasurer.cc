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

simsignal_t ResidenceTimeMeasurer::packetStayedSignal = cComponent::registerSignal("packetStayed");

Define_Module(ResidenceTimeMeasurer);

void ResidenceTimeMeasurer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this);
        auto measurementStartSignal = registerSignal(par("measurementStartSignal"));
        auto measurementEndSignal = registerSignal(par("measurementEndSignal"));
        subscriptionModule->subscribe(measurementStartSignal, this);
        subscriptionModule->subscribe(measurementEndSignal, this);
        subscriptionModule->subscribe(packetCreatedSignal, this);
        subscriptionModule->subscribe(packetDroppedSignal, this);
    }
}

void ResidenceTimeMeasurer::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    if (signal == packetCreatedSignal) {
        auto packet = check_and_cast<Packet *>(object);
        b offset = b(0);
        b length = packet->getDataLength();
        packet->addRegionTagsWhereAbsent<ResidenceTimeTag>(offset, length);
        packet->mapAllRegionTagsForUpdate<ResidenceTimeTag>(offset, length, [&] (b o, b l, const Ptr<ResidenceTimeTag>& tag) {
            tag->setStartTime(simTime());
        });
    }
    else if (signal == packetDroppedSignal) {
        auto packet = check_and_cast<Packet *>(object);
        b offset = b(0);
        b length = packet->getDataLength();
        packet->mapAllRegionTagsForUpdate<ResidenceTimeTag>(offset, length, [&] (b o, b l, const Ptr<ResidenceTimeTag>& tag) {
            tag->setEndTime(simTime());
        });
        emit(packetStayedSignal, packet);
        packet->removeRegionTagsWherePresent<ResidenceTimeTag>(offset, length);
    }
    else {
        auto physicalSignal = check_and_cast<cPacket *>(object);
        auto packet = check_and_cast<Packet *>(physicalSignal->getEncapsulatedPacket());
        b offset = b(0);
        b length = packet->getDataLength();
        if (signal == receptionStartedSignal) {
            packet->addRegionTagsWhereAbsent<ResidenceTimeTag>(offset, length);
            packet->mapAllRegionTagsForUpdate<ResidenceTimeTag>(offset, length, [&] (b o, b l, const Ptr<ResidenceTimeTag>& tag) {
                tag->setStartTime(simTime());
            });
        }
        else if (signal == receptionEndedSignal) {
            packet->addRegionTagsWhereAbsent<ResidenceTimeTag>(offset, length);
            packet->mapAllRegionTagsForUpdate<ResidenceTimeTag>(offset, length, [&] (b o, b l, const Ptr<ResidenceTimeTag>& tag) {
                tag->setStartTime(simTime());
            });
        }
        else if (signal == transmissionStartedSignal) {
            packet->mapAllRegionTagsForUpdate<ResidenceTimeTag>(offset, length, [&] (b o, b l, const Ptr<ResidenceTimeTag>& tag) {
                tag->setEndTime(simTime());
            });
            emit(packetStayedSignal, packet);
            packet->removeRegionTagsWherePresent<ResidenceTimeTag>(offset, length);
        }
        else if (signal == transmissionEndedSignal) {
            packet->mapAllRegionTagsForUpdate<ResidenceTimeTag>(offset, length, [&] (b o, b l, const Ptr<ResidenceTimeTag>& tag) {
                tag->setEndTime(simTime());
            });
            emit(packetStayedSignal, packet);
            packet->removeRegionTagsWherePresent<ResidenceTimeTag>(offset, length);
        }
        else
            throw cRuntimeError("Unknown signal");
    }
}

} // namespace inet

