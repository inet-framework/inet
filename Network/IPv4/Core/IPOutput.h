// -*- C++ -*-
//
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


#ifndef __IPOUTPUTCORE_H__
#define __IPOUTPUTCORE_H__

#include "QueueWithQoS.h"
#include "IPDatagram.h"
#include "ICMPAccess.h"


/**
 * Receives complete IP datagram (from IPFragmentation), and checks hop counter.
 * More detailed info in corresponding NED file.
 */
class IPOutput : public QueueWithQoS
{
  protected:
    ICMPAccess icmpAccess;

  public:
    Module_Class_Members(IPOutput, QueueWithQoS, 0);

  protected:
    virtual void endService(cMessage *msg);
};

#endif

