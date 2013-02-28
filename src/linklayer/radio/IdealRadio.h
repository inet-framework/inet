//
// Copyright (C) 2013 OpenSim Ltd
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
// author: Zoltan Bojthe
//


#ifndef __INET_IDEALRADIO_H
#define __INET_IDEALRADIO_H


#include "INETDefs.h"

#include "IdealAirFrame_m.h"
#include "IdealChannelModelAccess.h"
#include "IPowerControl.h"
#include "RadioState.h"


/**
 * This module implements a full-duplex, collision-free and interference-free radio.
 * It should be used together with IdealWirelessMac.
 *
 * See the NED file for details.
 */
class INET_API IdealRadio : public IdealChannelModelAccess, public IPowerControl
{
  public:
    IdealRadio();
    virtual ~IdealRadio();

    /** Returns the current transmission range */
    virtual int getTransmissionRange() const { return transmissionRange; }

  protected:
    virtual void initialize(int stage);
    virtual void finish();

    virtual void handleMessage(cMessage *msg);

    virtual void handleUpperMsg(cMessage *msg);

    virtual void handleSelfMsg(cMessage *msg);

    virtual void handleCommand(cMessage *msg);

    virtual void handleLowerMsgStart(IdealAirFrame *airframe);

    virtual void handleLowerMsgEnd(IdealAirFrame *airframe);

    /** Sends a message to the upper layer */
    virtual void sendUp(IdealAirFrame *airframe);

    /** Sends a message to the channel */
    virtual void sendDown(IdealAirFrame *airframe);

    /** Encapsulates a MAC frame into an Air Frame */
    virtual IdealAirFrame *encapsulatePacket(cPacket *msg);

    /** Updates the radio state, and also sends a radioState signal */
    virtual void updateRadioState();

    /** Create a new IdealAirFrame */
    virtual IdealAirFrame *createAirFrame() { return new IdealAirFrame(); }

    virtual void updateDisplayString();

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);

    // IPowerControl:
    virtual void enablingInitialization();
    virtual void disablingInitialization();

  protected:
    typedef std::list<cMessage *> RecvBuff;
    RecvBuff recvBuff;

    /** Parameters */
    //@{
    double transmissionRange;           // [meter]
    double bitrate;                     // [bps]
    bool drawCoverage;                  // if true, draw coverage circles
    //@}

    /** @name Gate Ids */
    //@{
    int upperLayerOutGateId;
    int upperLayerInGateId;
    int radioInGateId;
    //@}

    /** Radio State and helper variables */
    //@{
    int concurrentReceives;    // number of current receives
    bool inTransmit;           // is in transmit mode
    RadioState::State rs;
    //@}

    // signals:
    static simsignal_t radioStateSignal; // signaling RadioState::State enum when state changed
};

#endif

