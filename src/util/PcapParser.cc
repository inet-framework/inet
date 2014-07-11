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

#include "PcapParser.h"

#include <EthernetSerializer.h>

Define_Module(PcapParser);

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
}

void PcapParser::readRecord()
{
    struct pcaprec_hdr recordHeader;
    if(fread(&recordHeader, sizeof(recordHeader), 1, pcapFile))
    {
        EV_DEBUG<< "Timestamp seconds: " << recordHeader.ts_sec <<endl;
        EV_DEBUG<< "Timestamp microseconds: " << recordHeader.ts_usec <<endl;
        EV_DEBUG<< "Original length: " << recordHeader.orig_len <<endl;
        EV_DEBUG<< "Included length: " << recordHeader.orig_len <<endl;

        uint8 buf[MAXBUFLENGTH];
        memset((void*)&buf, 0, sizeof(buf));

        fread(&buf, recordHeader.orig_len, 1, pcapFile);

        nextPkt = new EthernetIIFrame;
        EthernetSerializer().parse(buf, recordHeader.incl_len, (EthernetIIFrame *)nextPkt);
        scheduleAt(SimTime(recordHeader.ts_sec, SIMTIME_S) + SimTime(recordHeader.ts_usec, SIMTIME_US), nextMsgTimer);
    }
}
