//
// Copyright (C) 2011 Andras Varga
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

#ifndef __INET_UDPECHOAPP_H
#define __INET_UDPECHOAPP_H

#include "inet/common/INETDefs.h"

#include "inet/applications/common/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"

namespace inet {

/**
 * UDP application. See NED for more info.
 */
class UDPEchoApp : public ApplicationBase
{
  protected:
    UDPSocket socket;
    int numEchoed;    // just for WATCH
    static simsignal_t pkSignal;

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessageWhenUp(cMessage *msg);
    virtual void finish();
    virtual void updateDisplay();

    virtual bool handleNodeStart(IDoneCallback *doneCallback);
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback);
    virtual void handleNodeCrash();
};

} // namespace inet

#endif // ifndef __INET_UDPECHOAPP_H

