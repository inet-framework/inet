//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <fstream>

#include "inet/common/INETDefs.h"

#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/linklayer/common/MacAddress.h"

namespace inet {

class INET_API PacketLoggerChannel : public cDatarateChannel
{
  protected:
    long int counter;
    std::ofstream logfile;

  public:
    explicit PacketLoggerChannel(const char *name = NULL) : cDatarateChannel(name) { counter = 0; }
    virtual cChannel::Result processMessage(cMessage *msg, const SendOptions& options, simtime_t t) override;

  protected:
    virtual void initialize() override;
    void finish() override;
};


Register_Class(PacketLoggerChannel);

void PacketLoggerChannel::initialize()
{
    EV << "PacketLogger initialize()\n";
    cDatarateChannel::initialize();
    const char *logfilename = par("logfile");
    if (*logfilename)
    {
        logfile.open(logfilename, std::ios::out | std::ios::trunc);
        if (!logfile.is_open())
            throw cRuntimeError("logfile '%s' open failed", logfilename);
    }
    counter = 0;
}

cChannel::Result PacketLoggerChannel::processMessage(cMessage *msg, const SendOptions& options, simtime_t t)
{
    EV << "PacketLogger processMessage()\n";
    cChannel::Result result = cDatarateChannel::processMessage(msg, options, t);

    const char *status = "";
    if (options.transmissionId_ == -1) {
        counter++;
    }
    else if (!options.isUpdate) {
        status = ":start";
        counter++;
    }
    else {
        status = (options.remainingDuration == SIMTIME_ZERO) ? ":end" : ":update";
    }

    if (logfile.is_open())
    {
        logfile << '#' << counter << ':' << t.raw() << ": '" << msg->getName() << status << "' (" << msg->getClassName() << ") sent:" << msg->getSendingTime().raw();
        cPacket* pk = dynamic_cast<cPacket *>(msg);
        if (pk)
            logfile << " (" << pk->getByteLength() << " byte)";
        logfile << " discard:" << result.discard << ", delay:" << result.delay.raw() << ", duration:" << result.duration.raw();
        logfile << endl;
    }
    return result;
}

void PacketLoggerChannel::finish()
{
    EV << "PacketLogger finish()\n";
    logfile.close();
}

} // namespace inet

