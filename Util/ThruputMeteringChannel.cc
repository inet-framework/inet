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

#include "ThruputMeteringChannel.h"

Register_Class(ThruputMeteringChannel);

ThruputMeteringChannel::ThruputMeteringChannel(const char *name) : cBasicChannel(name)
{
    fmtp = NULL;
    batchSize = 10;    // packets
    maxInterval = 0.1; // seconds

    numPackets = 0;
    numBits = 0;

    intvlStartTime = intvlLastPkTime = 0;
    intvlNumPackets = intvlNumBits = 0;
}

ThruputMeteringChannel::ThruputMeteringChannel(const ThruputMeteringChannel& ch) : cBasicChannel()
{
    setName(ch.name());
    operator=(ch);
    cArray& parlist = _parList();
    fmtp = (cPar *)parlist.get("format");
}

ThruputMeteringChannel::~ThruputMeteringChannel()
{
}

ThruputMeteringChannel& ThruputMeteringChannel::operator=(const ThruputMeteringChannel& ch)
{
    if (this==&ch) return *this;
    cBasicChannel::operator=(ch);
    numPackets = ch.numPackets;
    numBits = ch.numBits;
    return *this;
}

cPar& ThruputMeteringChannel::addPar(const char *s)
{
    cPar *p = &cBasicChannel::addPar(s);
    if (!opp_strcmp(s,"format"))
        fmtp = p;
    return *p;
}

cPar& ThruputMeteringChannel::addPar(cPar *p)
{
    cBasicChannel::addPar(p);
    const char *s = p->name();
    if (!opp_strcmp(s,"format"))
        fmtp = p;
    return *p;
}

bool ThruputMeteringChannel::deliver(cMessage *msg, simtime_t t)
{
    bool ret = cBasicChannel::deliver(msg, t);

    // count packets and bits
    numPackets++;
    numBits += msg->length();

    // packet should be counted to new interval
    if (intvlNumPackets >= batchSize || t-intvlStartTime >= maxInterval)
        beginNewInterval(t);

    intvlNumPackets++;
    intvlNumBits += msg->length();
    intvlLastPkTime = t;

    // update display string
    updateDisplay();

    return ret;
}

void ThruputMeteringChannel::beginNewInterval(simtime_t now)
{
    simtime_t duration = now - intvlStartTime;

    // record measurements
    currentBitPerSec = intvlNumBits/duration;
    currentPkPerSec = intvlNumPackets/duration;

    // restart counters
    intvlStartTime = now;
    intvlNumPackets = intvlNumBits = 0;
}

void ThruputMeteringChannel::updateDisplay()
{
    // retrieve format string
    const char *fmt = fmtp ? fmtp->stringValue() : "B";

    // produce label, based on format string
    char buf[200];
    char *p = buf;
    simtime_t tt = transmissionFinishes();
    if (tt==0) tt = simulation.simTime();
    double bps = (tt==0) ? 0 : numBits/tt;
    double bytes;
    for (const char *fp = fmt; *fp && buf+200-p>20; fp++)
    {
        switch (*fp)
        {
            case 'N': // number of packets
                p += sprintf(p, "%ld", numPackets);
                break;
            case 'V': // volume (in bytes)
                bytes = floor(numBits/8);
                if (bytes<1024)
                    p += sprintf(p, "%gB", bytes);
                else if (bytes<1024*1024)
                    p += sprintf(p, "%.3gKB", bytes/1024);
                else
                    p += sprintf(p, "%.3gMB", bytes/1024/1024);
                break;

            case 'p': // current packet/sec
                p += sprintf(p, "%.3gpps", currentPkPerSec);
                break;
            case 'b': // current bandwidth
                if (currentBitPerSec<1000000)
                    p += sprintf(p, "%.3gk", currentBitPerSec/1000);
                else
                    p += sprintf(p, "%.3gM", currentBitPerSec/1000000);
                break;
            case 'u': // current channel utilization (%)
                if (datarate()==0)
                    p += sprintf(p, "n/a");
                else
                    p += sprintf(p, "%.3g%%", currentBitPerSec/datarate()*100.0);
                break;

            case 'P': // average packet/sec on [0,now)
                p += sprintf(p, "%.3gpps", tt==0 ? 0 : numPackets/tt);
                break;
            case 'B': // average bandwidth on [0,now)
                if (bps<1000000)
                    p += sprintf(p, "%.3gk", bps/1000);
                else
                    p += sprintf(p, "%.3gM", bps/1000000);
                break;
            case 'U': // average channel utilization (%) on [0,now)
                if (datarate()==0)
                    p += sprintf(p, "n/a");
                else
                    p += sprintf(p, "%.3g%%", bps/datarate()*100.0);
                break;
            default:
                *p++ = *fp;
        }
    }
    *p = '\0';

    // display label
    fromGate()->displayString().setTagArg("t", 0, buf);
}

