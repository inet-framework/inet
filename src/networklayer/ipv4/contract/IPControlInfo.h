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

#ifndef __INET_IPCONTROLINFO_H
#define __INET_IPCONTROLINFO_H

#include "IPControlInfo_m.h"

class IPDatagram;

/**
 * Control information for sending/receiving packets over IP.
 *
 * See the IPControlInfo.msg file for more info.
 */
class INET_API IPControlInfo : public IPControlInfo_Base
{
  protected:
    IPDatagram *dgram;
  public:
    IPControlInfo() : IPControlInfo_Base() {dgram=NULL;}
    virtual ~IPControlInfo();
    IPControlInfo(const IPControlInfo& other) : IPControlInfo_Base() {dgram=NULL; operator=(other);}
    IPControlInfo& operator=(const IPControlInfo& other) {IPControlInfo_Base::operator=(other); return *this;}

    virtual void setOrigDatagram(IPDatagram *d);
    virtual IPDatagram *getOrigDatagram() const {return dgram;}
    virtual IPDatagram *removeOrigDatagram();
};

#endif


