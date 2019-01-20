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

#include "inet/physicallayer/base/packetlevel/FlatRadioBase.h"
#include "inet/physicallayer/base/packetlevel/FlatReceiverBase.h"
#include "inet/physicallayer/base/packetlevel/FlatTransmitterBase.h"
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"

namespace inet {

namespace physicallayer {

Define_Module(FlatRadioBase);

FlatRadioBase::FlatRadioBase() :
    NarrowbandRadioBase()
{
}

void FlatRadioBase::handleUpperCommand(cMessage *message)
{
    if (message->getKind() == RADIO_C_CONFIGURE) {
        ConfigureRadioCommand *configureCommand = check_and_cast<ConfigureRadioCommand *>(message->getControlInfo());
        W newPower = configureCommand->getPower();
        if (!std::isnan(newPower.get()))
            setPower(newPower);
        bps newBitrate = configureCommand->getBitrate();
        if (!std::isnan(newBitrate.get()))
            setBitrate(newBitrate);
    }
    NarrowbandRadioBase::handleUpperCommand(message);
}

void FlatRadioBase::setPower(W newPower)
{
    FlatTransmitterBase *flatTransmitter = const_cast<FlatTransmitterBase *>(check_and_cast<const FlatTransmitterBase *>(transmitter));
    flatTransmitter->setPower(newPower);
}

void FlatRadioBase::setBitrate(bps newBitrate)
{
    FlatTransmitterBase *flatTransmitter = const_cast<FlatTransmitterBase *>(check_and_cast<const FlatTransmitterBase *>(transmitter));
    flatTransmitter->setBitrate(newBitrate);
    receptionTimer = nullptr;
}

} // namespace physicallayer

} // namespace inet

