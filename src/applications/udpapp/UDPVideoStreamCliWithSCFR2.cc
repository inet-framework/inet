//
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


//
// based on the video streaming app of the similar name by Johnny Lai
//


#include "UDPVideoStreamCliWithSCFR2.h"
#include "IPAddressResolver.h"


Define_Module(UDPVideoStreamCliWithSCFR2);


void UDPVideoStreamCliWithSCFR2::initialize()
{
    // initialize module parameters
    simtime_t startTime = par("startTime");

    // initialize statistics
    fragmentStartSignal = registerSignal("fragmentStart");
    eedSignal = registerSignal("endToEndDelay");
    timestampSignal = registerSignal("timestamp");

    // schedule the start of video streaming
    if (startTime>=0)
        scheduleAt(startTime, new cMessage("UDPVideoStreamStart"));
}

void UDPVideoStreamCliWithSCFR2::receiveStream(cPacket *msg)
{
    EV << "Video stream packet:\n";
    printPacket(msg);

    bool fragmentStart = ((UDPVideoStreamPacket *)msg)->getFragmentStart();
    emit(fragmentStartSignal, int(fragmentStart));
    emit(eedSignal, simTime() - msg->getCreationTime());
    emit(timestampSignal, ((UDPVideoStreamPacket *)msg)->getTimestamp());

    delete msg;
}
