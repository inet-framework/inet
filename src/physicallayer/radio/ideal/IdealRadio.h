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

#ifndef __INET_IDEALRADIO_H
#define __INET_IDEALRADIO_H

#include "IdealRadioFrame.h"
#include "IdealRadioChannelAccess.h"
#include "ILifecycle.h"

/**
 * This module implements a full-duplex, collision-free and interference-free ideal radio.
 *
 * See the NED file for details.
 *
 * author: Zoltan Bojthe, Levente Meszaros
 */
class INET_API IdealRadio : public IdealRadioChannelAccess, public ILifecycle
{
  protected:
    /** Timers */
    //@{
    cMessage *endTransmissionTimer;
    typedef std::list<cMessage *> EndReceptionTimers;
    EndReceptionTimers endReceptionTimers;
    //@}

    /** Parameters */
    //@{
    double transmissionRange;           // [meter]
    double bitrate;                     // [bps]
    bool drawCoverage;
    //@}

  public:
    IdealRadio();
    virtual ~IdealRadio();

    virtual void setRadioMode(RadioMode radioMode);

    virtual void setRadioChannel(int radioChannel);

    virtual int getTransmissionRange() const { return transmissionRange; }

  protected:
    virtual void initialize(int stage);

    virtual void handleMessage(cMessage *message);

    virtual void handleSelfMessage(cMessage *message);

    virtual void handleCommand(cMessage *command);

    virtual void handleUpperFrame(cPacket *frame);

    virtual void handleLowerFrame(IdealRadioFrame *radioFrame);

    virtual void sendUp(IdealRadioFrame *radioFrame);

    virtual void sendDown(IdealRadioFrame *radioFrame);

    virtual IdealRadioFrame *encapsulatePacket(cPacket *packet);

    virtual void cancelAndDeleteEndReceptionTimers();

    virtual void updateRadioChannelState();

    virtual void updateDisplayString();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
};

#endif
