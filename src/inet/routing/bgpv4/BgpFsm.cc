//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/bgpv4/BgpFsm.h"

#include "inet/routing/bgpv4/BgpSession.h"

namespace inet {

namespace bgp {

namespace fsm {

// TopState
void TopState::init()
{
    setState<Idle>();
}

// RFC 4271 - 8.2.2.  Finite State Machine - IdleState
void Idle::ManualStart()
{
    EV_INFO << "Processing Idle::event1" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // In this state, BGP FSM refuses all incoming BGP connections for this peer.
    // No resources are allocated to the peer.  In response to a ManualStart event
    // (Event 1), the local system:
    // - initializes all BGP resources for the peer connection,
    // - sets ConnectRetryCounter to zero,
    session._connectRetryCounter = 0;
    // - starts the ConnectRetryTimer with the initial value,
    session.restartsConnectRetryTimer();
    // - listens for a connection that may be initiated by the remote BGP peer,
    session.listenConnectionFromPeer();
    // - initiates a TCP connection to the other BGP peer and,
    session.openTCPConnectionToPeer();
    // - changes its state to Connect.
    setState<Connect>();
}

// RFC 4271 - 8.2.2.  Finite State Machine - ConnectState
void Connect::ConnectRetryTimer_Expires()
{
    EV_INFO << "Processing Connect::ConnectRetryTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // In response to the ConnectRetryTimer_Expires event (Event 9), the local system:
    // - drops the TCP connection,
    session._info.socket->abort();
    // - restarts the ConnectRetryTimer,
    session.restartsConnectRetryTimer();
    // - initiates a TCP connection to the other BGP peer,
//    session._info.socket->renewSocket();
    session.openTCPConnectionToPeer();
    // - continues to listen for a connection that may be initiated by the remote BGP peer, and
    // - stays in the Connect state.
    setState<Connect>();
}

void Connect::HoldTimer_Expires()
{
    EV_INFO << "Processing Connect::HoldTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // In response to any other events (Events 8, 10-11, 13, 19, 23, 25-28), the local system:
    // - if the ConnectRetryTimer is running, stops and resets the ConnectRetryTimer (sets to zero),
    if (session._ptrConnectRetryTimer->isScheduled()) {
        session.restartsConnectRetryTimer(false);
    }
    // - if the DelayOpenTimer is running, stops and resets the DelayOpenTimer (sets to zero),
    // - releases all BGP resources,
    // - drops the TCP connection,
    session._info.socket->abort();
    // - increments the ConnectRetryCounter by 1,
    ++session._connectRetryCounter;
    // - performs peer oscillation damping if the DampPeerOscillations attribute is set to True, and
    // - changes its state to Idle.
    setState<Idle>();
}

void Connect::KeepaliveTimer_Expires()
{
    EV_INFO << "Processing Connect::KeepaliveTimer_Expires" << std::endl;
    Connect::HoldTimer_Expires();
}

void Connect::TcpConnectionConfirmed()
{
    EV_INFO << "Processing Connect::TcpConnectionConfirmed" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // If the TCP connection succeeds (Event 16 or Event 17), the local
    // system checks the DelayOpen attribute prior to processing.
    // If the DelayOpen attribute is set to FALSE, the local system:
    // - stops the ConnectRetryTimer (if running) and sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    // - completes BGP initialization
    // - sends an OPEN message to its peer,
    session.sendOpenMessage();
    // - sets the HoldTimer to a large value, and
    session.restartsHoldTimer();
    // - changes its state to OpenSent.
    setState<OpenSent>();
}

void Connect::TcpConnectionFails()
{
    setState<Active>();
}

void Connect::KeepAliveMsgEvent()
{
    EV_INFO << "Processing Connect::KeepAliveMsgEvent" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._keepAliveMsgRcv++;
    Connect::HoldTimer_Expires();
}

void Connect::UpdateMsgEvent()
{
    EV_INFO << "Processing Connect::UpdateMsgEvent" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._updateMsgRcv++;
    Connect::HoldTimer_Expires();
}

// RFC 4271 - 8.2.2.  Finite State Machine - ActiveState
void Active::ConnectRetryTimer_Expires()
{
    EV_INFO << "Processing Active::ConnectRetryTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // In response to a ConnectRetryTimer_Expires event (Event 9), the local system:
    // - restarts the ConnectRetryTimer (with initial value),
    session.restartsConnectRetryTimer();
    // - initiates a TCP connection to the other BGP peer,T);
//    session._info.socket->renewSocket();
    session.openTCPConnectionToPeer();
    // - continues to listen for a TCP connection that may be initiated by a remote BGP peer, and
    // - changes its state to Connect.
    setState<Connect>();
}

void Active::HoldTimer_Expires()
{
    EV_INFO << "Processing Active::HoldTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // In response to any other event (Events 8, 10-11, 13, 19, 23, 25-28), the local system:
    // - sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    // - releases all BGP resources,
    // - drops the TCP connection,
    session._info.socket->abort();
    // - increments the ConnectRetryCounter by one,
    ++session._connectRetryCounter;
    // - changes its state to Idle.
    setState<Idle>();
}

void Active::KeepaliveTimer_Expires()
{
    EV_TRACE << "Processing Active::KeepaliveTimer_Expires" << std::endl;
    Active::HoldTimer_Expires();
}

void Active::TcpConnectionConfirmed()
{
    EV_TRACE << "Processing Active::TcpConnectionConfirmed" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // In response to the success of a TCP connection (Event 16 or Event
    // 17), the local system :
    // - sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    // - sends the OPEN message to its peer,
    session.sendOpenMessage();
    // - sets its HoldTimer to a large value, and
    session.restartsHoldTimer();
    // - changes its state to OpenSent.
    setState<OpenSent>();
}

void Active::TcpConnectionFails()
{
    setState<Idle>();
}

void Active::OpenMsgEvent()
{
    EV_TRACE << "Processing Active::BgpOpen" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._openMsgRcv++;
    Active::HoldTimer_Expires();
}

void Active::KeepAliveMsgEvent()
{
    EV_TRACE << "Processing Active::KeepAliveMsgEvent" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._keepAliveMsgRcv++;
    Active::HoldTimer_Expires();
}

void Active::UpdateMsgEvent()
{
    EV_TRACE << "Processing Active::UpdateMsgEvent" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._updateMsgRcv++;
    Active::HoldTimer_Expires();
}

// RFC 4271 - 8.2.2.  Finite State Machine - OpenSentState
void OpenSent::ConnectRetryTimer_Expires()
{
    EV_TRACE << "Processing OpenSent::ConnectRetryTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // In response to any other event (Events 9, 11-13, 20, 25-28), the local system:
    // TODO- sends the NOTIFICATION with the Error Code Finite State Machine Error,
    // - sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    // - releases all BGP resources,
    // - drops the TCP connection,
    session._info.socket->abort();
    // - increments the ConnectRetryCounter by one,
    ++session._connectRetryCounter;
    // - (optionally) performs peer oscillation damping if the DampPeerOscillations attribute is set to TRUE, and
    // - changes its state to Idle.
    setState<Idle>();
}

void OpenSent::HoldTimer_Expires()
{
    EV_TRACE << "Processing OpenSent::HoldTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // If the HoldTimer_Expires (Event 10), the local system:
    // - sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    // - releases all BGP resources,
    // - drops the TCP connection,
    session._info.socket->abort();
    // - increments the ConnectRetryCounter,
    ++session._connectRetryCounter;
    // - (optionally) performs peer oscillation damping if the DampPeerOscillations attribute is set to TRUE, and
    // - changes its state to Idle.
    setState<Idle>();
}

void OpenSent::KeepaliveTimer_Expires()
{
    EV_TRACE << "Processing OpenSent::KeepaliveTimer_Expires" << std::endl;
    OpenSent::ConnectRetryTimer_Expires();
}

void OpenSent::TcpConnectionFails()
{
    EV_TRACE << "Processing OpenSent::BgpOpen" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // If a TcpConnectionFails event (Event 18) is received, the local system:
    // - closes the BGP connection,
    session._info.socket->abort();
    // - restarts the ConnectRetryTimer,
    session.restartsConnectRetryTimer();
    // - continues to listen for a connection that may be initiated by the remote BGP peer, and
    session.listenConnectionFromPeer();
    // - changes its state to Active.
    setState<Active>();
}

void OpenSent::OpenMsgEvent()
{
    EV_TRACE << "Processing OpenSent::BgpOpen" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._openMsgRcv++;
    // When an OPEN message is received, all fields are checked for correctness.
    // If there are no errors in the OPEN message (Event 19), the local system:
    // - sets the BGP ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    // - sends a KEEPALIVE message, and
    session.sendKeepAliveMessage();
    // - sets a KeepaliveTimer
    session.restartsKeepAliveTimer();
    // - sets the HoldTimer according to the negotiated value (see Section 4.2),
    session.restartsHoldTimer();
    // - changes its state to OpenConfirm.
    setState<OpenConfirm>();
}

void OpenSent::KeepAliveMsgEvent()
{
    EV_TRACE << "Processing OpenSent::KeepAliveMsgEvent" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._keepAliveMsgRcv++;
    OpenSent::ConnectRetryTimer_Expires();
}

void OpenSent::UpdateMsgEvent()
{
    EV_TRACE << "Processing OpenSent::UpdateMsgEvent" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._updateMsgRcv++;
    OpenSent::ConnectRetryTimer_Expires();
}

// RFC 4271 - 8.2.2.  Finite State Machine - OpenConfirmState
void OpenConfirm::ConnectRetryTimer_Expires()
{
    EV_TRACE << "Processing OpenConfirm::ConnectRetryTimer_Expires" << std::endl;
    // In response to any other event (Events 9, 12-13, 20, 27-28), the local system:
    BgpSession& session = TopState::box().getModule();
    // - sends a NOTIFICATION with a Cease,
    // - sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    // - releases all BGP resources,
    // - drops the TCP connection (send TCP FIN),
    session._info.socket->abort();
    // - increments the ConnectRetryCounter by 1,
    ++session._connectRetryCounter;
    // - changes its state to Idle.
    setState<Idle>();
}

void OpenConfirm::HoldTimer_Expires()
{
    EV_TRACE << "Processing OpenConfirm::HoldTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // If the HoldTimer_Expires event (Event 10) occurs before a KEEPALIVE message is received, the local system:
    // - sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    // - releases all BGP resources,
    // - drops the TCP connection,
    session._info.socket->abort();
    // - increments the ConnectRetryCounter by 1,
    ++session._connectRetryCounter;
    // - changes its state to Idle.
    setState<Idle>();
}

void OpenConfirm::KeepaliveTimer_Expires()
{
    EV_TRACE << "Processing OpenConfirm::KeepaliveTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // If the local system receives a KeepaliveTimer_Expires event (Event 11), the local system:
    // - sends a KEEPALIVE message,
    session.sendKeepAliveMessage();
    // - restarts the KeepaliveTimer, and
    session.restartsKeepAliveTimer();
    // - remains in the OpenConfirmed state.
}

void OpenConfirm::TcpConnectionFails()
{
    EV_TRACE << "OpenConfirm::TcpConnectionFails" << std::endl;
    setState<Idle>();
}

void OpenConfirm::OpenMsgEvent()
{
    EV_TRACE << "Processing OpenConfirm::BgpOpen - collision" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._openMsgRcv++;
    // If the local system receives a valid OPEN message (BgpOpen (Event 19)),
    // the collision detect function is processed per Section 6.8.
    // TODO session._bgpRouting.collisionDetection();
    // If this connection is to be dropped due to connection collision, the local system:
    OpenConfirm::ConnectRetryTimer_Expires();
}

void OpenConfirm::KeepAliveMsgEvent()
{
    EV_TRACE << "Processing OpenConfirm::KeepAliveMsgEvent" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._keepAliveMsgRcv++;
    // If the local system receives a KEEPALIVE message (KeepAliveMsg Event 26)), the local system:
    // - restarts the HoldTimer and
    session.restartsHoldTimer();
    // - changes its state to Established.
    setState<Established>();
}

void OpenConfirm::UpdateMsgEvent()
{
    EV_TRACE << "Processing OpenConfirm::UpdateMsgEvent" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._updateMsgRcv++;
    OpenConfirm::ConnectRetryTimer_Expires();
}

// RFC 4271 - 8.2.2.  Finite State Machine - Established State
void Established::entry()
{
    EV_DEBUG << "BGP session is established. \n";

    BgpSession& session = TopState::box().getModule();
    session._info.sessionEstablished = true;

    // if it's an EGP Session, send update messages with all routing information to BGP peer
    // if it's an IGP Session, send update message with only the BGP routes learned by EGP

    if (session.getType() == EGP) {
        auto IPRoutingTable = session.getIPRoutingTable();
        for (int i = 0; i < IPRoutingTable->getNumRoutes(); i++) {
            const Ipv4Route *rtEntry = IPRoutingTable->getRoute(i);
            if (session.isRouteExcluded(*rtEntry))
                continue;
            BgpRoutingTableEntry *BGPEntry = new BgpRoutingTableEntry(rtEntry);
            BGPEntry->addAS(session._info.ASValue);
            session.updateSendProcess(BGPEntry);
            delete BGPEntry;
        }
    }

    for (auto& elem : session.getBGPRoutingTable())
        session.updateSendProcess(elem);

    // when all EGP Sessions are in established state, start IGP Session(s)
    SessionId nextSession = session.findAndStartNextSession(EGP);
    if (nextSession == static_cast<SessionId>(-1)) {
        EV_DEBUG << ">>> all EGP session are established. Starting IGP sessions(s). \n";
        session.findAndStartNextSession(IGP);
    }
}

void Established::ConnectRetryTimer_Expires()
{
    EV_TRACE << "Processing Established::ConnectRetryTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // In response to any other event (Events 9, 12-13, 20-22), the local system:
    // TODO- deletes all routes associated with this connection,
    // - sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    // - releases all BGP resources,
    // - drops the TCP connection,
    session._info.socket->abort();
    // - increments the ConnectRetryCounter by 1,
    ++session._connectRetryCounter;
    // - changes its state to Idle.
    setState<Idle>();
}

void Established::HoldTimer_Expires()
{
    EV_TRACE << "Processing Established::HoldTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // If the HoldTimer_Expires event occurs (Event 10), the local system:
    // - sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    // - releases all BGP resources,
    // - drops the TCP connection,
    session._info.socket->abort();
    // - increments the ConnectRetryCounter by 1,
    ++session._connectRetryCounter;
    // - changes its state to Idle.
    setState<Idle>();
}

void Established::KeepaliveTimer_Expires()
{
    EV_TRACE << "Processing Established::KeepaliveTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    // If the KeepaliveTimer_Expires event occurs (Event 11), the local system:
    // - sends a KEEPALIVE message, and
    session.sendKeepAliveMessage();
    // - restarts the KeepaliveTimer, unless the negotiated HoldTime value is zero.
    if (session._holdTime != 0) {
        session.restartsKeepAliveTimer();
    }
}

void Established::TcpConnectionFails()
{
    EV_TRACE << "Processing Established::TcpConnectionFails" << std::endl;
}

void Established::OpenMsgEvent()
{
    EV_TRACE << "Processing Established::BgpOpen" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._openMsgRcv++;
}

void Established::KeepAliveMsgEvent()
{
    EV_TRACE << "Processing Established::KeepAliveMsgEvent" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._keepAliveMsgRcv++;
    // If the local system receives a KEEPALIVE message (Event 26), the local system:
    // - restarts its HoldTimer, if the negotiated HoldTime value is non-zero, and
    session.restartsHoldTimer();
    // - remains in the Established state.
}

void Established::UpdateMsgEvent()
{
    EV_TRACE << "Processing Established::UpdateMsgEvent" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._updateMsgRcv++;
    // If the local system receives an UPDATE message (Event 27), the local system:
    // - processes the message,
    // - restarts its HoldTimer, if the negotiated HoldTime value is non-zero, and
    session.restartsHoldTimer();
    // - remains in the Established state.
}

} // namespace fsm

} // namespace bgp

} // namespace inet

