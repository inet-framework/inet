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

/*
    file: SocketInterfacePacket.h
    Purpose:
    Usage:
    author: Ulrich Kaage
*/

#ifndef __SOCKETINTERFACEPACKET_H__
#define __SOCKETINTERFACEPACKET_H__

#include <omnetpp.h>
#include <string.h>
#include <iostream>

#include "sockets.h"

/*  -------------------------------------------------
    Type Definitions
    -------------------------------------------------   */

class SocketInterfacePacket: public cMessage
{
public:

  enum SockAction
    {
      SA_SOCKET,
      SA_SOCKET_RET,
      SA_BIND,
      SA_LISTEN,
      SA_ACCEPT,
      SA_ACCEPT_RET,
      SA_CONNECT,
      SA_CONNECT_RET,
      SA_WRITE,
      SA_WRITE_RET,
      SA_READ,
      SA_READ_RET,
      SA_SHUTDOWN,
      SA_CLOSE,
      SA_UNDEF
    };

private:

  SockAction _action;   // identifies the current action

  // address info
  IPAddress _laddr;       // local
  PortNumber _lport;
  IPAddress _faddr;       // foreign
  PortNumber _fport;

  // socket info
  Socket::Domain   _domain;
  Socket::Type     _type;
  Socket::Protocol _proto;
  Socket::Filedesc _filedesc;

  void _init();
  void _clear();
  void _check();


public:

  //
  // Constructors, destructors,...
  //

  SocketInterfacePacket();
  SocketInterfacePacket(const char* name);
  SocketInterfacePacket(const  SocketInterfacePacket& );

  SocketInterfacePacket& operator=(const SocketInterfacePacket& ip);
  virtual SocketInterfacePacket *dup() {return new SocketInterfacePacket(*this); }

  virtual void info(char *buf);
  virtual void writeContents(std::ostream& os);

  //
  // API  Application --> SocketLayer
  //

  // typical call sequences.
  //
  // SERVER: socket() -*-> bind() --> listen() --> accept() -*-> read()/write()
  // --> close()
  //
  // CLIENT: socket() -*-> --> connect() -*-> read()/write() -->
  // shutdown() --> close()
  //
  // --> denotes that the application can continue immediately with the next
  // call.
  //
  // -*-> denotes that the application has to wait for a message being sent from
  // the SocketLayer until it is allowed to proceed the call sequence.
  //

  // socket() creates a new socket associated with a file descriptor in the
  // SocketLayer. The descriptor is sent back to the application for further
  // system calls.
  void socket(Socket::Domain domain, Socket::Type type, Socket::Protocol proto);

  // bind() is used to associate a previously created socket with a local
  // network transport address. This needs to be done by a server application to
  // specify the local address/port for connection admission.
  void bind(Socket::Filedesc desc, IPAddress addr, PortNumber port);

  // listen() makes the transport layer accept connection requests by remote applications
  // (TCP passive open)
  void listen(Socket::Filedesc desc, int backlog = -1);

  // accept() accepts an incoming connection request.
  void accept(Socket::Filedesc desc);

  // connect() initiates a connection request to a remote application (TCP
  // active open)
  void connect(Socket::Filedesc desc, IPAddress faddr, PortNumber fport);

  // read/write calls
  void write(Socket::Filedesc desc, cMessage* msg);
  void read(Socket::Filedesc desc);

  // shutdown/close
  void shutdown(Socket::Filedesc desc);
  void close(Socket::Filedesc desc);

  //
  // API SocketLayer --> Application
  //

  void setSockPair(const IPAddress& laddr, PortNumber& lport, const IPAddress& faddr, PortNumber& fport);
  void setFiledesc(Socket::Filedesc desc) {_filedesc = desc;}

  void socket_ret(Socket::Filedesc desc);
  void accept_ret(Socket::Filedesc desc, const IPAddress& fadd, PortNumber& fport);
  void connect_ret(Socket::Filedesc desc);

  void read_ret(Socket::Filedesc desc, cMessage* msg, IPAddress faddr, PortNumber fport);

  //
  // general
  //
  // accessing private data (mainly by the SocketLayer)

  SockAction action() const {return _action;}
  const IPAddress& lAddr() const {return _laddr;}
  const PortNumber& lPort() const {return _lport;}
  const IPAddress& fAddr() const {return _faddr;}
  const PortNumber& fPort() const {return _fport;}

  Socket::Domain domain() const {return _domain;}
  Socket::Type type()   const {return _type;}
  Socket::Protocol proto()  const {return _proto;}
  Socket::Filedesc filedesc() const {return _filedesc;}
};

#endif // __SOCKETINTERFACEPACKET_H__
