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

#ifndef __INET_PCAPPARSER_H_
#define __INET_PCAPPARSER_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"

#include "inet/common/serializer/pcap.h"

namespace inet {
namespace serializer {

class PcapParser : public cSimpleModule
{
    protected:
        FILE *pcapFile;
        struct pcap_hdr fileHeader;
        cMessage* nextMsgTimer;
        cPacket *nextPkt;

        static simsignal_t packetSentSignal;

    public:
        PcapParser();
        ~PcapParser();

    protected:
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
        virtual void openPcap(const char* filename);
        virtual void closePcap();
        virtual void readHeader();
        virtual void readRecord();

};

} // namespace serializer
} // namespace inet

#endif
