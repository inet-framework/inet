//
// Copyright (C) 2004 Andras Varga
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __PLAINTCP_H
#define __PLAINTCP_H

#include <omnetpp.h>
#include "TCPAlgorithm.h"


/**
 * State variables for PlainTCP.
 */
class PlainTCPStateVariables : public TCPStateVariables
{
  public:
    //...
};


/**
 * Includes basic TCP algorithms: retransmission, PERSIST timer, keep-alive,
 * delayed acknowledge.
 */
class PlainTCP : public TCPAlgorithm
{
  protected:
    cMessage *rexmitTimer;
    cMessage *persistTimer;
    cMessage *delayedAckTimer;
    cMessage *keepAliveTimer;

    PlainTCPStateVariables *state;

  public:
    /**
     * Ctor.
     */
    PlainTCP();

    /**
     * Virtual dtor.
     */
    virtual ~PlainTCP();

    /**
     * Create and return a PlainTCPStateVariables object.
     */
    virtual TCPStateVariables *createStateVariables();

    /**
     * Process REXMIT, PERSIST, DELAYED-ACK and KEEP-ALIVE timers.
     */
    virtual void processTimer(cMessage *timer, TCPEventCode& event);

    virtual void sendCommandInvoked();

    virtual void receiveSeqChanged();

    virtual void receivedAck();

    virtual void receivedAckForDataNotYetSent(uint32 seq);

};

#endif


