//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETBUFFER_H
#define __INET_IPACKETBUFFER_H

#include "inet/common/packet/Packet.h"
#include "inet/queueing/contract/IPacketCollection.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet buffers.
 */
class INET_API IPacketBuffer : public virtual IPacketCollection
{
  public:
    class INET_API ICallback {
      public:
        /**
         * Notifies the packet owner about the packet being removed from the buffer.
         * The packet is never nullptr.
         */
        virtual void handlePacketRemoved(Packet *packet) = 0;
    };

  public:
    virtual ~IPacketBuffer() {}

    /**
     * Adds the packet to the buffer.
     */
    virtual void addPacket(Packet *packet) = 0;

    /**
     * Removes the packet from the buffer.
     */
    virtual void removePacket(Packet *packet) = 0;
};

} // namespace queueing
} // namespace inet

#endif

