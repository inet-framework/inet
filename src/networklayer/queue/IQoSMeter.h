//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim
// Copyright (C) 2005 Andras Varga
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


#ifndef __INET_IQOSMETER_H
#define __INET_IQOSMETER_H

#include <omnetpp.h>
#include "INETDefs.h"

/**
 * Abstract interface for QoS meters, used in QoS queues.
 * A QoS meter measures the temporal properties (e.g., rates) of a traffic stream
 * selected by a classifier. The instantaneous state of the meter may be used to
 * affect the operation of a marker, shaper, or dropper, and/or may be used for
 * accounting and measurement purposes.
 *
 */
class INET_API IQoSMeter : public cPolymorphic
{
  public:

    /**
     * Initialize the meter (e.g., the size of token bucket).
     */
    virtual void initialize();

    /**
     * The method should return the result of metering for the given packet.
     */
    virtual int meterPacket(cMessage *msg) = 0;
};

#endif
