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

#ifndef __INET_UTIL_INETPCAPFILE_H
#define __INET_UTIL_INETPCAPFILE_H


#include "PcapEntryParserIf.h"
#include "PcapFile.h"

#if NEED_PCAP_FILE_WRITER
#include "InetPcapSerializerIf.h"
#endif
/**
 * Pcap file reader with parser.
 * It read packets from pcap file and parse packets with parser.
 */
class InetPcapFileReader : public PcapFileReader
{
  protected:
    uint32 sec0;                // pcap time of first packet (secs)
    uint32 usec0;               // pcap time of first packet (microsecs)
    PcapEntryParserIf *parser;  // pointer to parser

  public:
    /** ctor */
    InetPcapFileReader() : sec0(0), usec0(0), parser(NULL) {}

    /** destructor */
    ~InetPcapFileReader() { delete parser; }

    /**
     * set pcap packet parser
     *
     * parserName: name of the parser class
     */
    void setParser(const char* parserName);

    /** open pcap file */
    void open(const char* filename);

    /**
     * read next packet from pcap
     *
     * output parameter stime: relative time of returned packet based on first packet
     * returns next packet from pcap in a new cPacket or NULL when don't parsed it
     */
    cPacket* read(simtime_t &stime);
};

#if NEED_PCAP_FILE_WRITER
/**
 * Pcap file writer with cPacket serializer.
 * It serialize packets with serializer and write packets to pcap file
 * with current simtime.
 */
class InetPcapFileWriter : public PcapFileWriter
{
  protected:
    InetPcapSerializerIf *serializer;   // pointer to serializer

  public:
    /** ctor */
    InetPcapFileWriter() : serializer(NULL) {}

    /** destructor */
    ~InetPcapFileWriter() { delete serializer; }

    /**
     * set cPacket serializerr
     *
     * serializerName: name of the serializer class
     */
    void setSerializer(const char* serializerName);

    /** serialize packet and write to pcap file */
    void write(cPacket *packet);
};
#endif

#endif //__INET_UTIL_INETPCAPFILE_H
