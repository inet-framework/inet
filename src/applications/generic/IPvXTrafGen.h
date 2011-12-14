//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


#ifndef __INET_IPTRAFGEN_H
#define __INET_IPTRAFGEN_H

#include <vector>

#include "INETDefs.h"

#include "IPvXAddress.h"
#include "IPvXTrafSink.h"


/**
 * IP traffic generator application. See NED for more info.
 */
class INET_API IPvXTrafGen : public IPvXTrafSink
{
  protected:
    int protocol;
    int numPackets;
    simtime_t stopTime;
    std::vector<IPvXAddress> destAddresses;
    cPar *packetLengthPar;  // volatile packetLength parameter

    static int counter; // counter for generating a global number for each packet

    int numSent;
    static simsignal_t sentPkSignal;

  protected:
    // chooses random destination address
    virtual IPvXAddress chooseDestAddr();
    virtual void sendPacket();

    virtual int numInitStages() const {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
};

#endif

