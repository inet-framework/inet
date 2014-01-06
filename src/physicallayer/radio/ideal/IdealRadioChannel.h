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

#ifndef __INET_IDEALRADIOCHANNEL_H
#define __INET_IDEALRADIOCHANNEL_H

#include "INETDefs.h"
#include "Coord.h"
#include "IRadio.h"
#include "RadioChannelBase.h"
#include "IdealRadioFrame.h"

/**
 * This class represent an ideal radio channel.
 *
 * Stores infos about all registered radios.
 * Forward messages to all other radios in max transmission range
 *
 * author: Zoltan Bojthe, Levente Meszaros
 */
class INET_API IdealRadioChannel : public RadioChannelBase
{
  public:
    struct RadioEntry
    {
        cModule *radioModule;   // the module that registered this radio interface
        IRadio *radio;          // the radio interface
    };

  protected:
    typedef std::list<RadioEntry> RadioList;
    RadioList radios;    // list of registered radios

    friend std::ostream& operator<<(std::ostream&, const RadioEntry&);

    /** the biggest transmission range in the network.*/
    double maxTransmissionRange;

  protected:
    /** Reads init parameters and calculates a maximal interference distance*/
    virtual void initialize(int stage);

    /** Returns the "handle" of a previously registered radio. The pointer to the registering (radio) module must be provided */
    virtual RadioEntry *lookupRadio(cModule *radioModule);

    /** recalculate the largest transmission range in the network.*/
    virtual void recalculateMaxTransmissionRange();

  public:
    IdealRadioChannel() : maxTransmissionRange(-1) { }
    virtual ~IdealRadioChannel() { }

    /** Registers the given radio */
    virtual RadioEntry * registerRadio(cModule *radioModule);

    /** Unregisters the given radio */
    virtual void unregisterRadio(RadioEntry *radioEntry);

    /** Called from IdealRadioChannelAccess, to transmit a frame to the radios in range, on the frame's channel */
    virtual void sendToChannel(RadioEntry *radioEntry, IdealRadioFrame *radioFrame);
};

#endif
