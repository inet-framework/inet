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


#ifndef SOCKET_H
#define SOCKET_H

#include <iostream>
#include <omnetpp.h>

#include "pcb.h"


class Socket : public cObject
{
public:

  enum Domain {
      SOCKET_AF_UNIX,  // file system pathnames not used
      SOCKET_AF_INET,   // internet address
      SOCKET_AF_UNDEF
    };

  static const char* const domain_string[]; // defined in sockets.cc

  enum Type {
      SOCKET_STREAM,      // sequenced, reliable, bidirectional, connection-mode
                        // byte streams, and may provide a transmission
                        // mechanism for out-of-band data (TCP)
      SOCKET_DGRAM,       // provides datagrams, which are connectionless-mode,
                        // unreliable messages of fixed maximum length (UDP)
      SOCKET_RAW,         // best-effort network-level datagram service (ICMP,
                        // IGMP, raw IP)
      SOCKET_SEQPACKET,   // provides sequenced, reliable, bidirectional,
                        // connection-mode transmission path for records.
      SOCKET_UNDEF
    };

  static const char* const type_string[]; // defined in sockets.cc

  enum Protocol
    {
      UDP,
      TCP,
      ICMP,
      IGMP,
      ROUTE,
      PROTO_UNDEF
    };

  static const char* const protocol_string[]; // defined in sockets.cc

  enum ConnectionState
    {
      CONN_LISTEN,
      CONN_HALFESTAB,
      CONN_ESTAB,
      CONN_UNDEF
    };

  static const char* const connstate_string[]; // defined in sockets.cc

  // SO_* names clashed with some Windows stuff, that's why they were renamed to SOPT_*
  // FIXME: Document which options are used
  struct Options
  {
    unsigned int SOPT_ACCEPTCONN  : 1; // socket accepts incoming connections
                                     // (kernel only)
    unsigned int SOPT_BROADCAST   : 1; // socket can send broadcast messages
    unsigned int SOPT_DEBUG       : 1; // socket records debugging information
    unsigned int SOPT_DONTROUTE   : 1; // output operations bypass routing tables
    unsigned int SOPT_KEEPALIVE   : 1; // socket probes idle connections
    unsigned int SOPT_OOBINLINE   : 1; // socket keeps out-of-band data inline
    unsigned int SOPT_REUSEADDR   : 1; // socket can reuse a local address
    unsigned int SOPT_REUSEPORT   : 1; // socket can reuse a local port
    unsigned int SOPT_USELOOPBACK : 1; // routing domain sockets only; sending
                                     // process receives its own routing
                                     // requests
  };

  // FIXME: Document which state fields are used
  struct State
  {
    unsigned int SS_CANTRCVMORE     : 1; // socket cannot receive more data from
                                         // peer
    unsigned int SS_CANTSENDMORE    : 1; // socket cannot send more data to peer
    unsigned int SS_ISCONFIRMING    : 1; // socket is negotiating a connection request
    unsigned int SS_ISCONNECTED     : 1; // socket is connected to a foreign socket
    unsigned int SS_ISCONNECTING    : 1; // socket is connecting to a foreign socket
    unsigned int SS_ISDISCONNECTING : 1; // socket is disconnecting from peer
    unsigned int SS_NOFDREF         : 1; // socket is not associated with a descriptor
    unsigned int SS_PRIV            : 1; // socket was created by a process with
                                         // superuser privileges
    unsigned int SS_RCVATMARK       : 1; // process has consumed all data
                                         // received before the most recent
                                         // out-of-band data was received
  };

  typedef int Filedesc; // file descriptor

  enum {FILEDESC_UNDEF = -1};

private:
  Domain   _domain; // in fact, belongs to the protosw structure but
                               // we have none so far
  Type     _type;
  Protocol _proto;
  Options  _options;
  State    _state;
  ConnectionState _connstate; // connection specific state of the
                                      // socket. In real world implementation
                                      // this is handled by different queues
                                      // attached to the listening socket
  bool _pending_accept; // for listening sockets, if an accept() has already
                        // been issued by the application

  PCB*             _pcb; // Pointer to Protocol Control Block class
  cQueue           _sockqueue; // queue for incoming data packets

  void _initOptions();
  void _initState();
  void _init();
public:


  // creation, destruction, copying
  Socket(const Socket& socket);
  Socket();
  explicit Socket(const char* name);
  Socket(Domain domain, Type type, Protocol proto);
  //Socket(const char* name, cObject* ownerobj);
  virtual ~Socket();
  virtual cObject* dup() const {return new Socket(*this);}
  virtual void info(char* buf);
  virtual void writeContents(std::ostream& os);
  Socket& operator=(const Socket& socket);

  // new member functions
  Type type() const {return _type;}
  Protocol protocol() const {return _proto;}
  PCB* pcb() const {return _pcb;}

  // Options
  void setSOPT_ACCEPTCONN(bool val = true)  {_options.SOPT_ACCEPTCONN  = val;}
  void setSOPT_BROADCAST(bool val = true)   {_options.SOPT_BROADCAST   = val;}
  void setSOPT_DEBUG(bool val = true)       {_options.SOPT_DEBUG       = val;}
  void setSOPT_DONTROUTE(bool val = true)   {_options.SOPT_DONTROUTE   = val;}
  void setSOPT_KEEPALIVE(bool val = true)   {_options.SOPT_KEEPALIVE   = val;}
  void setSOPT_OOBINLINE(bool val = true)   {_options.SOPT_OOBINLINE   = val;}
  void setSOPT_REUSEADDR(bool val = true)   {_options.SOPT_REUSEADDR   = val;}
  void setSOPT_REUSEPORT(bool val = true)   {_options.SOPT_REUSEPORT   = val;}
  void setSOPT_USELOOPBACK(bool val = true) {_options.SOPT_USELOOPBACK = val;}

  bool SOPT_ACCEPTCONN()  {return _options.SOPT_ACCEPTCONN;}
  bool SOPT_BROADCAST()   {return _options.SOPT_BROADCAST;}
  bool SOPT_DEBUG()       {return _options.SOPT_DEBUG;}
  bool SOPT_DONTROUTE()   {return _options.SOPT_DONTROUTE;}
  bool SOPT_KEEPALIVE()   {return _options.SOPT_KEEPALIVE;}
  bool SOPT_OOBINLINE()   {return _options.SOPT_OOBINLINE;}
  bool SOPT_REUSEADDR()   {return _options.SOPT_REUSEADDR;}
  bool SOPT_REUSEPORT()   {return _options.SOPT_REUSEPORT;}
  bool SOPT_USELOOPBACK() {return _options.SOPT_USELOOPBACK;}

  // State
  void setSS_CANTRCVMORE(bool val = true)     {_state.SS_CANTRCVMORE     = val;}
  void setSS_CANTSENDMORE(bool val = true)    {_state.SS_CANTSENDMORE    = val;}
  void setSS_ISCONFIRMING(bool val = true)    {_state.SS_ISCONFIRMING    = val;}
  void setSS_ISCONNECTED(bool val = true)     {_state.SS_ISCONNECTED     = val;}
  void setSS_ISCONNECTING(bool val = true)    {_state.SS_ISCONNECTING    = val;}
  void setSS_ISDISCONNECTING(bool val = true) {_state.SS_ISDISCONNECTING = val;}
  void setSS_NOFDREF(bool val = true)         {_state.SS_NOFDREF         = val;}
  void setSS_PRIV(bool val = true)            {_state.SS_PRIV            = val;}
  void setSS_RCVATMARK(bool val = true)       {_state.SS_RCVATMARK       = val;}

  bool SS_CANTRCVMORE()     {return _state.SS_CANTRCVMORE;}
  bool SS_CANTSENDMORE()    {return _state.SS_CANTSENDMORE;}
  bool SS_ISCONFIRMING()    {return _state.SS_ISCONFIRMING;}
  bool SS_ISCONNECTED()     {return _state.SS_ISCONNECTED;}
  bool SS_ISCONNECTING()    {return _state.SS_ISCONNECTING;}
  bool SS_ISDISCONNECTING() {return _state.SS_ISDISCONNECTING;}
  bool SS_NOFDREF()         {return _state.SS_NOFDREF;}
  bool SS_PRIV()            {return _state.SS_PRIV;}
  bool SS_RCVATMARK()       {return _state.SS_RCVATMARK;}

  void setConnState(enum ConnectionState state) {_connstate = state;}
  ConnectionState connState() const {return _connstate;}

  void setPendingAccept(bool val = true) {_pending_accept = val;}
  bool pendingAccept() {return _pending_accept;}
  bool isFullySpecified();
};

// helper functions
PCB* _createPCB(Socket::Protocol proto);


#endif // SOCKET_H
