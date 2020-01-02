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

#ifndef __INET_IPCAPWRITER_H
#define __INET_IPCAPWRITER_H

#include "inet/common/packet/Packet.h"

namespace inet {

class INET_API IPcapWriter
{
  public:
    virtual ~IPcapWriter() { }

    virtual void open(const char *filename, unsigned int snaplen, uint32_t linkType) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;

    virtual void setFlush(bool flush) = 0;

    virtual void writePacket(simtime_t time, const Packet *packet) = 0;
};

} // namespace inet

#endif // ifndef __INET_IPCAPWRITER_H

