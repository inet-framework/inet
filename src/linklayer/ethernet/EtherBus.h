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

#ifndef __INET_ETHERBUS_H
#define __INET_ETHERBUS_H

#include <vector>

#include "inet/common/INETDefs.h"

namespace inet {

// Direction of frame travel on bus; also used as selfmessage kind
#define UPSTREAM      0
#define DOWNSTREAM    1

/**
 * Implements the shared coaxial cable in classic Ethernet. See the NED file
 * for more description.
 */
class INET_API EtherBus : public cSimpleModule, cListener
{
  protected:
    /**
     * Implements the physical locations on the bus where each
     * network entity is connected to on the bus
     */
    struct BusTap
    {
        int id;    // which tap is this
        double position;    // Physical location of where each entity is connected to on the bus, (physical location of the tap on the bus)
        simtime_t propagationDelay[2];    // Propagation delays to the adjacent tap points on the bus: 0:upstream, 1:downstream
    };

    // configuration
    double propagationSpeed;    // propagation speed of electrical signals through copper
    BusTap *tap;    // array of BusTaps: physical locations taps where that connect stations to the bus
    int numTaps;    // number of tap points on the bus
    int inputGateBaseId;    // gate id of ethg$i[0]
    int outputGateBaseId;    // gate id of ethg$o[0]

    // state
    bool dataratesDiffer;

    // statistics
    long numMessages;    // number of messages handled

  public:
    EtherBus();
    virtual ~EtherBus();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);

    virtual void checkConnections(bool errorWhenAsymmetric);
};

} // namespace inet

#endif // ifndef __INET_ETHERBUS_H

