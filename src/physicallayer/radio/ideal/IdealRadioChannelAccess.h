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


#ifndef __INET_IDEALRADIOCHANNELACCESS_H
#define __INET_IDEALRADIOCHANNELACCESS_H


#include "RadioBase.h"
#include "IdealRadioChannel.h"

// Forward declarations
class IdealRadioFrame;


/**
 * This class contains functions that cooperate with IdealRadioChannel.
 * Subscribes to position change and updates cached radio position if
 * position change signal arrived.
 *
 * The radio module has to be derived from this class!
 */
class INET_API IdealRadioChannelAccess : public cSimpleModule, protected cListener
{
  protected:
    static simsignal_t mobilityStateChangedSignal;
    IdealRadioChannel *cc;  // Pointer to the IdealRadioChannel module
    IdealRadioChannel::RadioEntry *myRadioRef;  // Identifies this radio in the IdealRadioChannel module
    cModule *hostModule;    // the host that contains this radio model
    Coord radioPos;  // the physical position of the radio (derived from display string or from mobility models)
    bool positionUpdateArrived;

  public:
    IdealRadioChannelAccess() : cc(NULL), myRadioRef(NULL), hostModule(NULL) {}
    virtual ~IdealRadioChannelAccess();

    /**
     * @brief Called by the signaling mechanism to inform of changes.
     *
     * IdealRadioChannelAccess is subscribed to position changes.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);

  protected:
    /** Sends a message to all radios in range */
    virtual void sendToChannel(IdealRadioFrame *msg);

    virtual cPar& getRadioChannelPar(const char *parName) { return (cc)->par(parName); }
    const Coord& getRadioPosition() const { return radioPos; }
    cModule *getHostModule() const { return hostModule; }

    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
};

#endif

