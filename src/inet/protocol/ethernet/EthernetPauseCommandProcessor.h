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

#ifndef __INET_EthernetPauseCommandProcessor_H
#define __INET_EthernetPauseCommandProcessor_H

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"

namespace inet {

class INET_API EthernetPauseCommandProcessor : public cSimpleModule
{
  protected:
    int seqNum = 0;
    static simsignal_t pauseSentSignal;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

  protected:
    void handleSendPause(Request *msg, Ieee802PauseCommand *etherctrl);
};

} // namespace inet

#endif // ifndef __INET_EthernetPauseCommandProcessor_H

