//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    Enter_Method("%s", cComponent::getSignalName(signal));

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
            EV_DEBUG << "Reception started" << EV_FIELD(packet) << std::endl;
            packet->addRegionTagsWhereAbsent<ResidenceTimeTag>(offset, length);
            packet->mapAllRegionTagsForUpdate<ResidenceTimeTag>(offset, length, [&] (b o, b l, const Ptr<ResidenceTimeTag>& tag) {
                tag->setStartTime(simTime());
            });
        }
        else if (signal == receptionEndedSignal) {
            EV_DEBUG << "Reception ended" << EV_FIELD(packet) << std::endl;
            packet->addRegionTagsWhereAbsent<ResidenceTimeTag>(offset, length);
            packet->mapAllRegionTagsForUpdate<ResidenceTimeTag>(offset, length, [&] (b o, b l, const Ptr<ResidenceTimeTag>& tag) {
                tag->setStartTime(simTime());
            });
        }
        else if (signal == transmissionStartedSignal) {
            EV_DEBUG << "Transmission started" << EV_FIELD(packet) << std::endl;
            packet->mapAllRegionTagsForUpdate<ResidenceTimeTag>(offset, length, [&] (b o, b l, const Ptr<ResidenceTimeTag>& tag) {
                tag->setEndTime(simTime());
            });
            emit(packetStayedSignal, packet);
            packet->removeRegionTagsWherePresent<ResidenceTimeTag>(offset, length);
        }
        else if (signal == transmissionEndedSignal) {
            EV_DEBUG << "Transmission ended" << EV_FIELD(packet) << std::endl;
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

