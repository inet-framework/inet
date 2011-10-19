//
// Copyright (C) 2011 OpenSim Ltd
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
// @author Zoltan Bojthe
//

#include <stdio.h>

#include <omnetpp.h>

#ifdef HAVE_PCAP
// prevent pcap.h to redefine int8_t,... types on Windows
#include "bsdint.h"
#define HAVE_U_INT8_T
#define HAVE_U_INT16_T
#define HAVE_U_INT32_T
#define HAVE_U_INT64_T
#include <pcap.h>
#endif

#include "PcapFile.h"

#include "IPv4Datagram.h"
#include "IPv4Serializer.h"

PcapFileReader::~PcapFileReader()
{
    close();
}

void PcapFileReader::open(const char* filename)
{
    char errbuf[PCAP_ERRBUF_SIZE];

    pcap = pcap_open_offline(filename, errbuf);

    if(!pcap)
        throw cRuntimeError("Pcap open file '%s' error: %s\n", filename, errbuf);
    fgetpos(pcap_file(pcap), &pos0);
}

const void* PcapFileReader::read(uint32_t &sec, uint32_t &usec, uint32_t &capLen, uint32_t& origLen)
{
    if (!pcap)
        return NULL;

    struct pcap_pkthdr ph;
    const u_char* buf = pcap_next(pcap, &ph);
    if (buf)
    {
        sec = ph.ts.tv_sec;
        usec = ph.ts.tv_usec;
        capLen = ph.caplen;
        origLen = ph.len;
    }
    return buf;
}

bool PcapFileReader::eof()
{
    return !pcap || feof(pcap_file(pcap));
}

void PcapFileReader::restart()
{
    if (pcap)
        fsetpos(pcap_file(pcap), &pos0);
}

void PcapFileReader::close()
{
    if (pcap)
    {
        pcap_close(pcap);
        pcap = NULL;
    }
}

// Enable it in PcapFile.h when you want to write pcap file
#if NEED_PCAP_FILE_WRITER
PcapFileWriter::PcapFileWriter() :
    pcap(NULL), pcapDumper(NULL)
{
}

PcapFileWriter::~PcapFileWriter()
{
    close();
}

void PcapFileWriter::close()
{
    if (pcapDumper)
    {
        pcap_dump_close(pcapDumper);
        pcapDumper = NULL;
    }
    if (pcap)
    {
        pcap_close(pcap);
        pcap = NULL;
    }
}

void PcapFileWriter::open(const char* filename, unsigned int snaplen)
{
    char errbuf[PCAP_ERRBUF_SIZE];

    pcap = pcap_open_offline(filename, errbuf);

    if(!pcap)
        throw cRuntimeError("Pcap open dumpfile '%s' error: %s\n", filename, errbuf);
    pcapDumper = pcap_dump_open(pcap,filename);
    if(!pcapDumper)
    {
        throw cRuntimeError("Pcap dump open dumpfile '%s' error: %s\n", filename, pcap_geterr(pcap));
    }
    snapLen = snaplen;
    if (0 != pcap_set_snaplen(pcap, snapLen))
        throw cRuntimeError("Pcap open dumpfile '%s' error: Invalid snaplen=%d\n", filename, snaplen);
}

void PcapFileWriter::write(uint32_t sec, uint32_t usec, const void *buff, uint32_t capLen, uint32_t fullLen)
{
    struct pcap_pkthdr ph;
    ph.ts.tv_sec = sec;
    ph.ts.tv_usec = usec;
    ph.len = fullLen;
    ph.caplen = std::min(std::min(snapLen, capLen), fullLen);

    pcap_dump((unsigned char *)pcapDumper, &ph, (const unsigned char *)buff);
}
#endif

