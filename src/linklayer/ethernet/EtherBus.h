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
#include "INETDefs.h"

// Direction of frame travel on bus; also used as selfmessage kind
#define UPSTREAM        0
#define DOWNSTREAM      1

/**
 * Implements the bus which connects hosts, switches and other LAN entities on an Ethernet LAN.
 */
class INET_API EtherBus : public cSimpleModule
{
  protected:
    /**
     * Implements the physical locations on the bus where each
     * network entity is connected to on the bus
     */
    struct BusTap
    {
        int id;                         // which tap is this
        double position;                // Physical location of where each entity is connected to on the bus, (physical location of the tap on the bus)
        simtime_t propagationDelay[2];  // Propagation delays to the adjacent tap points on the bus: 0:upstream, 1:downstream
    };

    double  propagationSpeed;  // propagation speed of electrical signals through copper

    BusTap *tap;  // physical locations of where the hosts is connected to the bus
    int taps;     // number of tap points on the bus

    long numMessages;             // number of messages handled

  public:
    EtherBus();
    virtual ~EtherBus();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage*);
    virtual void finish();

    // tokenize string containing space-separated numbers into the array
    virtual void tokenize(const char *str, std::vector<double>& array);
};

#endif


