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

#include "inet/physicallayer/ieee80211/Ieee80211Radio.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Consts.h"
#include "inet/physicallayer/base/NarrowbandTransmitterBase.h"
#include "inet/physicallayer/base/NarrowbandReceiverBase.h"
#include "inet/physicallayer/contract/RadioControlInfo_m.h"

namespace inet {

using namespace ieee80211;

namespace physicallayer {

Define_Module(Ieee80211Radio);

simsignal_t Ieee80211Radio::radioChannelChangedSignal = cComponent::registerSignal("radioChannelChanged");

Ieee80211Radio::Ieee80211Radio() :
    Radio(),
    channelNumber(-1)
{
}

void Ieee80211Radio::initialize(int stage)
{
    Radio::initialize(stage);
    if (stage == INITSTAGE_PHYSICAL_LAYER)
        setChannelNumber(par("channelNumber"));
}

void Ieee80211Radio::handleUpperCommand(cMessage *message)
{
    if (message->getKind() == RADIO_C_CONFIGURE) {
        RadioConfigureCommand *configureCommand = check_and_cast<RadioConfigureCommand *>(message->getControlInfo());
        bps newBitrate = configureCommand->getBitrate();
        if (!isNaN(newBitrate.get()))
            setBitrate(newBitrate);
        int newChannelNumber = configureCommand->getChannelNumber();
        if (newChannelNumber != -1)
            setChannelNumber(newChannelNumber);
        delete message;
    }
    else
        Radio::handleUpperCommand(message);
}

void Ieee80211Radio::setBitrate(bps newBitrate)
{
    NarrowbandTransmitterBase *flatTransmitter = const_cast<NarrowbandTransmitterBase *>(check_and_cast<const NarrowbandTransmitterBase *>(transmitter));
    flatTransmitter->setBitrate(newBitrate);
    endReceptionTimer = NULL;
}

void Ieee80211Radio::setChannelNumber(int newChannelNumber)
{
    if (channelNumber != newChannelNumber) {
        Hz carrierFrequency = Hz(CENTER_FREQUENCIES[newChannelNumber + 1]);
        NarrowbandTransmitterBase *flatTransmitter = const_cast<NarrowbandTransmitterBase *>(check_and_cast<const NarrowbandTransmitterBase *>(transmitter));
        NarrowbandReceiverBase *flatReceiver = const_cast<NarrowbandReceiverBase *>(check_and_cast<const NarrowbandReceiverBase *>(receiver));
        flatTransmitter->setCarrierFrequency(carrierFrequency);
        flatReceiver->setCarrierFrequency(carrierFrequency);
        EV << "Changing radio channel from " << channelNumber << " to " << newChannelNumber << ".\n";
        channelNumber = newChannelNumber;
        endReceptionTimer = NULL;
        emit(radioChannelChangedSignal, newChannelNumber);
        emit(listeningChangedSignal, 0);
    }
}

} // namespace physicallayer

} // namespace inet

