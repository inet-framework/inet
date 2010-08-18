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

#ifndef __INET_TCPCOMMAND_H
#define __INET_TCPCOMMAND_H_

#include <omnetpp.h>
#include "TCPCommand_m.h"


class TCPDataMsg : public TCPDataMsg_Base
{
  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const TCPDataMsg&);

  public:
    TCPDataMsg(const char *name=NULL);
    TCPDataMsg(const TCPDataMsg& other);
    virtual ~TCPDataMsg();
    TCPDataMsg& operator=(const TCPDataMsg& other);
    virtual TCPDataMsg *dup() const {return new TCPDataMsg(*this);}

    virtual void setDataObject(const cPacketPtr& dataObject);
    virtual cPacket* removeDataObject();
};

inline void doPacking(cCommBuffer *b, TCPDataMsg& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, TCPDataMsg& obj) {obj.parsimUnpack(b);}


#endif // __INET_TCPCOMMAND_H_
