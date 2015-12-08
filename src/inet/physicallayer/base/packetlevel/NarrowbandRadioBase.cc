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

#include "inet/physicallayer/base/packetlevel/NarrowbandRadioBase.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandTransmitterBase.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandReceiverBase.h"
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"

namespace inet {

namespace physicallayer {

Define_Module(NarrowbandRadioBase);

NarrowbandRadioBase::NarrowbandRadioBase() :
    Radio()
{
}

void NarrowbandRadioBase::handleUpperCommand(cMessage *message)
{
    if (message->getKind() == RADIO_C_CONFIGURE) {
        Radio::handleUpperCommand(message);
        ConfigureRadioCommand *configureCommand = check_and_cast<ConfigureRadioCommand *>(message->getControlInfo());
        const IModulation *newModulation = configureCommand->getModulation();
        if (newModulation != nullptr)
            setModulation(newModulation);
        Hz newCarrierFrequency = configureCommand->getCarrierFrequency();
        if (!std::isnan(newCarrierFrequency.get()))
            setCarrierFrequency(newCarrierFrequency);
        Hz newBandwidth = configureCommand->getBandwidth();
        if (!std::isnan(newBandwidth.get()))
            setBandwidth(newBandwidth);
    }
    else
        Radio::handleUpperCommand(message);
}

void NarrowbandRadioBase::setModulation(const IModulation *newModulation)
{
    NarrowbandTransmitterBase *narrowbandTransmitter = const_cast<NarrowbandTransmitterBase *>(check_and_cast<const NarrowbandTransmitterBase *>(transmitter));
    narrowbandTransmitter->setModulation(newModulation);
    NarrowbandReceiverBase *narrowbandReceiver = const_cast<NarrowbandReceiverBase *>(check_and_cast<const NarrowbandReceiverBase *>(receiver));
    narrowbandReceiver->setModulation(newModulation);
}

void NarrowbandRadioBase::setCarrierFrequency(Hz newCarrierFrequency)
{
    NarrowbandTransmitterBase *narrowbandTransmitter = const_cast<NarrowbandTransmitterBase *>(check_and_cast<const NarrowbandTransmitterBase *>(transmitter));
    narrowbandTransmitter->setCarrierFrequency(newCarrierFrequency);
    NarrowbandReceiverBase *narrowbandReceiver = const_cast<NarrowbandReceiverBase *>(check_and_cast<const NarrowbandReceiverBase *>(receiver));
    narrowbandReceiver->setCarrierFrequency(newCarrierFrequency);
}

void NarrowbandRadioBase::setBandwidth(Hz newBandwidth)
{
    NarrowbandTransmitterBase *narrowbandTransmitter = const_cast<NarrowbandTransmitterBase *>(check_and_cast<const NarrowbandTransmitterBase *>(transmitter));
    narrowbandTransmitter->setBandwidth(newBandwidth);
    NarrowbandReceiverBase *narrowbandReceiver = const_cast<NarrowbandReceiverBase *>(check_and_cast<const NarrowbandReceiverBase *>(receiver));
    narrowbandReceiver->setBandwidth(newBandwidth);
    receptionTimer = nullptr;
}

} // namespace physicallayer

} // namespace inet

