/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __INET_ETHERLLC_H
#define __INET_ETHERLLC_H

#include "INETDefs.h"

// Forward declarations:
class EtherFrameWithLLC;


/**
 * Implements the LLC sub-layer of the Datalink Layer in Ethernet networks
 */
class INET_API EtherLLC : public cSimpleModule
{
  protected:
    int seqNum;
    typedef std::map<int,int> DsapToPortMap;  // DSAP registration table
    DsapToPortMap dsapToPort;  // DSAP registration table

    // statistics
    long dsapsRegistered;       // number DSAPs (higher layers) registered
    long totalFromHigherLayer;  // total number of packets received from higher layer
    long totalFromMAC;          // total number of frames received from MAC
    long totalPassedUp;         // total number of packets passed up to higher layer
    long droppedUnknownDSAP;    // frames dropped because no such DSAP was registered here
    static simsignal_t dsapSignal;
    static simsignal_t encapPkSignal;
    static simsignal_t decapPkSignal;
    static simsignal_t passedUpPkSignal;
    static simsignal_t droppedPkUnknownDSAPSignal;
    static simsignal_t pauseSentSignal;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    virtual void processPacketFromHigherLayer(cPacket *msg);
    virtual void processFrameFromMAC(EtherFrameWithLLC *msg);
    virtual void handleRegisterSAP(cMessage *msg);
    virtual void handleDeregisterSAP(cMessage *msg);
    virtual void handleSendPause(cMessage *msg);
    virtual int findPortForSAP(int sap);

    // utility function
    virtual void updateDisplayString();
};

#endif
