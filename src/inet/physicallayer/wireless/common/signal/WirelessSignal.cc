//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/signal/WirelessSignal.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"

namespace inet {
namespace physicallayer {

WirelessSignal::WirelessSignal(const ITransmission *transmission) :
    transmissionId(transmission->getId()),
    transmission(transmission),
    radioMedium(transmission->getTransmitter()->getMedium())
{
}

WirelessSignal::WirelessSignal(const WirelessSignal& other) :
    Signal(other),
    transmissionId(other.transmissionId),
    transmission(nullptr),
    radioMedium(other.radioMedium)
{
}

std::ostream& WirelessSignal::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream << (cPacket *)this;
}

const IRadio *WirelessSignal::getTransmitter() const
{
    auto transmission = getTransmission();
    return transmission != nullptr ? transmission->getTransmitter() : nullptr;
}

const IRadio *WirelessSignal::getReceiver() const
{
    if (receiver == nullptr)
        receiver = check_and_cast<const IRadio *>(getArrivalModule());
    return receiver;
}

const ITransmission *WirelessSignal::getTransmission() const
{
    if (!isDup())
        return transmission;
    else
        return radioMedium->getTransmission(transmissionId);
}

const IArrival *WirelessSignal::getArrival() const
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

const IListening *WirelessSignal::getListening() const
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

const IReception *WirelessSignal::getReception() const
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

