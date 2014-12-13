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

#include "inet/physicallayer/base/NarrowbandRadioBase.h"
#include "inet/physicallayer/base/NarrowbandTransmitterBase.h"
#include "inet/physicallayer/base/NarrowbandReceiverBase.h"
#include "inet/physicallayer/contract/RadioControlInfo_m.h"

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
        ConfigureRadioCommand *configureCommand = check_and_cast<ConfigureRadioCommand *>(message->getControlInfo());
        Radio::handleUpperCommand(message);
        W newPower = configureCommand->getPower();
        if (!isNaN(newPower.get()))
            setPower(newPower);
        bps newBitrate = configureCommand->getBitrate();
        if (!isNaN(newBitrate.get()))
            setBitrate(newBitrate);
    }
    else
        Radio::handleUpperCommand(message);
}

void NarrowbandRadioBase::setPower(W newPower)
{
    NarrowbandTransmitterBase *narrowbandTransmitter = const_cast<NarrowbandTransmitterBase *>(check_and_cast<const NarrowbandTransmitterBase *>(transmitter));
    narrowbandTransmitter->setPower(newPower);
}

void NarrowbandRadioBase::setBitrate(bps newBitrate)
{
    NarrowbandTransmitterBase *narrowbandTransmitter = const_cast<NarrowbandTransmitterBase *>(check_and_cast<const NarrowbandTransmitterBase *>(transmitter));
    narrowbandTransmitter->setBitrate(newBitrate);
    endReceptionTimer = nullptr;
}

} // namespace physicallayer

} // namespace inet

