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


#ifndef __INET_IDEALCHANNELMODELACCESS_H
#define __INET_IDEALCHANNELMODELACCESS_H


#include "INETDefs.h"

#include "BasicModule.h"
#include "IdealChannelModel.h"

// Forward declarations
class IdealAirFrame;


/**
 * This class contains functions that cooperate with IdealChannelModel.
 * Subscribes to position change and updates cached radio position if
 * position change signal arrived.
 *
 * The radio module has to be derived from this class!
 */
class INET_API IdealChannelModelAccess : public BasicModule, protected cListener
{
  protected:
    static simsignal_t mobilityStateChangedSignal;
    IdealChannelModel *cc;  // Pointer to the IdealChannelModel module
    IdealChannelModel::RadioEntry *myRadioRef;  // Identifies this radio in the IdealChannelModel module
    cModule *hostModule;    // the host that contains this radio model
    Coord radioPos;  // the physical position of the radio (derived from display string or from mobility models)
    bool positionUpdateArrived;

  public:
    IdealChannelModelAccess() : cc(NULL), myRadioRef(NULL), hostModule(NULL) {}
    virtual ~IdealChannelModelAccess();

    /**
     * @brief Called by the signaling mechanism to inform of changes.
     *
     * IdealChannelModelAccess is subscribed to position changes.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);

  protected:
    /** Sends a message to all radios in range */
    virtual void sendToChannel(IdealAirFrame *msg);

    virtual cPar& getChannelControlPar(const char *parName) { return (cc)->par(parName); }
    const Coord& getRadioPosition() const { return radioPos; }
    cModule *getHostModule() const { return hostModule; }

    /** Register with ChannelControl and subscribe to hostPos*/
    virtual void initialize(int stage);
    virtual int numInitStages() const { return 3; }
};

#endif

