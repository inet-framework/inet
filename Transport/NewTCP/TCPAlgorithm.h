//
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

#ifndef __TCPALGORITHM_H
#define __TCPALGORITHM_H

#include <omnetpp.h>
#include "TCPConnection.h"


class TCPSegment;
class TCPInterfacePacket;


/**
 * Abstract base class for TCP algorithms which encapsulate all behaviour
 * during data transfer state: flavour of congestion control, fast
 * retransmit/recovery, selective acknowledgement etc. Subclasses
 * may implement various sets and flavours of the above these algorithms.
 */
class TCPAlgorithm : public cPolymorphic   // FIXME let it be TCPBehaviour? or TCPDataTransfer?
{
  protected:
    TCPConnection *conn; // TCP connection object

  public:
    /**
     * Ctor.
     */
    TCPAlgorithm(TCPConnection *_conn)  {conn=_conn;}

    /**
     * Virtual dtor.
     */
    virtual ~TCPAlgorithm() {}

    /**
     * Process SEND app command.
     */
    virtual void process_SEND(TCPInterfacePacket *tcpIfPacket) = 0;

    /**
     * Process RECEIVE app command. FIXME simplify args to ulong numOctets?
     */
    virtual void process_RECEIVE(TCPInterfacePacket *tcpIfPacket) = 0;
    //...
};

#endif


