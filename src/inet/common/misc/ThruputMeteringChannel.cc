//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/misc/ThruputMeteringChannel.h"

namespace inet {

Register_Class(ThruputMeteringChannel);

ThruputMeteringChannel::ThruputMeteringChannel(const char *name) : cDatarateChannel(name)
{
    displayAsTooltip = false;
    fmt = nullptr;
    batchSize = 10; // packets
    maxInterval = 0.1; // seconds

    numPackets = 0;
    numBits = 0;

    intvlStartTime = intvlLastPkTime = 0;
    intvlNumPackets = intvlNumBits = 0;

    currentBitPerSec = currentPkPerSec = 0;
}

ThruputMeteringChannel::~ThruputMeteringChannel()
{
}

//ThruputMeteringChannel& ThruputMeteringChannel::operator=(const ThruputMeteringChannel& ch)
//{
//    if (this==&ch) return *this;
//    cDatarateChannel::operator=(ch);
//    numPackets = ch.numPackets;
//    numBits = ch.numBits;
//    fmt = ch.fmt;
//    return *this;
//}

void ThruputMeteringChannel::initialize()
{
    cDatarateChannel::initialize();
    displayAsTooltip = par("displayAsTooltip");
    fmt = par("thruputDisplayFormat");
}

cChannel::Result ThruputMeteringChannel::processMessage(cMessage *msg, const SendOptions& options, simtime_t t)
{
    cChannel::Result result = cDatarateChannel::processMessage(msg, options, t);

    cPacket *pkt = dynamic_cast<cPacket *>(msg);
    // TODO handle disabled state (show with different style?/color? or print "disabled"?)
    if (!pkt || !fmt || *fmt == 0 || result.discard)
        return result;

    // count packets and bits
    numPackets++;
    numBits += pkt->getBitLength();

    // packet should be counted to new interval
    if (intvlNumPackets >= batchSize || t - intvlStartTime >= maxInterval)
        beginNewInterval(t);

    intvlNumPackets++;
    intvlNumBits += pkt->getBitLength();
    intvlLastPkTime = t;
    return result;
}

void ThruputMeteringChannel::beginNewInterval(simtime_t now)
{
    simtime_t duration = now - intvlStartTime;

    // record measurements
    currentBitPerSec = intvlNumBits / duration;
    currentPkPerSec = intvlNumPackets / duration;

    // restart counters
    intvlStartTime = now;
    intvlNumPackets = intvlNumBits = 0;
}

void ThruputMeteringChannel::refreshDisplay() const
{
    // produce label, based on format string
    char buf[200];
    char *p = buf;
    simtime_t tt = getTransmissionFinishTime();
    if (tt == 0)
        tt = simTime();
    double bps = (tt == 0) ? 0 : numBits / tt;
    double bytes;
    for (const char *fp = fmt; *fp && buf + 200 - p > 20; fp++) {
        switch (*fp) {
            case 'N': // number of packets
                p += sprintf(p, "%ld", numPackets);
                break;

            case 'V': // volume (in bytes)
                bytes = floor(numBits / 8);
                if (bytes < 1024)
                    p += sprintf(p, "%gB", bytes);
                else if (bytes < 1024 * 1024)
                    p += sprintf(p, "%.3gKiB", bytes / 1024);
                else
                    p += sprintf(p, "%.3gMiB", bytes / 1024 / 1024);
                break;

            case 'p': // current packet/sec
                p += sprintf(p, "%.3gpps", currentPkPerSec);
                break;

            case 'b': // current bandwidth
                if (currentBitPerSec < 1000000)
                    p += sprintf(p, "%.3gk", currentBitPerSec / 1000);
                else
                    p += sprintf(p, "%.3gM", currentBitPerSec / 1000000);
                break;

            case 'u': // current channel utilization (%)
                if (getDatarate() == 0)
                    p += sprintf(p, "n/a");
                else
                    p += sprintf(p, "%.3g%%", currentBitPerSec / getDatarate() * 100.0);
                break;

            case 'P': // average packet/sec on [0,now)
                p += sprintf(p, "%.3gpps", tt == 0 ? 0 : numPackets / tt);
                break;

            case 'B': // average bandwidth on [0,now)
                if (bps < 1000000)
                    p += sprintf(p, "%.3gk", bps / 1000);
                else
                    p += sprintf(p, "%.3gM", bps / 1000000);
                break;

            case 'U': // average channel utilization (%) on [0,now)
                if (getDatarate() == 0)
                    p += sprintf(p, "n/a");
                else
                    p += sprintf(p, "%.3g%%", bps / getDatarate() * 100.0);
                break;

            default:
                *p++ = *fp;
                break;
        }
    }
    *p = '\0';

    // display label
    auto srcGate = getSourceGate();
    if (srcGate && srcGate->getChannel())
        srcGate->getDisplayString().setTagArg(displayAsTooltip ? "tt" : "t", 0, buf);
}

} // namespace inet

