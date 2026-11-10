// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_IMESHDATAMAC_H
#define __INET_IMESHDATAMAC_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"

namespace inet {
namespace ieee80211 {

class INET_API IMeshDataMac
{
  public:
    virtual ~IMeshDataMac() = default;
    // Takes a complete resolved MPDU directly into the normal originator path.
    virtual void sendMeshData(Packet *packet) = 0;
    // Takes a decapsulated LLC MSDU; endpoint addresses are not hop addresses.
    virtual void deliverMeshMsdu(Packet *packet, MacAddress source, MacAddress destination, int tid) = 0;
};

} // namespace ieee80211
} // namespace inet
#endif
