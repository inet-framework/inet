//
// Copyright (C) 2001 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
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

#ifndef SOCKETLAYER_H
#define SOCKETLAYER_H

#include <omnetpp.h>
#include "sockets.h"
#include "RoutingTable.h"
#include "SocketInterfacePacket.h"
#include "UDPControlInfo_m.h"

class SocketLayer : public cSimpleModule
{
 private:

  // indexes to _socklist container elements
  enum {SOCKET_ID = 0 };
  enum {FROM_APPL_ID = 1};

  // module parameters
  bool debug;

  // gate cache
  int _from_ip;
  int _to_ip;
  int _from_tcp;
  int _to_tcp;
  int _from_udp;
  int _to_udp;
  int _from_appl;
  int _from_appl_size;
  int _to_appl;
  int _to_appl_size;

  RoutingTable* _rt; // needed to determine the local address

  cArray _sockar;       // all sockets are stored here. The array index of a socket
                        // denotes the file descriptor number. Note, that this
                        // array is shared by all application layers being
                        // connected to the socket layer

  enum {WK_PORTS = 1024 };
  // array of ports.
  // well known ports are all ports WK_PORTS
  cArray _portar;

  //
  // new private functions
  //

  void _init();
  void _findRoutingTable();
  IPAddress _defaultIPAddr();
  Socket::Filedesc _newSocket(Socket::Domain dom, Socket::Type type,
                              Socket::Protocol proto, int from_appl_gate);
  void _deleteSocket(Socket::Filedesc desc);

  // _bindPort() tries to bind to the specified port. if the specified port is
  // already bound an error message is generates. If port is undefined the next
  // free ephemeral port is bound starting from port WK_PORTS.
  PortNumber _bindPort(PortNumber port, Socket* socket);
  void _releasePort(Socket* socket);


  void _handleFromUDP(cMessage *msg);
  void _handleFromAppl(SocketInterfacePacket* msg);
  int _returnGate(int arrivalgate);
  void _sendDown(cMessage* msg, Socket* socket);

 public:
  Module_Class_Members(SocketLayer, cSimpleModule, 0);

  // methods
  virtual void initialize();
  virtual void handleMessage(cMessage* msg);

  //
  // new functions
  //

  // getSocket() - retrieve a socket by its associated file descriptor
  //
  // Parameters:
  //
  // desc - file descriptor
  //
  // fullyspecified == true - check if socket is fully specified
  //
  // from_appl_gate != -1 - check if the socket was created by an
  // SocketInterfacePacket coming from the gate with ID from_appl_gate.
  //
  Socket* getSocket(Socket::Filedesc desc, bool fullyspecified = false, int from_appl_gate_id = -1) const;

  // getSocket() - retrieve a socket by its address fields
  Socket* getSocket(Socket::Protocol proto, IPAddress laddr, PortNumber lport, IPAddress faddr, PortNumber fport);


};

#endif
