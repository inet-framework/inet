//
// Copyright (C) OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_PCAPCAPTURE_H
#define __INET_PCAPCAPTURE_H

#include <pcap.h>
#include "inet/common/scheduler/RealTimeScheduler.h"
#include "inet/linklayer/base/MacBase.h"

namespace inet {

/**
 * TODO
 *
 * Requires RealTimeScheduler to be configured as scheduler in omnetpp.ini.
 *
 * See NED file for more details.
 */
class INET_API PcapCapture : public cSimpleModule, public RealTimeScheduler::ICallback
{
  protected:
    const char *device = nullptr;
    RealTimeScheduler *realTimeScheduler = nullptr;

    // statistics
    int numRcvd = 0;

    // state
    pcap_t *pd = nullptr;
    int fd = -1;
    int datalink = -1;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void openPcap(const char *device, const char *filter);
    virtual bool notify(int fd) override;

    static void ext_packet_handler(u_char *usermod, const struct pcap_pkthdr *hdr, const u_char *bytes);

  public:
    virtual ~PcapCapture();

};

} // namespace inet

#endif // ifndef __INET_PCAPCAPTURE_H

