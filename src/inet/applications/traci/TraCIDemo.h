//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef __INET_TRACIDEMO_H
#define __INET_TRACIDEMO_H

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/mobility/group/TraCIMobility.h"

namespace inet {

/**
 * Small IVC Demo
 * Documentation for these modules is at http://veins.car2x.org/
 */
class TraCIDemo : public cSimpleModule, protected cListener, public ILifecycle
{
  protected:
    // state
    TraCIMobility *traci;
    UDPSocket socket;
    bool sentMessage;
    static simsignal_t mobilityStateChangedSignal;

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

    void setupLowerLayer();
    virtual void handleSelfMsg(cMessage *apMsg);
    virtual void handleLowerMsg(cMessage *apMsg);

    virtual void sendMessage();
    virtual void handlePositionUpdate();

  public:
    TraCIDemo() { traci = NULL; }
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
};

} // namespace inet

#endif // ifndef __INET_TRACIDEMO_H

