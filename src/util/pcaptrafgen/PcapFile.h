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

#ifndef __INET_UTIL_PCAPFILE_H
#define __INET_UTIL_PCAPFILE_H


#include "INETDefs.h"

#ifndef HAVE_PCAP
#error "Need the PCAP library and set the HAVE_PCAP flag in src/makefrag!"
#endif

// Foreign declarations
#ifndef lib_pcap_pcap_h
typedef void *pcap_t;
typedef void *pcap_dumper_t;
#endif


// Enable it when you want to write pcap files
#define NEED_PCAP_FILE_WRITER   0

#if NEED_PCAP_FILE_WRITER
/**
 * pcap file writer class.
 *
 * This class uses pcap library for write data to pcap file.
 */
class PcapFileWriter
{
  protected:
    pcap_t *pcap;
    pcap_dumper_t *pcapDumper;
    uint32  snapLen;
  public:
    PcapFileWriter();
    ~PcapFileWriter();
    void open(const char* filename, unsigned int snaplen);
    void close();
    bool isOpen() { return NULL != pcapDumper; }
    void write(uint32_t sec, uint32_t usec, const void *buff, uint32_t capLen, uint32_t fullLen);
};
#endif

/**
 * pcap file reader.
 *
 * This class uses pcap library for read data from pcap file.
 */
class PcapFileReader
{
  protected:
    pcap_t *pcap;
    fpos_t pos0;
  public:
    PcapFileReader() : pcap(NULL) {}
    virtual ~PcapFileReader();
    void open(const char* filename);
    void close();
    bool eof();
    bool isOpen() { return NULL != pcap; }
    void restart();
    const void* read(uint32_t &sec, uint32_t &usec, uint32_t &capLen, uint32_t& origLen);
};

#endif //__INET_UTIL_PCAPFILE_H
