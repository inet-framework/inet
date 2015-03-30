//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Consts.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandTransmitterBase.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandReceiverBase.h"
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"

namespace inet {

using namespace ieee80211;

namespace physicallayer {

Define_Module(Ieee80211Radio);

simsignal_t Ieee80211Radio::radioChannelChangedSignal = cComponent::registerSignal("radioChannelChanged");

Ieee80211Radio::Ieee80211Radio() :
    FlatRadioBase(),
    channelNumber(-1)
{
}

void Ieee80211Radio::initialize(int stage)
{
    FlatRadioBase::initialize(stage);
    if (stage == INITSTAGE_PHYSICAL_LAYER)
        setChannelNumber(par("channelNumber"));
}

void Ieee80211Radio::handleUpperCommand(cMessage *message)
{
    if (message->getKind() == RADIO_C_CONFIGURE) {
        ConfigureRadioCommand *configureCommand = check_and_cast<ConfigureRadioCommand *>(message->getControlInfo());
        FlatRadioBase::handleUpperCommand(message);
        int newChannelNumber = configureCommand->getChannelNumber();
        if (newChannelNumber != -1)
            setChannelNumber(newChannelNumber);
    }
    else
        FlatRadioBase::handleUpperCommand(message);
}

void Ieee80211Radio::setChannelNumber(int newChannelNumber)
{
    if (channelNumber != newChannelNumber) {
        Hz carrierFrequency = Hz(CENTER_FREQUENCIES[newChannelNumber + 1]);
        NarrowbandTransmitterBase *narrowbandTransmitter = const_cast<NarrowbandTransmitterBase *>(check_and_cast<const NarrowbandTransmitterBase *>(transmitter));
        NarrowbandReceiverBase *narrowbandReceiver = const_cast<NarrowbandReceiverBase *>(check_and_cast<const NarrowbandReceiverBase *>(receiver));
        narrowbandTransmitter->setCarrierFrequency(carrierFrequency);
        narrowbandReceiver->setCarrierFrequency(carrierFrequency);
        EV << "Changing radio channel from " << channelNumber << " to " << newChannelNumber << ".\n";
        channelNumber = newChannelNumber;
        endReceptionTimer = nullptr;
        emit(radioChannelChangedSignal, newChannelNumber);
        emit(listeningChangedSignal, 0);
    }
}

} // namespace physicallayer

} // namespace inet

