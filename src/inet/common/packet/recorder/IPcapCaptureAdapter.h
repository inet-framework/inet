//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPCAPCAPTUREADAPTER_H
#define __INET_IPCAPCAPTUREADAPTER_H

#include <optional>
#include <vector>

#include "inet/common/DirectionTag_m.h"
#include "inet/common/IPrintableObject.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/recorder/IPcapWriter.h"

namespace inet {

class INET_API PcapCaptureObservation
{
  public:
    const Packet *const packet;
    const Direction direction;
    const IPrintableObject *const transmission;
    const IPrintableObject *const reception;

    PcapCaptureObservation(const Packet *packet, Direction direction, const IPrintableObject *transmission = nullptr, const IPrintableObject *reception = nullptr) :
        packet(packet), direction(direction), transmission(transmission), reception(reception) {}
};

class INET_API PcapCaptureRecord
{
  protected:
    std::vector<uint8_t> prefix;

  public:
    const b frontOffset;
    const b backOffset;

    PcapCaptureRecord(b frontOffset, b backOffset, std::vector<uint8_t> prefix = {}) :
        prefix(std::move(prefix)), frontOffset(frontOffset), backOffset(backOffset) {}

    const std::vector<uint8_t>& getPrefix() const { return prefix; }
};

class INET_API IPcapCaptureAdapter
{
  public:
    virtual ~IPcapCaptureAdapter() {}
    virtual PcapLinkType getLinkType() const = 0;
    virtual std::optional<std::pair<b, b>> tryResolvePacket(const Packet *, b, b) const { return std::nullopt; }
    virtual std::vector<PcapCaptureRecord> createRecords(const PcapCaptureObservation& observation, b frontOffset, b backOffset) const = 0;
};

class INET_API IPcapCaptureObservationAdapter
{
  public:
    virtual ~IPcapCaptureObservationAdapter() {}
    virtual std::optional<PcapCaptureObservation> tryCreateObservation(const cObject *object, Direction direction) const = 0;
};

} // namespace inet

#endif
