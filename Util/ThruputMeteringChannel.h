//
// Copyright (C) 2005 Andras Varga
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

#ifndef __THRUPUTMETERINGCHANNEL_H
#define __THRUPUTMETERINGCHANNEL_H

#include <omnetpp.h>
#include "INETDefs.h"

/**
 * A cBasicChannel extended with throughput calculation.
 */
class SIM_API ThruputMeteringChannel : public cBasicChannel
{
  protected:
    long count;

  public:
    /**
     * Constructor.
     */
    explicit ThruputMeteringChannel(const char *name=NULL);

    /**
     * Copy constructor.
     */
    ThruputMeteringChannel(const ThruputMeteringChannel& ch);

    /**
     * Destructor.
     */
    virtual ~ThruputMeteringChannel();

    /**
     * Assignment
     */
    ThruputMeteringChannel& operator=(const ThruputMeteringChannel& ch);

    /**
     * Creates and returns an exact copy of this object.
     * See cObject for more details.
     */
    virtual cPolymorphic *dup() const  {return new ThruputMeteringChannel(*this);}

    /**
     * Performs bit error rate, delay and transmission time modelling.
     */
    virtual bool deliver(cMessage *msg, simtime_t at);
};

#endif


