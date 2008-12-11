//
// Copyright (C) 1992-2004 Andras Varga
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

#ifndef __INET_INET_SINK_H
#define __INET_INET_SINK_H

#include "INETDefs.h"

/**
 * A module that just deletes every packet it receives, and collects
 * basic statistics (packet count, bit count, packet rate, bit rate).
 */
class INET_API Sink : public cSimpleModule
{
  protected:
    int numPackets;
    long numBits;
    double throughput; // bit/sec
    double packetPerSec;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

#endif


