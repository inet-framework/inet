//
// Copyright (C) 2011 OpenSim Ltd
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
// @author Zoltan Bojthe
//

#ifndef __INET_PCAPTRAFFICGENERATOR_H_
#define __INET_PCAPTRAFFICGENERATOR_H_

#include <omnetpp.h>

#include "InetPcapFile.h"


/**
 * Generate traffic from a pcap file
 */
class PcapTrafficGenerator : public cSimpleModule
{
  protected:
    bool enabled;
    InetPcapFileReader pcapFile;
    // parameters:
    simtime_t timeShift;    // simtime of first pcap entry
    simtime_t endTime;      // simtime of last sent packet, 0 is infinity
    simtime_t repeatGap;    // wait repeatGap time after last packet of pcap, and restart sending with first packet of pcap
                            // the 0 or smaller value disable restarting

  public:
    PcapTrafficGenerator() : enabled(false) {}

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void scheduleNextPacket();
};

#endif
