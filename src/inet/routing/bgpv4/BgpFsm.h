//
// Copyright (C) 2010 Helene Lageber
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

#ifndef __INET_BGPFSM_H
#define __INET_BGPFSM_H

#include <iostream>

#include "inet/common/INETDefs.h"
#include "inet/common/Macho.h"

namespace inet {

namespace bgp {

class BgpSession;

namespace fsm {

////////////////////////////////////////////////////////
// State declarations

// Machine's top state
TOPSTATE(TopState) {
    struct Box
    {
        Box() : _mod(0) {}
        Box(BgpSession& session) : _mod(&session) {}
        BgpSession& getModule() { return *_mod; }

      private:
        BgpSession *_mod;
    };

    STATE(TopState)

    // Machine's event protocol
    // RFC 4271, 8.1.2.  Administrative Events
    // -------------------------------------
    /*Event 1: ManualStart
       Definition: Local system administrator manually starts the peer connection.
       Status:     Mandatory*/
    virtual void ManualStart() {}
    /*Event 2: ManualStop
       Definition: Local system administrator manually stops the peer connection.
       Status:     Mandatory*/
    //virtual void event2() {}

    // RFC 4271, 8.1.3.  Timer Events
    // -------------------------------------
    /*Event 9: ConnectRetryTimer_Expires
       Definition: An event generated when the ConnectRetryTimer
                 expires.
       Status:     Mandatory*/
    virtual void ConnectRetryTimer_Expires() {}
    /*Event 10: HoldTimer_Expires
       Definition: An event generated when the HoldTimer expires.
       Status:     Mandatory*/
    virtual void HoldTimer_Expires() {}
    /*Event 11: KeepaliveTimer_Expires
       Definition: An event generated when the KeepaliveTimer expires.
       Status:     Mandatory*/
    virtual void KeepaliveTimer_Expires() {}

    //RFC 4271, 8.1.4.  TCP Connection-Based Events
    // -------------------------------------
    /*Event 16: Tcp_CR_Acked
       Definition: Event indicating the local system's request to
                 establish a TCP connection to the remote peer.
                 The local system's TCP connection sent a TCP SYN,
                 received a TCP SYN/ACK message, and sent a TCP ACK.
       Status:     Mandatory*/
    //virtual void Tcp_CR_Acked() {}
    /*Event 17: TcpConnectionConfirmed
       Definition: Event indicating that the local system has received
                 a confirmation that the TCP connection has been
                 established by the remote site.
                 The remote peer's TCP engine sent a TCP SYN.  The
                 local peer's TCP engine sent a SYN, ACK message and
                 now has received a final ACK.
       Status:     Mandatory*/
    virtual void TcpConnectionConfirmed() {}
    /*Event 18: TcpConnectionFails
       Definition: Event indicating that the local system has received
                 a TCP connection failure notice.
                 The remote BGP peer's TCP machine could have sent a
                 FIN.  The local peer would respond with a FIN-ACK.
                 Another possibility is that the local peer
                 indicated a timeout in the TCP connection and
                 downed the connection.
       Status:     Mandatory*/
    virtual void TcpConnectionFails() {}

    //RFC 4271, 8.1.5.  BGP Message-Based Events
    // -------------------------------------
    /*Event 19: OpenMsgEvent
       Definition: An event is generated when a valid OPEN message has been received.
       Status:     Mandatory*/
    virtual void OpenMsgEvent() {}
    /*Event 26: KeepAliveMsgEvent
       Definition: An event is generated when a KEEPALIVE message is received.
       Status:     Mandatory*/
    virtual void KeepAliveMsgEvent() {}
    /*Event 27: UpdateMsgEvent
       Definition: An event is generated when a valid UPDATE message is received.
       Status:     Mandatory*/
    virtual void UpdateMsgEvent() {}

  private:
    void init() override;
};

// A superstate
SUBSTATE(Idle, TopState) {
    STATE(Idle)

    void ManualStart() override;

  private:
    void entry() override { EV_STATICCONTEXT; EV_DEBUG << "Idle::entry" << std::endl; }
    void exit() override { EV_STATICCONTEXT; EV_DEBUG << "Idle::exit" << std::endl; }
};

// A substate
SUBSTATE(Connect, TopState) {
    STATE(Connect)

    void ConnectRetryTimer_Expires() override;
    void HoldTimer_Expires() override;
    void KeepaliveTimer_Expires() override;
    void TcpConnectionConfirmed() override;
    void TcpConnectionFails() override;
    void KeepAliveMsgEvent() override;
    void UpdateMsgEvent() override;

  private:
    void entry() override { EV_STATICCONTEXT; EV_DEBUG << "Connect::entry" << std::endl; }
    void exit() override { EV_STATICCONTEXT; EV_DEBUG << "Connect::exit" << std::endl; }
};

// A substate
SUBSTATE(Active, TopState) {
    STATE(Active)

    void ConnectRetryTimer_Expires() override;
    void HoldTimer_Expires() override;
    void KeepaliveTimer_Expires() override;
    void TcpConnectionConfirmed() override;
    void TcpConnectionFails() override;
    void OpenMsgEvent() override;
    void KeepAliveMsgEvent() override;
    void UpdateMsgEvent() override;

  private:
    void entry() override { EV_STATICCONTEXT; EV_DEBUG << "Active::entry" << std::endl; }
    void exit() override { EV_STATICCONTEXT; EV_DEBUG << "Active::exit" << std::endl; }
};

// A substate
SUBSTATE(OpenSent, TopState) {
    STATE(OpenSent)

    void ConnectRetryTimer_Expires() override;
    void HoldTimer_Expires() override;
    void KeepaliveTimer_Expires() override;
    void TcpConnectionFails() override;
    void OpenMsgEvent() override;
    void KeepAliveMsgEvent() override;
    void UpdateMsgEvent() override;

  private:
    void entry() override { EV_STATICCONTEXT; EV_DEBUG << "OpenSent::entry" << std::endl; }
    void exit() override { EV_STATICCONTEXT; EV_DEBUG << "OpenSent::exit" << std::endl; }
};

// A substate
SUBSTATE(OpenConfirm, TopState) {
    STATE(OpenConfirm)

    void ConnectRetryTimer_Expires() override;
    void HoldTimer_Expires() override;
    void KeepaliveTimer_Expires() override;
    void TcpConnectionFails() override;
    void OpenMsgEvent() override;
    void KeepAliveMsgEvent() override;
    void UpdateMsgEvent() override;

  private:
    void entry() override { EV_STATICCONTEXT; EV_DEBUG << "OpenConfirm::entry" << std::endl; }
    void exit() override { EV_STATICCONTEXT; EV_DEBUG << "OpenConfirm::exit" << std::endl; }
};

// A substate
SUBSTATE(Established, TopState) {
    STATE(Established)

    void ConnectRetryTimer_Expires() override;
    void HoldTimer_Expires() override;
    void KeepaliveTimer_Expires() override;
    void TcpConnectionFails() override;
    void OpenMsgEvent() override;
    void KeepAliveMsgEvent() override;
    void UpdateMsgEvent() override;

  private:
    void entry() override;
    void exit() override { EV_STATICCONTEXT; EV_DEBUG << "Established::exit" << std::endl; }
};

} // namespace fsm

} // namespace bgp

} // namespace inet

#endif // ifndef __INET_BGPFSM_H

