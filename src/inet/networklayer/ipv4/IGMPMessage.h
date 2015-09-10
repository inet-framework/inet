//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_IGMPMESSAGE_H
#define __INET_IGMPMESSAGE_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/ipv4/IGMPMessage_m.h"

namespace inet {

class INET_API IGMPv3Query : public IGMPv3Query_Base
{
  public:
    IGMPv3Query(const char *name = nullptr) : IGMPv3Query_Base(name) {}
    IGMPv3Query(const IGMPv3Query& other) : IGMPv3Query_Base(other) {}
    IGMPv3Query& operator=(const IGMPv3Query& other) { IGMPv3Query_Base::operator=(other); return *this; }
    virtual IGMPv3Query *dup() const override { return new IGMPv3Query(*this); }

//        virtual unsigned char getMaxRespTime() const;
//        virtual void setMaxRespTime(unsigned char maxRespTime);
    virtual unsigned char getMaxRespCode() const override { return maxRespTime_var; }
    virtual void setMaxRespCode(unsigned char maxRespCode) override { this->maxRespTime_var = maxRespCode; }
};

}    // namespace inet

#endif // ifndef _IGMPMESSAGE_H_

