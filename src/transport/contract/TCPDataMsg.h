//
// Copyright (C) 2010 Zoltan Bojthe
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

#ifndef __INET_TCPDATAMSG_H
#define __INET_TCPDATAMSG_H

#include <omnetpp.h>
#include "TCPCommand_m.h"


class TCPDataMsg : public ByteArrayMessage
{
  protected:
    cPacket* payloadPacket_var;
    bool isPayloadStart_var;

    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const TCPDataMsg&);

  public:
    TCPDataMsg(const char *name=NULL);
    TCPDataMsg(const TCPDataMsg& other);
    virtual ~TCPDataMsg();
    TCPDataMsg& operator=(const TCPDataMsg& other);
    virtual TCPDataMsg *dup() const {return new TCPDataMsg(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual cPacket* getPayloadPacket();
    virtual const cPacket* getPayloadPacket() const {return const_cast<TCPDataMsg*>(this)->getPayloadPacket();}
    virtual cPacket* removePayloadPacket();
    virtual void setPayloadPacket(cPacket* payloadPacket_var);
    virtual bool getIsPayloadStart() const;
    virtual void setIsPayloadStart(bool isPayloadStart_var);
};

inline void doPacking(cCommBuffer *b, TCPDataMsg& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, TCPDataMsg& obj) {obj.parsimUnpack(b);}


#endif // __INET_TCPDATAMSG_H
