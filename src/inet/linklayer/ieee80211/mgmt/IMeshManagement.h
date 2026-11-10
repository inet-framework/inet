// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_IMESHMANAGEMENT_H
#define __INET_IMESHMANAGEMENT_H

#include "inet/common/INETDefs.h"

namespace inet {
class Packet;
namespace physicallayer { class IIeee80211Mode; }
namespace ieee80211 {

class INET_API IMeshManagement
{
  public:
    virtual ~IMeshManagement() = default;
    virtual void prepareManagementTransmission(Packet *packet, simtime_t ifs,
            const physicallayer::IIeee80211Mode *mode) = 0;
    virtual void managementTransmissionComplete(const Packet *packet) = 0;
    virtual bool isBeaconPending() const = 0;
    virtual bool isMeshManagementActive() const = 0;
    virtual simtime_t getNextTbtt() const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
