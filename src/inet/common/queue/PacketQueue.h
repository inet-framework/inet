//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_PACKETQUEUE_H
#define __INET_PACKETQUEUE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API PacketQueue : public cPacketQueue
{
    protected:
        int maxPacketLength = -1;
        int64_t maxBitLength = -1;

    protected:
        void checkInsertion(cPacket *packet);

    public:
        PacketQueue(const char *name = nullptr);

        virtual void insert(cPacket *packet);
        virtual void insertBefore(cPacket *where, cPacket *packet);
        virtual void insertAfter(cPacket *where, cPacket *packet);

        virtual int getMaxPacketLength() { return maxPacketLength; }
        virtual void setMaxPacketLength(int maxPacketLength) { this->maxPacketLength = maxPacketLength; }

        virtual int64_t getMaxBitLength() { return maxBitLength; }
        virtual void setMaxBitLength(int64_t maxBitLength) { this->maxBitLength = maxBitLength; }
};

} // namespace inet

#endif // ifndef __INET_PACKETQUEUE_H
