//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

#include "TokenBucketMeter.h"
#include "DiffservUtil.h"

using namespace DiffservUtil;

Define_Module(TokenBucketMeter);

void TokenBucketMeter::initialize(int stage)
{
    if (stage == 0)
    {
        numRcvd = 0;
        numRed = 0;
        WATCH(numRcvd);
        WATCH(numRed);
    }
    else if (stage == 2)
    {
        const char *cirStr = par("cir");
        CIR = parseInformationRate(cirStr, "cir", *this, 0);
        CBS = 8 * (int)par("cbs");
        colorAwareMode = par("colorAwareMode");
        Tc = CBS;
        lastUpdateTime = simTime();
    }
}

void TokenBucketMeter::handleMessage(cMessage *msg)
{
    cPacket *packet = findIPDatagramInPacket(check_and_cast<cPacket*>(msg));
    if (!packet)
        error("TokenBucketMeter received a packet that does not encapsulate an IP datagram.");

    numRcvd++;
    int color = meterPacket(packet);
    if (color == GREEN)
    {
        send(packet, "greenOut");
    }
    else
    {
        numRed++;
        send(packet, "redOut");
    }

    if (ev.isGUI())
    {
        char buf[50] = "";
        if (numRcvd>0) sprintf(buf+strlen(buf), "rcvd: %d ", numRcvd);
        if (numRed>0) sprintf(buf+strlen(buf), "red:%d ", numRed);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

int TokenBucketMeter::meterPacket(cPacket *packet)
{
    // update token buckets
    simtime_t currentTime = simTime();
    long numTokens = (long)(SIMTIME_DBL(currentTime-lastUpdateTime) * CIR);
    lastUpdateTime = currentTime;
    if (Tc + numTokens <= CBS)
        Tc += numTokens;
    else
        Tc = CBS;

    // update meter state
    int oldColor = colorAwareMode ? getColor(packet) : -1;
    int newColor;
    int packetSizeInBits = packet->getBitLength();
    if (oldColor <= GREEN && Tc - packetSizeInBits >= 0)
    {
        Tc -= packetSizeInBits;
        newColor = GREEN;
    }
    else
        newColor = RED;

    setColor(packet, newColor);
    return newColor;
}
