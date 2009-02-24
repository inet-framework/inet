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

#include "sockets.h"
#include "tcpcb.h"

using std::ostream;

Register_Class(Socket);

// string arrays - must be kept in sync with the corresponding enums!
const char* const Socket::domain_string[] = {"AF_UNIX", "AF_INET", "AF_UNDEF"};
const char* const Socket::type_string[] = {"SOCK_STREAM", "SOCK_DGRAM", "SOCK_RAW",
                                      "SOCK_SEQPACKET", "SOCK_UNDEF"};
const char* const Socket::protocol_string[] = {"UDP", "TCP", "ICMP", "IGMP", "ROUTE", "PROTO_UNDEF"};
const char* const Socket::connstate_string[] = {"CONN_LISTEN", "CONN_HALFESTAB", "CONN_ESTAB", "CONN_UNDEF"};


void Socket::_initOptions()
{
  _options.SOPT_ACCEPTCONN  = false;
  _options.SOPT_BROADCAST   = false;
  _options.SOPT_DEBUG       = false;
  _options.SOPT_DONTROUTE   = false;
  _options.SOPT_KEEPALIVE   = false;
  _options.SOPT_OOBINLINE   = false;
  _options.SOPT_REUSEADDR   = false;
  _options.SOPT_REUSEPORT   = false;
  _options.SOPT_USELOOPBACK = false;
}

void Socket::_initState()
{
  _state.SS_CANTRCVMORE     = false;
  _state.SS_CANTSENDMORE    = false;
  _state.SS_ISCONFIRMING    = false;
  _state.SS_ISCONNECTED     = false;
  _state.SS_ISCONNECTING    = false;
  _state.SS_ISDISCONNECTING = false;
  _state.SS_NOFDREF         = false;
  _state.SS_PRIV            = false;
  _state.SS_RCVATMARK       = false;
}


void Socket::_init()
{
  _domain         = SOCKET_AF_UNDEF;
  _type           = SOCKET_UNDEF;
  _proto          = PROTO_UNDEF;
  _connstate      = CONN_UNDEF;
  _pending_accept = false;

  _pcb       = NULL;
  _sockqueue.setName("Socket Queue");
  _initOptions();
  _initState();
}


Socket::Socket() : cObject("Socket")
{
  _init();
}

Socket::Socket(const Socket& socket) : cObject()
{
  setName(socket.name());
  operator=(socket);
}

Socket::Socket(const char* name = NULL) : cObject(name)
{
  _init();
}

Socket::Socket(Socket::Domain domain, Socket::Type type, Socket::Protocol proto) : cObject("Socket")
{
  _init();

  _domain = domain;
  _type   = type;
  _proto  = proto;

  _pcb = _createPCB(proto);
}

Socket::~Socket()
{
  delete _pcb;
}

void Socket::info(char* buf)
{
  cObject::info(buf);
  sprintf(buf+strlen(buf), " Socket: %s, %s, %s, %s ", domain_string[_domain], type_string[_type], protocol_string[_proto], connstate_string[_connstate]);
  if (_pcb)
    _pcb->info(buf + strlen(buf));
}

void Socket::writeContents(ostream& os)
{
  cObject::writeContents(os);
  os << " Socket:\n"
     << "  Domain:   " << domain_string[_domain] << '\n'
     << "  Type:     " << type_string[_type] << '\n'
     << "  Protocol: " << protocol_string[_proto] << '\n'
     << "  State:    " << connstate_string[_connstate] << '\n';
  if (_pcb)
    _pcb->writeContents(os);
}

Socket& Socket::operator=(const Socket& socket)
{
  if (this != &socket)
    {
      cObject::operator=(socket);
      _type = socket.type();
      _pcb = new PCB(*socket.pcb());
    }
  return *this;
}

bool Socket::isFullySpecified()
{
  return (_pcb->fAddr() != IPADDRESS_UNDEF &&
          _pcb->fPort() != PortNumber(PORT_UNDEF) &&
          _pcb->lAddr() != IPADDRESS_UNDEF &&
          _pcb->lPort() != PortNumber(PORT_UNDEF));
}

// helper functions

PCB* _createPCB(Socket::Protocol proto)
{
  PCB* pcb = NULL;

  switch(proto)
    {
    case Socket::TCP:
      opp_error("TCP socketinterface not implemented yet!");
      // pcb = new TCPCB("TCPCB");
      break;

    case Socket::UDP:
      // UDP in fact uses the standard PCB
      pcb = new PCB("UDP_PCB");
      break;

    case Socket::ICMP:
    case Socket::IGMP:
    case Socket::ROUTE:
      opp_error("This PCB is not implemented yet (%d)", (int) proto);
      break;
    case Socket::PROTO_UNDEF:
      opp_error("No Protocol defined (PROTO_UNDEF)");
      break;
    }
  return pcb;
}
