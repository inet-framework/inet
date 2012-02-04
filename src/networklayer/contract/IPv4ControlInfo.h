//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IPv4CONTROLINFO_H
#define __INET_IPv4CONTROLINFO_H

#include "IPv4ControlInfo_m.h"

class IPv4Datagram;

/**
 * Control information for sending/receiving packets over IPv4.
 *
 * See the IPv4ControlInfo.msg file for more info.
 */
class INET_API IPv4ControlInfo : public IPv4ControlInfo_Base
{
  protected:
    IPv4Datagram *dgram;

  private:
    void copy(const IPv4ControlInfo& other);
    void clean();

  public:
    IPv4ControlInfo() : IPv4ControlInfo_Base() {dgram = NULL;}
    virtual ~IPv4ControlInfo();
    IPv4ControlInfo(const IPv4ControlInfo& other) : IPv4ControlInfo_Base(other) { dgram = NULL; copy(other); }
    IPv4ControlInfo& operator=(const IPv4ControlInfo& other);
    virtual IPv4ControlInfo *dup() const {return new IPv4ControlInfo(*this);}

    virtual void setOrigDatagram(IPv4Datagram *d);
    virtual IPv4Datagram *getOrigDatagram() const {return dgram;}
    virtual IPv4Datagram *removeOrigDatagram();
};

#endif


