//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/noise/NoiseSource.h"

#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/wireless/common/medium/RadioMedium.h"

namespace inet {
namespace physicallayer {

Define_Module(NoiseSource);

NoiseSource::~NoiseSource()
{
    // NOTE: can't use the medium module here, because it may have been already deleted
    cModule *medium = getSimulation()->getModule(mediumModuleId);
    if (medium != nullptr)
        check_and_cast<IRadioMedium *>(medium)->removeRadio(this);
    cancelAndDelete(sleepTimer);
    cancelAndDelete(transmissionTimer);
}

void NoiseSource::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        sleepTimer = new cMessage("sleepTimer");
        transmissionTimer = new cMessage("transmissionTimer");
        antenna = check_and_cast<IAntenna *>(getSubmodule("antenna"));
        transmitter = check_and_cast<ITransmitter *>(getSubmodule("transmitter"));
        medium.reference(this, "radioMediumModule", true);
        mediumModuleId = check_and_cast<cModule *>(medium.get())->getId();
        radioIn = gate("radioIn");
        radioIn->setDeliverImmediately(true);
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        medium->addRadio(this);
        scheduleSleepTimer();
    }
}

void NoiseSource::handleMessage(cMessage *message)
{
    if (message == sleepTimer)
        startTransmission();
    else if (message == transmissionTimer)
        endTransmission();
    else
        delete message;
}

void NoiseSource::startTransmission()
{
    WirelessSignal *signal = check_and_cast<WirelessSignal *>(medium->transmitPacket(this, nullptr));
    transmissionTimer->setContextPointer(const_cast<WirelessSignal *>(signal));
    scheduleTransmissionTimer(signal->getTransmission());
}

void NoiseSource::endTransmission()
{
    transmissionTimer->setContextPointer(nullptr);
    scheduleSleepTimer();
}

void NoiseSource::scheduleSleepTimer()
{
    scheduleAfter(par("sleepInterval"), sleepTimer);
}

void NoiseSource::scheduleTransmissionTimer(const ITransmission *transmission)
{
    scheduleAfter(transmission->getDuration(), transmissionTimer);
}

const ITransmission *NoiseSource::getTransmissionInProgress() const
{
    if (!transmissionTimer->isScheduled())
        return nullptr;
    else
        return static_cast<WirelessSignal *>(transmissionTimer->getContextPointer())->getTransmission();
}

} // namespace physicallayer
} // namespace inet

