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
//

/*
    file: Socketinterfacepacket.cc
    Purpose:
    Usage:
*/


#include <omnetpp.h>
#include <string.h>
#include <stdio.h>

#include "SocketInterfacePacket.h"

using std::ostream;

// constructors
SocketInterfacePacket::SocketInterfacePacket(): cMessage()
{
  _init();
}

SocketInterfacePacket::SocketInterfacePacket(const char* name): cMessage(name)
{
  _init();
}

// copy constructor
SocketInterfacePacket::SocketInterfacePacket( const SocketInterfacePacket& sockp)
        : cMessage()
{
  setName ( sockp.name() );
  operator=( sockp );
}

// private function
void SocketInterfacePacket::_init()
{
  _action = SocketInterfacePacket::SA_UNDEF;

  _laddr = _faddr = IPADDRESS_UNDEF;
  _lport = _fport = PORT_UNDEF;

  //_domain   = Socket::AF_UNDEF;
  //_type     = Socket::SOCK_UNDEF;
  _domain   = Socket::SOCKET_AF_UNDEF;
  _type     = Socket::SOCKET_UNDEF;
  _proto    = Socket::PROTO_UNDEF;
  _filedesc = Socket::FILEDESC_UNDEF;
}

void SocketInterfacePacket::_clear()
{
  _action = SocketInterfacePacket::SA_UNDEF;
}

void SocketInterfacePacket::_check()
{
  if (_action != SocketInterfacePacket::SA_UNDEF)
    opp_error("Already initialized! Please don't reuse SocketInterfacePackets.");
}

// assignment operator
SocketInterfacePacket& SocketInterfacePacket::operator=
        (const SocketInterfacePacket& sockp)
{
    cMessage::operator=(sockp);

    _init();
        _action = sockp._action;

    return *this;
}

// // encapsulation
// void SocketInterfacePacket::encapsulate(cPacket *p)
// {
//     cPacket::encapsulate(p);
// }

// // decapsulation: convert to cPacket *
// cPacket *SocketInterfacePacket::decapsulate()
// {
//     return (cPacket *)(cPacket::decapsulate());
// }

// output functions
void SocketInterfacePacket::info( char *buf )
{
    cMessage::info( buf );
        sprintf(buf+strlen(buf), "  _action = %d ", (int) _action);
}

void SocketInterfacePacket::writeContents(ostream& os)
{
}

// Socket creation
void SocketInterfacePacket::socket(Socket::Domain domain, Socket::Type type, Socket::Protocol proto)
{
  _check();
  _action = SA_SOCKET;

  _domain = domain;
  _type   = type;
  _proto  = proto;
}

void SocketInterfacePacket::bind(Socket::Filedesc desc, IPAddress addr, PortNumber port)
{
  _check();
  _action = SA_BIND;

  _filedesc = desc;
  _laddr    = addr;
  _lport    = port;
}

// server interface functions
void SocketInterfacePacket::listen(Socket::Filedesc desc, int backlog)
{
  _check();
  _action = SA_LISTEN;

  _filedesc = desc;
}

void SocketInterfacePacket::accept(Socket::Filedesc desc)
{
  _check();
  _action = SA_ACCEPT;

  _filedesc = desc;
}

// client interface functions
void SocketInterfacePacket::connect(Socket::Filedesc desc, IPAddress faddr, PortNumber fport)
{
  _check();
  _action = SA_CONNECT;
  _filedesc   = desc;
  _faddr  = faddr;
  _fport  = fport;

  ev << "Value of faddr: " << _faddr << endl
     << "Value of fport: " << _fport << endl;
}

// read/write calls
void SocketInterfacePacket::write(Socket::Filedesc desc, cMessage* msg)
{
  _check();
  _filedesc   = desc;
  _action = SA_WRITE;
  encapsulate(msg);

}

void SocketInterfacePacket::read(Socket::Filedesc desc)
{
  _check();
  _filedesc   = desc;
  _action = SA_READ;

}

void SocketInterfacePacket::shutdown(Socket::Filedesc desc)
{
  _check();
  _filedesc = desc;
  _action = SA_SHUTDOWN;

}

void SocketInterfacePacket::close(Socket::Filedesc desc)
{
 // _check();
  _filedesc = desc;
  _action = SA_CLOSE;

}

void SocketInterfacePacket::setSockPair(const IPAddress& laddr, PortNumber& lport, const IPAddress& faddr, PortNumber& fport)
{
  _laddr = laddr;
  _lport = lport;
  _faddr = faddr;
  _fport = fport;
}

void SocketInterfacePacket::socket_ret(Socket::Filedesc desc)
{
  _check();
  _action = SA_SOCKET_RET;
  setFiledesc(desc);
}

void SocketInterfacePacket::accept_ret(Socket::Filedesc desc, const IPAddress& fadd, PortNumber& fport)
{
  _check();
  _action = SA_ACCEPT_RET;
  setFiledesc(desc);
  _faddr = fadd;
  _fport = fport;
}

void SocketInterfacePacket::connect_ret(Socket::Filedesc desc)
{
  _check();
  _action = SA_CONNECT_RET;
  setFiledesc(desc);
}

void SocketInterfacePacket::read_ret(Socket::Filedesc desc, cMessage* msg, IPAddress faddr, PortNumber fport)
{
  _check();
  _filedesc   = desc;
  _action = SA_READ_RET;
  _faddr  = faddr;
  _fport  = fport;

  ev << "Value of faddr: " << _faddr << endl
     << "Value of fport: " << _fport << endl;

  encapsulate(msg);

}
