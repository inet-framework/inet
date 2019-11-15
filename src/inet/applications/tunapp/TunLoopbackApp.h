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

#ifndef __INET_TUNLOOPBACKAPP_H
#define __INET_TUNLOOPBACKAPP_H

#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/linklayer/tun/TunSocket.h"

namespace inet {

class INET_API TunLoopbackApp : public cSimpleModule, public LifecycleUnsupported
{
    protected:
        const char *tunInterface = nullptr;

        unsigned int packetsSent = 0;
        unsigned int packetsReceived = 0;

        TunSocket tunSocket;

    protected:
        void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        void handleMessage(cMessage *msg) override;
        void finish() override;
};

} // namespace inet

# endif

