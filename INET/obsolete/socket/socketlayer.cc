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

#include "socketlayer.h"


Define_Module(SocketLayer);

void SocketLayer::_init()
{
  _rt = NULL;
  _sockar.setName("Socket-List");
  _portar.setName("Ports");
}

void SocketLayer::_findRoutingTable()
{
  cObject *foundmod;
  cModule *curmod = this;

  // find Routing Table
  _rt = NULL;
  for (curmod = parentModule(); curmod != NULL;
       curmod = curmod->parentModule())
    {
      if ((foundmod = curmod->findObject("routingTable", true)) != NULL)
        {
          _rt = (RoutingTable *)foundmod;
          break;
        }
    }
  if (!_rt)
    error("Module routingTable not found!");
}

IPAddress SocketLayer::_defaultIPAddr()
{
  if (_rt)
    return _rt->interfaceByPortNo(0)->inetAddr;
  else
    return IPADDRESS_UNDEF;
}

Socket::Filedesc SocketLayer::_newSocket(Socket::Domain dom, Socket::Type type,
                                         Socket::Protocol proto, int from_appl_gate)
{
  // put a new socket and the from_appl_gate id into a container object. Add the
  // container to the socket array
  Socket::Filedesc desc;
  cArray*          container    = new cArray("Socket/Gate-Container");
  Socket*          socket       = new Socket(dom, type, proto);
  cPar*            from_appl_id = new cMsgPar("from_appl_gate_id");
  char             buf[1024];

  *from_appl_id = from_appl_gate;

  container->addAt(SOCKET_ID, socket);
  container->addAt(FROM_APPL_ID, from_appl_id);

  desc = _sockar.add(container);

  if (debug)
    {
      ev << "created new socket with filedescriptor no. " << (int) desc << endl
         << "socket info:";
      socket->info(&buf[0]);
      ev << &buf[0] << endl;
    }

  return desc;
}

void SocketLayer::_deleteSocket(Socket::Filedesc desc)
{

  cArray* container = (cArray*) _sockar.remove(desc);

  if (debug)
    {
      char buf[1024];
      ev << "Deleting Socket ";
      (*container)[SOCKET_ID]->info(&buf[0]);
      ev << &buf[0] << endl;
    }

  delete container->remove(SOCKET_ID);
  delete container->remove(FROM_APPL_ID);
  delete container;
}

PortNumber SocketLayer::_bindPort(PortNumber port, Socket* socket)
{
  cArray* container = new cArray("Socket-Container");
  container->takeOwnership(false); // Sockets inserted here are owned
                   // by _sockar instead;
  container->add(socket);

  if (port == PortNumber(PORT_UNDEF))
    {
      // bind to a ephemeral port
      for (int idx = WK_PORTS; (port == PortNumber(PORT_UNDEF)) && (port < PortNumber(PORT_MAX)); idx++)
        {
          if (!_portar.exist(idx))
            port = idx;
        }
    }

  if (port == PortNumber(PORT_MAX))
    error("No more ports available");

  if (debug)
    ev << "Binding to port " << (int) port << endl;

  return _portar.addAt(port, container);

}

void SocketLayer::_releasePort(Socket* socket)
{
  cArray* container = NULL;
  int ctr_idx;
  bool found = false;

  for(int idx = 0; (idx < _portar.items()) && (found == false); idx++)
    {
      if ((container = (cArray*) _portar.get(idx)) != NULL)
        if ((ctr_idx = container->find(socket)) != -1)
          {
            container->remove(ctr_idx);
            if (container->items() == 0)
              _portar.remove(idx);
            found = true;
          }
    }
}

void SocketLayer::initialize()
{
  _init();
  // get module parameters
  debug = par("debug");

  // initialize gate cache
  _from_ip        = gate("from_ip")->id();
  _to_ip          = gate("to_ip")->id();
  _from_tcp       = gate("from_tcp")->id();
  _to_tcp         = gate("to_tcp")->id();
  _from_udp       = gate("from_udp")->id();
  _to_udp         = gate("to_udp")->id();
  _from_appl      = gate("from_appl")->id();
  _from_appl_size = gateSize(_from_appl);
  _to_appl        = gate("to_appl")->id();
  _to_appl_size   = gateSize(_to_appl);

  // find the routing table module
  _findRoutingTable();
}

void SocketLayer::handleMessage(cMessage* msg)
{
  int arrivalgate = msg->arrivalGateId();
  // dispatch events
  if (arrivalgate == _from_ip)
    {
    }
  else if (arrivalgate == _from_tcp)
    {
    }
  else if (arrivalgate == _from_udp)
    {
      _handleFromUDP(msg);
    }
  else if (arrivalgate >= _from_appl && arrivalgate <= (_from_appl + _from_appl_size))
    {
      _handleFromAppl((SocketInterfacePacket*) msg);
    }
  else
    error("Message coming from unknown gate!");

  delete msg;
}

int SocketLayer::_returnGate(int arrivalgate)
{
  int outgate;

  if (arrivalgate == _from_ip)
    outgate = _to_ip;
  else if (arrivalgate == _from_tcp)
    outgate = _to_tcp;
  else if (arrivalgate == _from_udp)
    outgate = _to_udp;
  else if (arrivalgate >= _from_appl && arrivalgate <= (_from_appl + _from_appl_size))
    outgate = _to_appl + (arrivalgate - _from_appl);
  else
    {
      error("Cannot determine output gate!");
      outgate = -1;
    }
  return outgate;
}

void SocketLayer::_sendDown(cMessage* msg, Socket* socket)
{
  char* name = NULL;
  int outgate;

  switch(socket->protocol())
    {
    case Socket::UDP:
      outgate = _to_udp;
      name = "UDP";
      break;
    case Socket::TCP:
      outgate = _to_tcp;
      name = "TCP";
      break;
    case Socket::ICMP:
      outgate = _to_ip;
      name = "ICMP";
      break;
    case Socket::IGMP:
      outgate = _to_ip;
      name = "IGMP";
      break;
    case Socket::ROUTE:
      outgate = _to_ip;
      name = "Route";
      break;
    default:
      error("Unknown Protocol");
      break;
    }
  msg->setName(name);

  if (debug)
    ev << "Sending " << name << "-msg to gate nr. " << outgate << endl;

  send(msg, outgate);
}

void SocketLayer::_handleFromUDP(cMessage *msg)
{
  cArray* container = NULL;
  int from_appl_id = -1;
  Socket::Filedesc filedesc;
  bool found = false;

  if (debug)
    ev << "Received msg " << msg->name() << " from UDP\n";

  UDPControlInfo *controlInfo = check_and_cast<UDPControlInfo *>(msg->removeControlInfo());

  Socket* socket = getSocket(Socket::UDP,
                             controlInfo->getDestAddr(), controlInfo->getDestPort(),
                             controlInfo->getSrcAddr(), controlInfo->getSrcPort());

  if (socket)
    {
      if (debug) ev << "Forwarding packet to application layer\n";
      SocketInterfacePacket* sockipack = new SocketInterfacePacket("FromUDP");
      // get file descriptor and return gate
      for(int idx=0; (idx < _sockar.items()) && (found == false); idx++)
        {
          container = (cArray*) _sockar[idx];
          if (container && ((Socket*) (container->get(SOCKET_ID)) == socket))
            {
              from_appl_id = (int) *((cPar*) container->get(FROM_APPL_ID));
              filedesc = idx;
              found = true;
            }
        }

      if (found == false)
        error("No filedescriptor found");

      sockipack->read_ret(filedesc, msg, controlInfo->getSrcAddr(), controlInfo->getSrcPort());
      send(sockipack, _returnGate(from_appl_id));
    }
  else
    {
      error("No Socket available for UDP Port %d", controlInfo->getDestPort());
    }
  delete controlInfo;
}


void SocketLayer::_handleFromAppl(SocketInterfacePacket* msg)
{
  cMessage *datapacket = NULL;
  UDPControlInfo *controlInfo = NULL;
  Socket* socket = NULL;
  SocketInterfacePacket* sockipack = NULL;
  Socket::Filedesc filedesc;


  if (strcmp(msg->className(), "SocketInterfacePacket") != 0)
    error("Need a msg of class SocketInterfacePacket");

  SocketInterfacePacket::SockAction action = msg->action();


  switch(action)
    {
    case SocketInterfacePacket::SA_SOCKET:

      // create a new socket and put it into the socket list;


      filedesc = _newSocket(msg->domain(), msg->type(), msg->proto(), msg->arrivalGateId());

      // return a message with the filedescriptor to the application
      sockipack = new SocketInterfacePacket("Socket_Ret");
      sockipack->socket_ret(filedesc);
      send(sockipack, _returnGate(msg->arrivalGateId()));


      break;

    case SocketInterfacePacket::SA_BIND:

      socket = getSocket(msg->filedesc(), false, msg->arrivalGateId());

      if (msg->lAddr() == IPADDRESS_UNDEF)
        socket->pcb()->setLAddr(_defaultIPAddr());
      else
        socket->pcb()->setLAddr(msg->lAddr());


      // occupy the port
      socket->pcb()->setLPort(_bindPort(msg->lPort(), socket));

      // nothing is returned.
      break;

    case SocketInterfacePacket::SA_LISTEN:

      socket = getSocket(msg->filedesc(), false, msg->arrivalGateId());

      //socket->setSOPT_ACCEPTCONN();
      socket->setConnState(Socket::CONN_LISTEN);

      if (debug)
        ev << "Server Socket sending PASSIVE OPEN to UDP.\n";

      // FIXME: send passive open message to TCP

      break;
    case SocketInterfacePacket::SA_ACCEPT:

      socket = getSocket(msg->filedesc(), false, msg->arrivalGateId());

      if (socket->connState() != Socket::CONN_LISTEN)
        error("Socket %d is not listening", msg->filedesc());
      else
        {
          socket->setSOPT_ACCEPTCONN();
          socket->setPendingAccept();
          // FIXME: return accept_ret() to application
        }
      break;

    case SocketInterfacePacket::SA_CONNECT:

      socket = getSocket(msg->filedesc(), false, msg->arrivalGateId());

      if (socket->pcb()->lAddr() == IPADDRESS_UNDEF)
        socket->pcb()->setLAddr(_defaultIPAddr());
      else
        socket->pcb()->setLAddr(msg->lAddr());

      if (socket->pcb()->lPort() == PortNumber(PORT_UNDEF))
        {
          // get a new ephemeral local port
          socket->pcb()->setLPort(_bindPort(msg->lPort(), socket));
        }

      if (msg->fAddr() == IPADDRESS_UNDEF)
        error("No Address defined");
      if (msg->fPort() == PortNumber(PORT_UNDEF))
        error("No Port defined");

      socket->pcb()->setFAddr(msg->fAddr());
      socket->pcb()->setFPort(msg->fPort());

      sockipack = new SocketInterfacePacket("Connect_Ret");

      sockipack->connect_ret(msg->filedesc());
      send(sockipack,_returnGate(msg->arrivalGateId()));

      // UDP: socket is now fully specified
      socket->setConnState(Socket::CONN_ESTAB);

      // TCP: send active open

      break;



    case SocketInterfacePacket::SA_WRITE:

      socket = getSocket(msg->filedesc(), true, msg->arrivalGateId());
      ev << "Sending data to transport layer. \n";

      datapacket = msg->decapsulate();

      controlInfo = new UDPControlInfo();
      controlInfo->setSrcAddr(socket->pcb()->lAddr());
      controlInfo->setDestAddr(socket->pcb()->fAddr());
      controlInfo->setSrcPort(socket->pcb()->lPort());
      controlInfo->setDestPort(socket->pcb()->fPort());
      datapacket->setControlInfo(controlInfo);

      // send data to transport layer
      _sendDown(datapacket, socket);

      break;

    case SocketInterfacePacket::SA_READ:

      socket = getSocket(msg->filedesc(), true, msg->arrivalGateId());
      ev <<"Reading data from UDP. \n";
      sockipack = new SocketInterfacePacket("Read_Ret");
      sockipack->read(filedesc);
      send(sockipack,_returnGate(msg->arrivalGateId()));

      // FIXME: Retrieve data from TCP

      break;

    case SocketInterfacePacket::SA_SHUTDOWN:

      socket = getSocket(msg->filedesc(), true, msg->arrivalGateId());
      // FIXME: Shutdown connection

      break;
    case SocketInterfacePacket::SA_CLOSE:

      socket = getSocket(msg->filedesc(), false, msg->arrivalGateId());

      // FIXME: A timer should be set to delete the socket and port after some
      // time to be able to handle packages that arrive too late.

      // Delete Socket
      _releasePort(socket);
      _deleteSocket(msg->filedesc());

      break;
    default:
      error("Action %d not handled in switch", (int) action);
    }
}

Socket* SocketLayer::getSocket(Socket::Filedesc desc, bool fullyspecified, int from_appl_gate_id) const
{
  cArray* container = (cArray*) _sockar[desc];

  // checking if the socket was created by a message coming from the same gate.
  if (from_appl_gate_id != -1)
    {
      if (debug)
        ev << "checking for gate correctness... ";
      int _appl_gate_id = *((cPar*) (*container)[FROM_APPL_ID]);
      if (_appl_gate_id != from_appl_gate_id)
        {
          if (debug)
            ev << "failed\n";
          error ("Gates do not match");
        }
      else if (debug)
        ev << " passed\n";
    }

  Socket* socket = (Socket*) (*container)[SOCKET_ID];

  if (socket == NULL)
    error("No socket associated with Filedescriptor %d", desc);

  // checking if socket is fully specified - if requested only
  if (fullyspecified && !socket->isFullySpecified())
    error("Socket is not fully specified");

  if (debug)
    ev << "Retrieving Socket with file descriptor " << desc << endl;

  return socket;
}

Socket* SocketLayer::getSocket(Socket::Protocol proto, IPAddress laddr, PortNumber lport, IPAddress faddr, PortNumber fport)
{
  Socket* socket      = NULL;
  Socket* sock_listen = NULL;   // a socket in listen mode
  Socket* sock_full   = NULL;   // a full estabilshed socket

  cArray* container = (cArray*) _portar.get(lport);

  if (container == NULL)
    {
      error("No socket associated with port %d", (int) lport);
    }
  else
    {
      bool    found  = false;

      for(int idx = 0; (idx < container->items()) && (!found); idx++)
        {
          if((socket = (Socket*) container->get(idx)) != NULL)
            {
              PCB* pcb = socket->pcb();
              if ((socket->protocol() == proto) && (pcb->lAddr() == laddr) && (pcb->lPort() == lport))
                {
                  if ((socket->connState() == Socket::CONN_LISTEN))
                    {
                      sock_listen = socket;
                    }
                  else if ((socket->connState() == Socket::CONN_ESTAB) &&
                           (pcb->fAddr() == faddr) && (pcb->fPort() == fport))
                    {
                      sock_full = socket;
                      found = true;
                    }
                  else if ((socket->protocol() == Socket::UDP))
                    {
                      sock_full = socket;
                      found = true;
                    }
                }
            }
        }
      socket = sock_full? sock_full : sock_listen;

      if (! socket)
        opp_error("Couldn't find socket");
    }
  return socket;
}
