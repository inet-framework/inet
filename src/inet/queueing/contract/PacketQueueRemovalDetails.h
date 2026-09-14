// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_PACKETQUEUEREMOVALDETAILS_H
#define __INET_PACKETQUEUEREMOVALDETAILS_H

#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {
namespace queueing {

class INET_API PacketQueueRemovalDetails : public cObject
{
  protected:
    IPacketQueue::PacketRemovalReason reason;

  public:
    explicit PacketQueueRemovalDetails(IPacketQueue::PacketRemovalReason reason) : reason(reason) {}
    IPacketQueue::PacketRemovalReason getReason() const { return reason; }
};

} // namespace queueing
} // namespace inet

#endif
