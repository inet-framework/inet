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

#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/common/packetlevel/NoiseSource.h"
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"

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
        medium = getModuleFromPar<IRadioMedium>(par("radioMediumModule"), this);
        mediumModuleId = check_and_cast<cModule *>(medium)->getId();
        radioIn = gate("radioIn");
        radioIn->setDeliverOnReceptionStart(true);
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

