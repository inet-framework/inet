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

#include <errno.h>

#include "inet/common/serializer/PcapParser.h"

#include "inet/common/serializer/headerserializers/SerializerUtil.h"

#include "inet/common/serializer/headerserializers/ethernet/EthernetSerializer.h"
#include "inet/common/serializer/headerserializers/ieee80211/Ieee80211Serializer.h"

namespace inet {
namespace serializer {

Define_Module(PcapParser);

using namespace serializer;

simsignal_t PcapParser::packetSentSignal = registerSignal("packetSent");
PcapParser::PcapParser()
{
    pcapFile = NULL;
    nextMsgTimer = NULL;
    nextPkt = NULL;
}

PcapParser::~PcapParser()
{
    closePcap();
    cancelAndDelete(nextMsgTimer);
}

void PcapParser::initialize()
{
    nextMsgTimer = new cMessage("nextMsg");
    openPcap(par("pcapFile"));

    readHeader();
    readRecord();
}

void PcapParser::handleMessage(cMessage *msg)
{
    EV_DEBUG<< "Packet sent: " << nextPkt << endl;
    emit(packetSentSignal, nextPkt);
    send(nextPkt,"packageOut");
    readRecord();
}

void PcapParser::openPcap(const char* filename)
{

    if (!filename || !filename[0])
        throw cRuntimeError("Cannot open pcap file: file name is empty");

    pcapFile = fopen(filename, "rb");

    if (!pcapFile)
        throw cRuntimeError("Cannot open pcap file [%s] for writing: %s", filename, strerror(errno));
}

void PcapParser::closePcap()
{   if(pcapFile)
        fclose(pcapFile);
}

void PcapParser::readHeader()
{
    fread(&fileHeader, sizeof(fileHeader), 1, pcapFile);
    if (fileHeader.magic != 0xa1b2c3d4)
    {
        fileHeader.version_major = swapByteOrder16(fileHeader.version_major);
        fileHeader.version_minor = swapByteOrder16(fileHeader.version_minor);
        fileHeader.thiszone = swapByteOrder32(fileHeader.thiszone);
        fileHeader.sigfigs = swapByteOrder32(fileHeader.sigfigs);
        fileHeader.snaplen = swapByteOrder32(fileHeader.snaplen);
        fileHeader.network = swapByteOrder32(fileHeader.network);
    }
}

void PcapParser::readRecord()
{
    struct pcaprec_hdr recordHeader;
    if(fread(&recordHeader, sizeof(recordHeader), 1, pcapFile))
    {
        if (fileHeader.magic != 0xa1b2c3d4)
        {
            recordHeader.ts_sec = swapByteOrder32(recordHeader.ts_sec);
            recordHeader.ts_usec = swapByteOrder32(recordHeader.ts_usec);
            recordHeader.orig_len = swapByteOrder32(recordHeader.orig_len);
            recordHeader.incl_len = swapByteOrder32(recordHeader.incl_len);
        }

        EV_DEBUG<< "Timestamp seconds: " << recordHeader.ts_sec <<endl;
        EV_DEBUG<< "Timestamp microseconds: " << recordHeader.ts_usec <<endl;
        EV_DEBUG<< "Original length: " << recordHeader.orig_len <<endl;
        EV_DEBUG<< "Included length: " << recordHeader.incl_len <<endl;

        uint8 buf[MAXBUFLENGTH];
        memset((void*)&buf, 0, sizeof(buf));

        fread(&buf, recordHeader.orig_len, 1, pcapFile);

        switch(fileHeader.network)
        {
            case LINKTYPE_ETHERNET:
                nextPkt = EthernetSerializer().parse(buf, recordHeader.incl_len);
                scheduleAt(SimTime(recordHeader.ts_sec, SIMTIME_S) + SimTime(recordHeader.ts_usec, SIMTIME_US), nextMsgTimer);
                break;

            case LINKTYPE_IEEE801_11:
                nextPkt = Ieee80211Serializer().parse(buf, recordHeader.incl_len);
                scheduleAt(SimTime(recordHeader.ts_sec, SIMTIME_S) + SimTime(recordHeader.ts_usec, SIMTIME_US), nextMsgTimer);
                break;
        }
    }
}

} // namespace serializer
} // namespace inet
