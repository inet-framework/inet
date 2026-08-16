//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/physicallayer/wireless/common/pcap/WirelessPcapCaptureObservationAdapter.h"

#include "inet/common/packet/recorder/PcapCaptureAdapterRegistry.h"
#include "inet/physicallayer/common/Signal.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"

namespace inet {
namespace physicallayer {

Register_Pcap_Capture_Observation_Adapter("wireless", WirelessPcapCaptureObservationAdapter);

std::optional<PcapCaptureObservation> WirelessPcapCaptureObservationAdapter::tryCreateObservation(const cObject *object, Direction direction) const
{
    if (auto signal = dynamic_cast<const Signal *>(object)) {
        auto packet = dynamic_cast<const Packet *>(signal->getEncapsulatedPacket());
        return packet != nullptr ? std::optional<PcapCaptureObservation>(PcapCaptureObservation(packet, direction)) : std::nullopt;
    }
    else if (auto transmission = dynamic_cast<const ITransmission *>(object))
        return PcapCaptureObservation(transmission->getPacket(), direction, transmission);
    else if (auto reception = dynamic_cast<const IReception *>(object))
        return PcapCaptureObservation(reception->getTransmission()->getPacket(), direction, reception->getTransmission(), reception);
    else
        return std::nullopt;
}

} // namespace physicallayer
} // namespace inet
