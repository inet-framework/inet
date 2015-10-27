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

#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211TransmitterBase.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ReceiverBase.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211Radio);

simsignal_t Ieee80211Radio::radioChannelChangedSignal = cComponent::registerSignal("radioChannelChanged");

Ieee80211Radio::Ieee80211Radio() :
    FlatRadioBase()
{
}

void Ieee80211Radio::initialize(int stage)
{
    FlatRadioBase::initialize(stage);
    if (stage == INITSTAGE_PHYSICAL_LAYER) {
        int channelNumber = par("channelNumber");
        if (channelNumber != -1)
            setChannelNumber(channelNumber);
    }
}

void Ieee80211Radio::handleUpperCommand(cMessage *message)
{
    if (message->getKind() == RADIO_C_CONFIGURE) {
        FlatRadioBase::handleUpperCommand(message);
        Ieee80211ConfigureRadioCommand *configureCommand = dynamic_cast<Ieee80211ConfigureRadioCommand *>(message->getControlInfo());
        if (configureCommand != nullptr) {
            const char *opMode = configureCommand->getOpMode();
            if (*opMode)
                setModeSet(Ieee80211ModeSet::getModeSet(opMode));
            const Ieee80211ModeSet *modeSet = configureCommand->getModeSet();
            if (modeSet != nullptr)
                setModeSet(modeSet);
            const IIeee80211Mode *mode = configureCommand->getMode();
            if (mode != nullptr)
                setMode(mode);
            IIeee80211Band *band = configureCommand->getBand();
            if (band != nullptr)
                setBand(band);
            Ieee80211Channel *channel = configureCommand->getChannel();
            if (channel != nullptr)
                setChannel(channel);
            int newChannelNumber = configureCommand->getChannelNumber();
            if (newChannelNumber != -1)
                setChannelNumber(newChannelNumber);
        }
    }
    else
        FlatRadioBase::handleUpperCommand(message);
}

void Ieee80211Radio::setModeSet(const Ieee80211ModeSet *modeSet)
{
    Ieee80211TransmitterBase *ieee80211Transmitter = const_cast<Ieee80211TransmitterBase *>(check_and_cast<const Ieee80211TransmitterBase *>(transmitter));
    Ieee80211ReceiverBase *ieee80211Receiver = const_cast<Ieee80211ReceiverBase *>(check_and_cast<const Ieee80211ReceiverBase *>(receiver));
    ieee80211Transmitter->setModeSet(modeSet);
    ieee80211Receiver->setModeSet(modeSet);
    EV << "Changing radio mode set to " << modeSet << endl;
    receptionTimer = nullptr;
    emit(listeningChangedSignal, 0);
}

void Ieee80211Radio::setMode(const IIeee80211Mode *mode)
{
    Ieee80211TransmitterBase *ieee80211Transmitter = const_cast<Ieee80211TransmitterBase *>(check_and_cast<const Ieee80211TransmitterBase *>(transmitter));
    ieee80211Transmitter->setMode(mode);
    EV << "Changing radio mode to " << mode << endl;
    receptionTimer = nullptr;
    emit(listeningChangedSignal, 0);
}

void Ieee80211Radio::setBand(const IIeee80211Band *band)
{
    Ieee80211TransmitterBase *ieee80211Transmitter = const_cast<Ieee80211TransmitterBase *>(check_and_cast<const Ieee80211TransmitterBase *>(transmitter));
    Ieee80211ReceiverBase *ieee80211Receiver = const_cast<Ieee80211ReceiverBase *>(check_and_cast<const Ieee80211ReceiverBase *>(receiver));
    ieee80211Transmitter->setBand(band);
    ieee80211Receiver->setBand(band);
    EV << "Changing radio band to " << band << endl;
    receptionTimer = nullptr;
    emit(listeningChangedSignal, 0);
}

void Ieee80211Radio::setChannel(const Ieee80211Channel *channel)
{
    Ieee80211TransmitterBase *ieee80211Transmitter = const_cast<Ieee80211TransmitterBase *>(check_and_cast<const Ieee80211TransmitterBase *>(transmitter));
    Ieee80211ReceiverBase *ieee80211Receiver = const_cast<Ieee80211ReceiverBase *>(check_and_cast<const Ieee80211ReceiverBase *>(receiver));
    ieee80211Transmitter->setChannel(channel);
    ieee80211Receiver->setChannel(channel);
    EV << "Changing radio channel to " << channel->getChannelNumber() << endl;
    receptionTimer = nullptr;
    emit(radioChannelChangedSignal, channel->getChannelNumber());
    emit(listeningChangedSignal, 0);
}

void Ieee80211Radio::setChannelNumber(int newChannelNumber)
{
    Ieee80211TransmitterBase *ieee80211Transmitter = const_cast<Ieee80211TransmitterBase *>(check_and_cast<const Ieee80211TransmitterBase *>(transmitter));
    Ieee80211ReceiverBase *ieee80211Receiver = const_cast<Ieee80211ReceiverBase *>(check_and_cast<const Ieee80211ReceiverBase *>(receiver));
    ieee80211Transmitter->setChannelNumber(newChannelNumber);
    ieee80211Receiver->setChannelNumber(newChannelNumber);
    EV << "Changing radio channel to " << newChannelNumber << ".\n";
    receptionTimer = nullptr;
    emit(radioChannelChangedSignal, newChannelNumber);
    emit(listeningChangedSignal, 0);
}

} // namespace physicallayer

} // namespace inet

