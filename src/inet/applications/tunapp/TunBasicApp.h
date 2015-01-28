//
// Copyright (C) 2015 Irene Ruengeler
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_TUNBASICAPP_H
#define __INET_TUNBASICAPP_H

#include "inet/transportlayer/contract/udp/UDPSocket.h"

namespace inet {

class INET_API TunBasicApp : public cSimpleModule
{
    public:
        virtual ~TunBasicApp();
    private:
        unsigned int packetsReceivedViaUdp;
        unsigned int packetsToSend;
        unsigned int packetsReceivedViaTun;
        unsigned int packetsSentOverUdp;
        int localPort;
        int remotePort;
        L3Address localAddress;
        L3Address remoteAddress;
        UDPSocket socket;
        cMessage *timeMsg;

        static simsignal_t sentPkSignal;
        static simsignal_t rcvdPkSignal;

    protected:
        void initialize(int stage) override;
        void handleMessage(cMessage *msg) override;
        void finish() override;
        void handleTimer(cMessage *msg);

        void sendPacket();
};

} // namespace inet
# endif
