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

#include "inet/physicallayer/common/packetlevel/Signal.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"

namespace inet {
namespace physicallayer {

Signal::Signal(const ITransmission *transmission) :
    transmissionId(transmission->getId()),
    transmission(transmission),
    radioMedium(transmission->getTransmitter()->getMedium())
{
}

Signal::Signal(const Signal& other) :
    cPacket(other),
    transmissionId(other.transmissionId),
    transmission(nullptr),
    radioMedium(other.radioMedium)
{
}

std::ostream& Signal::printToStream(std::ostream& stream, int level) const
{
    return stream << (cPacket *)this;
}

const IRadio *Signal::getTransmitter() const
{
    auto transmission = getTransmission();
    return transmission != nullptr ? transmission->getTransmitter() : nullptr;
}

const IRadio *Signal::getReceiver() const
{
    if (receiver == nullptr)
        receiver = check_and_cast<const IRadio *>(getArrivalModule());
    return receiver;
}

const ITransmission *Signal::getTransmission() const
{
    if (!isDup())
        return transmission;
    else
        return radioMedium->getTransmission(transmissionId);
}

const IArrival *Signal::getArrival() const
{
    if (!isDup()) {
        if (arrival == nullptr)
            arrival = radioMedium->getArrival(getReceiver(), transmission);
        return arrival;
    }
    else {
        auto transmission = getTransmission();
        return transmission != nullptr ? radioMedium->getArrival(getReceiver(), transmission) : nullptr;
    }
}

const IListening *Signal::getListening() const
{
    if (!isDup()) {
        if (listening == nullptr)
            listening = radioMedium->getListening(getReceiver(), transmission);
        return listening;
    }
    else {
        auto transmission = getTransmission();
        return transmission != nullptr ? radioMedium->getListening(getReceiver(), transmission) : nullptr;
    }
}

const IReception *Signal::getReception() const
{
    if (!isDup()) {
        if (reception == nullptr)
            reception = radioMedium->getReception(getReceiver(), transmission);
        return reception;
    }
    else {
        auto transmission = getTransmission();
        return transmission != nullptr ? radioMedium->getReception(getReceiver(), transmission) : nullptr;
    }
}

} // namespace physicallayer
} // namespace inet

