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

#ifndef __INET_MACRELAYUNIT_H
#define __INET_MACRELAYUNIT_H

#include "inet/common/INETDefs.h"

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/linklayer/ethernet/switch/IMACAddressTable.h"

namespace inet {

class EtherFrame;

class INET_API MACRelayUnit : public cSimpleModule, public ILifecycle
{
  public:
    MACRelayUnit();

  protected:
    IMACAddressTable *addressTable;
    int numPorts;

    // Parameters for statistics collection
    long numProcessedFrames;
    long numDiscardedFrames;

    bool isOperational;    // for lifecycle

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    /**
     * Updates address table with source address, determines output port
     * and sends out (or broadcasts) frame on ports. Includes calls to
     * updateTableWithAddress() and getPortForAddress().
     *
     * The message pointer should not be referenced any more after this call.
     */
    virtual void handleAndDispatchFrame(EtherFrame *frame);

    /**
     * Utility function: sends the frame on all ports except inputport.
     * The message pointer should not be referenced any more after this call.
     */
    virtual void broadcastFrame(EtherFrame *frame, int inputport);

    /**
     * Calls handleIncomingFrame() for frames arrived from outside,
     * and processFrame() for self messages.
     */
    virtual void handleMessage(cMessage *msg);

    /**
     * Writes statistics.
     */
    virtual void finish();

    // for lifecycle:

  public:
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

  protected:
    virtual void start();
    virtual void stop();
};

} // namespace inet

#endif // ifndef __INET_MACRELAYUNIT_H

