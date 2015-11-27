//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2011 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_PCAPRECORDER_H
#define __INET_PCAPRECORDER_H

#include "inet/common/INETDefs.h"

#include "inet/common/packet/PacketDump.h"
#include "inet/common/packet/PcapDump.h"

namespace inet {

/**
 * Dumps every packet using the PcapDump and PacketDump classes
 */
class INET_API PcapRecorder : public cSimpleModule, protected cListener
{
  protected:
    typedef std::map<simsignal_t, bool> SignalList;
    SignalList signalList;
    PacketDump packetDumper;
    PcapDump pcapDumper;
    unsigned int snaplen = 0;
    bool dumpBadFrames = false;

  public:
    PcapRecorder();
    ~PcapRecorder();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;
    virtual void recordPacket(cPacket *msg, bool l2r);
};

} // namespace inet

#endif // ifndef __INET_PCAPRECORDER_H

