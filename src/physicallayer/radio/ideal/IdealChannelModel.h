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

#ifndef __INET_IDEALCHANNELMODEL_H
#define __INET_IDEALCHANNELMODEL_H


#include "INETDefs.h"

#include "Coord.h"

// Forward declarations
class IdealAirFrame;
class IdealRadio;


/**
 * This class represent an ideal channel model (aka channelControl)
 *
 * Stores infos about all registered radios.
 * Forward messages to all other radios in max transmission range
 */
class INET_API IdealChannelModel : public cSimpleModule
{
  public:
    struct RadioEntry
    {
        cModule *radioModule;   // the module that registered this radio interface
        cGate *radioInGate;     // gate on host module used to receive airframes
        Coord pos;              // cached radio position
        bool isActive;          // radio module is active
    };

  protected:
    typedef std::list<RadioEntry> RadioList;
    RadioList radios;    // list of registered radios

    friend std::ostream& operator<<(std::ostream&, const RadioEntry&);

    /** the biggest transmission range in the network.*/
    double maxTransmissionRange;

  protected:
    /** Reads init parameters and calculates a maximal interference distance*/
    virtual void initialize();

    /** Returns the "handle" of a previously registered radio. The pointer to the registering (radio) module must be provided */
    virtual RadioEntry *lookupRadio(cModule *radioModule);

    /** recalculate the largest transmission range in the network.*/
    virtual void recalculateMaxTransmissionRange();

  public:
    IdealChannelModel();
    virtual ~IdealChannelModel();

    /** Registers the given radio. If radioInGate==NULL, the "radioIn" gate is assumed */
    virtual RadioEntry * registerRadio(cModule *radioModule, cGate *radioInGate = NULL);

    /** Unregisters the given radio */
    virtual void unregisterRadio(RadioEntry *r);

    /** To be called when the host moved; updates proximity info */
    virtual void setRadioPosition(RadioEntry *r, const Coord& pos);

    /** Called from IdealChannelModelAccess, to transmit a frame to the radios in range, on the frame's channel */
    virtual void sendToChannel(RadioEntry * srcRadio, IdealAirFrame *airFrame);

    /** Disable the reception in the reference module */
    virtual void disableReception(RadioEntry *r) { r->isActive = false; };

    /** Enable the reception in the reference module */
    virtual void enableReception(RadioEntry *r) { r->isActive = true; };

    /** returns speed of signal in meter/sec */
    virtual double getSignalSpeed() { return SPEED_OF_LIGHT; }
};

#endif      // __INET_IDEALCHANNELMODEL_H

