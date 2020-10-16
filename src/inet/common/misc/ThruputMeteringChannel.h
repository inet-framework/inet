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

#ifndef __INET_THRUPUTMETERINGCHANNEL_H
#define __INET_THRUPUTMETERINGCHANNEL_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * A cDatarateChannel extended with throughput calculation. Values
 * get displayed on the link, using the connection's "t=" display
 * string tag.
 *
 * The display can be customized with the "format" attribute.
 * In the format string, the following characters will get expanded:
 *   - 'N': number of packets
 *   - 'V': volume (in bytes)
 *   - 'p': current packet/sec
 *   - 'b': current bandwidth
 *   - 'u': current channel utilization (%)
 *   - 'P': average packet/sec on [0,now)
 *   - 'B': average bandwidth on [0,now)
 *   - 'U': average channel utilization (%) on [0,now)
 * Other characters are copied verbatim.
 *
 * "Current" actually means the last measurement interval, which is
 * 10 packets or 0.1s, whichever comes first.
 *
 * PROBLEM: display only gets updated if there's traffic! (For example, a
 * high pk/sec value might stay displayed even when the link becomes idle!)
 */
class INET_API ThruputMeteringChannel : public cDatarateChannel
{
  protected:
    // configuration
    bool displayAsTooltip;
    const char *fmt;    // display format
    unsigned int batchSize;    // number of packets in a batch
    simtime_t maxInterval;    // max length of measurement interval (measurement ends
                              // if either batchSize or maxInterval is reached, whichever
                              // is reached first)

    // global statistics
    long numPackets;
    double numBits;    // double to avoid overflow

    // current measurement interval
    simtime_t intvlStartTime;
    simtime_t intvlLastPkTime;
    unsigned long intvlNumPackets;
    unsigned long intvlNumBits;

    // reading from last interval
    double currentBitPerSec;
    double currentPkPerSec;

  protected:
    virtual void beginNewInterval(simtime_t now);
    virtual void refreshDisplay() const override;

  public:
    /**
     * Constructor.
     */
    explicit ThruputMeteringChannel(const char *name = nullptr);

    /**
     * Copy constructor.
     */
//    ThruputMeteringChannel(const ThruputMeteringChannel& ch);

    /**
     * Destructor.
     */
    virtual ~ThruputMeteringChannel();

    /**
     * Assignment
     */
//    ThruputMeteringChannel& operator=(const ThruputMeteringChannel& ch);

    /**
     * Creates and returns an exact copy of this object.
     * See cObject for more details.
     */
//    virtual ThruputMeteringChannel *dup() const {return new ThruputMeteringChannel(*this);}

    /**
     * Add parameters and initialize the stat variables
     */
    virtual void initialize() override;

    /**
     * Adds statistics and live display to the channel.
     */
    OMNETPP6_CODE(virtual Result processMessage(cMessage *msg, const SendOptions& options, simtime_t t) override;)
    OMNETPP5_CODE(virtual void processMessage(cMessage *msg, simtime_t t, result_t& result) override;)
};

} // namespace inet

#endif // ifndef __INET_THRUPUTMETERINGCHANNEL_H

