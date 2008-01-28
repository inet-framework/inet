//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#ifndef __IPTUNNELING_H__
#define __IPTUNNELING_H__


#include "IPDatagram.h"

/**
 * Receives an IP datagram, set tunnel destination address and Protocol fields,
 * then sends it to IPSend to be newly encapsulated.
 * More detailed comment in NED file.
 */
class INET_API IPTunneling : public cSimpleModule
{
  public:
    Module_Class_Members(IPTunneling, cSimpleModule, 0);

  protected:
    virtual void handleMessage(cMessage *msg);
};

#endif

