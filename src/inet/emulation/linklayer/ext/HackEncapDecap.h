//
// Copyright (C) OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_HACKENCAPDECAP_H
#define __INET_HACKENCAPDECAP_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/base/MacBase.h"

namespace inet {

/**
 * TODO
 *
 * See NED file for more details.
 */
class INET_API HackEncapDecap : public MacBase
{
  protected:
    B headerLength = B(-1);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    // MacBase functions
    InterfaceEntry *createInterfaceEntry() override {}
    virtual void flushQueue() override {}
    virtual void clearQueue() override {}
    virtual bool isUpperMsg(cMessage *msg) override { return msg->arrivedOn("upperLayerIn"); }
};

} // namespace inet

#endif // ifndef __INET_HACKENCAPDECAP_H

