// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_MESHPAYLOADTRANSMISSION_H
#define INET_MESHPAYLOADTRANSMISSION_H
#include "inet/linklayer/common/MacAddress.h"

namespace inet::ieee80211 {

// Local model observation: one completed on-air unicast payload PPDU, regardless
// of its later ACK outcome. No packet ownership or recovery state is carried.
class INET_API MeshPayloadTransmission : public cObject
{
  public:
    const MacAddress peer;
    const int msduCount;
    MeshPayloadTransmission(MacAddress peer, int msduCount) : peer(peer), msduCount(msduCount) {}
};

} // namespace inet::ieee80211
#endif
