//
// Copyright (C) 2015 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

// This file is based on the Ppp.h of INET written by Andras Varga.

#ifndef __INET_TUNINTERFACE_H
#define __INET_TUNINTERFACE_H

#include "inet/linklayer/base/MacBase.h"

namespace inet {

class INET_API Tun : public MacBase
{
    protected:
        std::vector<int> socketIds;

    protected:
        InterfaceEntry *createInterfaceEntry() override;
        virtual void flushQueue() override;
        virtual void clearQueue() override;
        virtual bool isUpperMsg(cMessage *message) override { return message->arrivedOn("upperLayerIn"); }

    public:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void handleMessage(cMessage *message) override;
};

} // namespace inet

#endif // ifndef __INET_TUNINTERFACE_H

