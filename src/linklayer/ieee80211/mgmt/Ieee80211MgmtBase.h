//
// Copyright (C) 2006 Andras Varga
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

#ifndef IEEE80211_MGMT_BASE_H
#define IEEE80211_MGMT_BASE_H

#include "INETDefs.h"

#include "MACAddress.h"
#include "PassiveQueueBase.h"
#include "NotificationBoard.h"
#include "Ieee80211Frame_m.h"
#include "Ieee80211MgmtFrames_m.h"
#include "ILifecycle.h"


/**
 * Abstract base class for 802.11 infrastructure mode management components.
 * Performs queueing for MAC, and dispatching incoming frames by frame type.
 * Also keeps some simple statistics (frame counts).
 *
 * @author Andras Varga
 */
class INET_API Ieee80211MgmtBase : public PassiveQueueBase, public ILifecycle, public INotifiable
{
  protected:
    // configuration
    int frameCapacity;
    MACAddress myAddress;
    bool isOperational;     // for lifecycle

    // state
    cQueue dataQueue; // queue for data frames
    cQueue mgmtQueue; // queue for management frames (higher priority than data frames)

    // statistics
    long numDataFramesReceived;
    long numMgmtFramesReceived;
    long numMgmtFramesDropped;

    // queue statistics
    static simsignal_t dataQueueLenSignal;

  protected:
    virtual int numInitStages() const {return 2;}
    virtual void initialize(int);

    /** Dispatches incoming messages to handleTimer(), handleUpperMessage() or processFrame(). */
    virtual void handleMessage(cMessage *msg);

    /** Should be redefined to deal with self-messages */
    virtual void handleTimer(cMessage *frame) = 0;

    /** Should be redefined to encapsulate and enqueue msgs from higher layers */
    virtual void handleUpperMessage(cPacket *msg) = 0;

    /** Should be redefined to handle commands from the "agent" (if present) */
    virtual void handleCommand(int msgkind, cObject *ctrl) = 0;

    /** Utility method for implementing handleUpperMessage(): gives the message to PassiveQueueBase */
    virtual void sendOrEnqueue(cPacket *frame);

    /** Redefined from PassiveQueueBase. */
    virtual cMessage *enqueue(cMessage *msg);

    /** Redefined from PassiveQueueBase. */
    virtual cMessage *dequeue();

    /** Redefined from IPassiveQueue. */
    virtual bool isEmpty();

    /** Redefined from PassiveQueueBase: send message to MAC */
    virtual void sendOut(cMessage *msg);

    /** Utility method to dispose of an unhandled frame */
    virtual void dropManagementFrame(Ieee80211ManagementFrame *frame);

    /** Utility method: sends the packet to the upper layer */
    virtual void sendUp(cMessage *msg);

    /** Dispatch to frame processing methods according to frame type */
    virtual void processFrame(Ieee80211DataOrMgmtFrame *frame);

    /** @name Processing of different frame types */
    //@{
    virtual void handleDataFrame(Ieee80211DataFrame *frame) = 0;
    virtual void handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame) = 0;
    virtual void handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame) = 0;
    virtual void handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame) = 0;
    virtual void handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame) = 0;
    virtual void handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame) = 0;
    virtual void handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame) = 0;
    virtual void handleDisassociationFrame(Ieee80211DisassociationFrame *frame) = 0;
    virtual void handleBeaconFrame(Ieee80211BeaconFrame *frame) = 0;
    virtual void handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame) = 0;
    virtual void handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame) = 0;
    //@}

    /** lifecycle support */
    //@{
  protected:
    virtual void start();
    virtual void stop();

  public:
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
    //@}
};

#endif
