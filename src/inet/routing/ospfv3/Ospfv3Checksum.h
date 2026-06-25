//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OSPFV3CHECKSUM_H
#define __INET_OSPFV3CHECKSUM_H

#include "inet/common/checksum/ChecksumMode_m.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
#include "inet/routing/ospfv3/Ospfv3Packet_m.h"

namespace inet {
namespace ospfv3 {

// Sets the OSPFv3 packet "Checksum" header field according to checksumMode. For CHECKSUM_COMPUTED
// it is the Internet checksum over the IPv6 upper-layer pseudo-header (source/destination address,
// upper-layer packet length, next header = OSPF) followed by the entire packet, per RFC 5340 2.5.
// (Computing the real value requires the addresses, so unlike a plain chunk serializer this is
// done by the module at send time; the serializer just writes the resulting value.)
INET_API void setOspfChecksum(const Ptr<Ospfv3Packet>& ospfPacket, ChecksumMode checksumMode,
        const Ipv6Address& srcAddress, const Ipv6Address& destAddress);

// Sets the LSA "LS Checksum" header field according to checksumMode. For CHECKSUM_COMPUTED it is
// the Fletcher checksum over the LSA contents excepting the LS Age field, per RFC 2328 12.1.7.
INET_API void setLsaChecksum(Ospfv3Lsa& lsa, ChecksumMode checksumMode);

} // namespace ospfv3
} // namespace inet

#endif
