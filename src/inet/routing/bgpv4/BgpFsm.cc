//
// Copyright (C) 2010 Helene Lageber
//    2019 Adrian Novak - multi address-family support
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

#include "inet/routing/bgpv4/BgpFsm.h"
#include "inet/routing/bgpv4/BgpSession.h"

namespace inet {

namespace bgp {

namespace fsm {

//TopState
void TopState::init()
{
    setState<Idle>();
}

//RFC 4271 - 8.2.2.  Finite State Machine - IdleState
void Idle::entry()
{
    std::cout << "Idle::entry" << std::endl;
    BgpSession& session = TopState::box().getModule();

    if (session._connectRetryCounter != 0) {
        session._info.sessionEstablished = false;
        session.restartConnection();
    }
}

void Idle::ManualStart()
{
    EV_INFO << "Processing Idle::event1" << std::endl;
    BgpSession& session = TopState::box().getModule();

    //In this state, BGP FSM refuses all incoming BGP connections for this peer.
    //No resources are allocated to the peer.  In response to a ManualStart event
    //(Event 1), the local system:
    //- initializes all BGP resources for the peer connection,
    //- sets ConnectRetryCounter to zero,
    session._connectRetryCounter = 0;
    //- starts the ConnectRetryTimer with the initial value,
    session.restartsConnectRetryTimer();
    //- listens for a connection that may be initiated by the remote BGP peer,
    session.listenConnectionFromPeer();
    //- initiates a TCP connection to the other BGP peer and,
    session.openTCPConnectionToPeer();
    //- changes its state to Connect.
    setState<Connect>();
}

//RFC 4271 - 8.2.2.  Finite State Machine - ConnectState
void Connect::ConnectRetryTimer_Expires()
{
    EV_INFO << "Processing Connect::ConnectRetryTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    //In response to the ConnectRetryTimer_Expires event (Event 9), the local system:
    //- drops the TCP connection,
    session._info.socket->abort();
    //- restarts the ConnectRetryTimer,
    session.restartsConnectRetryTimer();
    //- initiates a TCP connection to the other BGP peer,
    //session._info.socket->renewSocket();
    session.openTCPConnectionToPeer();
    //- continues to listen for a connection that may be initiated by the remote BGP peer, and
    //- stays in the Connect state.
    setState<Connect>();
}

void Connect::HoldTimer_Expires()
{
    EV_INFO << "Processing Connect::HoldTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    //In response to any other events (Events 8, 10-11, 13, 19, 23, 25-28), the local system:
    //- if the ConnectRetryTimer is running, stops and resets the ConnectRetryTimer (sets to zero),
    if (session._ptrConnectRetryTimer->isScheduled()) {
        session.restartsConnectRetryTimer(false);
    }
    //- if the DelayOpenTimer is running, stops and resets the DelayOpenTimer (sets to zero),
    //- releases all BGP resources,
    //- drops the TCP connection,
    session._info.socket->abort();
    //- increments the ConnectRetryCounter by 1,
    session._connectRetryCounter++;
    //- performs peer oscillation damping if the DampPeerOscillations attribute is set to True, and
    //- changes its state to Idle.
    setState<Idle>();
}

void Connect::KeepaliveTimer_Expires()
{
    EV_INFO << "Processing Connect::KeepaliveTimer_Expires" << std::endl;
    Connect::HoldTimer_Expires();
}

void Connect::TcpConnectionConfirmed()
{   //delayOpen attribute

    EV_INFO << "Processing Connect::TcpConnectionConfirmed" << std::endl;
    BgpSession& session = TopState::box().getModule();
    //If the TCP connection succeeds (Event 16 or Event 17), the local
    //system checks the DelayOpen attribute prior to processing.
    //If the DelayOpen attribute is set to FALSE, the local system:
    //- stops the ConnectRetryTimer (if running) and sets the ConnectRetryTimer to zero,
    if (session._ptrConnectRetryTimer->isScheduled()) {
        session.restartsConnectRetryTimer(false);
    }
    //- completes BGP initialization
    //- sends an OPEN message to its peer,
    session.sendOpenMessage();
    //- sets the HoldTimer to a large value, and
    session.restartsHoldTimer();
    //- changes its state to OpenSent.
    setState<OpenSent>();
}

void Connect::TcpConnectionFails()
{
    //delayOpenTimer ??
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

//RFC 4271 - 8.2.2.  Finite State Machine - ActiveState
void Active::ConnectRetryTimer_Expires()
{
    EV_INFO << "Processing Active::ConnectRetryTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    //In response to a ConnectRetryTimer_Expires event (Event 9), the local system:
    //- restarts the ConnectRetryTimer (with initial value),
    session.restartsConnectRetryTimer();
    //- initiates a TCP connection to the other BGP peer,T);
    //session._info.socket->renewSocket();
    session.openTCPConnectionToPeer();
    //- continues to listen for a TCP connection that may be initiated by a remote BGP peer, and
    //- changes its state to Connect.
    setState<Connect>();
}

void Active::HoldTimer_Expires()
{
    EV_INFO << "Processing Active::HoldTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    //In response to any other event (Events 8, 10-11, 13, 19, 23, 25-28), the local system:
    //- sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- releases all BGP resources,
    //- drops the TCP connection,
    session._info.socket->abort();
    //- increments the ConnectRetryCounter by one,
    session._connectRetryCounter++;
    //- changes its state to Idle.
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
    //In response to the success of a TCP connection (Event 16 or Event
    //17), the local system :
    //- sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- sends the OPEN message to its peer,
    session.sendOpenMessage();
    //- sets its HoldTimer to a large value, and
    session.restartsHoldTimer();
    //- changes its state to OpenSent.
    setState<OpenSent>();
}

void Active::TcpConnectionFails()
{
    /*If the local system receives a TcpConnectionFails event (Event
      18), the local system:
     *  restarts the ConnectRetryTimer (with the initial value),
     *  releases all BGP resource,
     *  increments the ConnectRetryCounter by 1
     *  changes its state to Idle
     * */
    BgpSession& session = TopState::box().getModule();
    session.restartsConnectRetryTimer();
    session._connectRetryCounter++;
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

//RFC 4271 - 8.2.2.  Finite State Machine - OpenSentState
void OpenSent::ConnectRetryTimer_Expires()
{
    EV_TRACE << "Processing OpenSent::ConnectRetryTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    //In response to any other event (Events 9, 11-13, 20, 25-28), the local system:
    //TODO- sends the NOTIFICATION with the Error Code Finite State Machine Error,
    //- sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- releases all BGP resources,
    //- drops the TCP connection,
    session._info.socket->abort();
    //- increments the ConnectRetryCounter by one,
    session._connectRetryCounter++;
    //- (optionally) performs peer oscillation damping if the DampPeerOscillations attribute is set to TRUE, and
    //- changes its state to Idle.
    setState<Idle>();
}

void OpenSent::HoldTimer_Expires()
{
    EV_TRACE << "Processing OpenSent::HoldTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    //If the HoldTimer_Expires (Event 10), the local system:
    //- sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- releases all BGP resources,
    //- drops the TCP connection,
    session._info.socket->abort();
    //- increments the ConnectRetryCounter,
    session._connectRetryCounter++;
    //- (optionally) performs peer oscillation damping if the DampPeerOscillations attribute is set to TRUE, and
    //- changes its state to Idle.
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
    //If a TcpConnectionFails event (Event 18) is received, the local system:
    //- closes the BGP connection,
    session._info.socket->abort();
    //- restarts the ConnectRetryTimer,
    session.restartsConnectRetryTimer();
    //- continues to listen for a connection that may be initiated by the remote BGP peer, and
    session.listenConnectionFromPeer();
    //- changes its state to Active.
    setState<Active>();
}

void OpenSent::OpenMsgEvent()
{
    EV_TRACE << "Processing OpenSent::BgpOpen" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._openMsgRcv++;
    //When an OPEN message is received, all fields are checked for correctness.
    //If there are no errors in the OPEN message (Event 19), the local system:
    //- sets the BGP ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- sends a KEEPALIVE message, and
    session.sendKeepAliveMessage();
    //- sets a KeepaliveTimer
    session.restartsKeepAliveTimer();
    //- sets the HoldTimer according to the negotiated value (see Section 4.2),
    session.restartsHoldTimer();
    //- changes its state to OpenConfirm.
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

//RFC 4271 - 8.2.2.  Finite State Machine - OpenConfirmState
void OpenConfirm::ConnectRetryTimer_Expires()
{
    std::cout << "OpenConfirm::ConnectRetryTimer_Expires" << std::endl;
    EV_TRACE << "Processing OpenConfirm::ConnectRetryTimer_Expires" << std::endl;
    //In response to any other event (Events 9, 12-13, 20, 27-28), the local system:
    BgpSession& session = TopState::box().getModule();
    //- sends a NOTIFICATION with a Cease,
    //- sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- releases all BGP resources,
    //- drops the TCP connection (send TCP FIN),
    session._info.socket->abort();
    //- increments the ConnectRetryCounter by 1,
    session._connectRetryCounter++;
    //- changes its state to Idle.
    setState<Idle>();
}

void OpenConfirm::HoldTimer_Expires()
{
    std::cout << "OpenConfirm::HoldTimer_Expires" << std::endl;
    EV_TRACE << "Processing OpenConfirm::HoldTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    //If the HoldTimer_Expires event (Event 10) occurs before a KEEPALIVE message is received, the local system:
    //- sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- releases all BGP resources,
    //- drops the TCP connection,
    session._info.socket->abort();
    //- increments the ConnectRetryCounter by 1,
    session._connectRetryCounter++;
    //- changes its state to Idle.
    setState<Idle>();
}

void OpenConfirm::KeepaliveTimer_Expires()
{
    std::cout << "OpenConfirm::KeepaliveTimer_Expires" << std::endl;
    EV_TRACE << "Processing OpenConfirm::KeepaliveTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    //If the local system receives a KeepaliveTimer_Expires event (Event 11), the local system:
    //- sends a KEEPALIVE message,
    session.sendKeepAliveMessage();
    //- restarts the KeepaliveTimer, and
    session.restartsKeepAliveTimer();
    //- remains in the OpenConfirmed state.
}

void OpenConfirm::TcpConnectionFails()
{
//    BgpSession& session = TopState::box().getModule();
//    session.restartsConnectRetryTimer(false);
    /*xnovak1j -  If the local system receives a TcpConnectionFails event (Event 18)
      from the underlying TCP or a NOTIFICATION message (Event 25), the
      local system
     * sets the ConnectRetryTimer to zero
     * releases all BGP resources
     * drops the TCP connection
     * increments the ConnectRetryCounter by 1
     * changes its state to Idle
     * */
    std::cout << "OpenConfirm::TcpConnectionFails" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session.restartsConnectRetryTimer();
    session._connectRetryCounter++;
    setState<Idle>();
}

void OpenConfirm::OpenMsgEvent()
{
    EV_TRACE << "Processing OpenConfirm::BgpOpen - collision" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._openMsgRcv++;
    //If the local system receives a valid OPEN message (BgpOpen (Event 19)),
    //the collision detect function is processed per Section 6.8.
//TODO session._bgpRouting.collisionDetection();
    //If this connection is to be dropped due to connection collision, the local system:
    OpenConfirm::ConnectRetryTimer_Expires();
}

void OpenConfirm::KeepAliveMsgEvent()
{
    std::cout << "OpenConfirm::KeepAliveMsgEvent" << std::endl;
    EV_TRACE << "Processing OpenConfirm::KeepAliveMsgEvent" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._keepAliveMsgRcv++;
    //If the local system receives a KEEPALIVE message (KeepAliveMsg Event 26)), the local system:
    //- restarts the HoldTimer and
    session.restartsHoldTimer();
    //- changes its state to Established.
    setState<Established>();
}

void OpenConfirm::UpdateMsgEvent()
{
    std::cout << "OpenConfirm::UpdateMsgEvent" << std::endl;
    EV_TRACE << "Processing OpenConfirm::UpdateMsgEvent" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._updateMsgRcv++;
    OpenConfirm::ConnectRetryTimer_Expires();
}

//RFC 4271 - 8.2.2.  Finite State Machine - Established State
void Established::entry()
{
    //if it's an EGP Session, send update messages with all routing information to BGP peer
    //if it's an IGP Session, send update message with only the BGP routes learned by EGP
    BgpSession& session = TopState::box().getModule();
    session._info.sessionEstablished = true;

    if(session._info.multiAddress) {
        std::cout << "Established::entry - send an Ipv6 update message" << std::endl;
        const Ipv6Route *rtEntry6;
        RoutingTableEntry6 *BGPEntry6;
        Ipv6RoutingTable *IPRoutingTable6 = session.getIPRoutingTable6();
        std::vector<Ipv6Address> networksToAdvertise6 = session.getNetworksToAdvertise6();

//        for (auto network : networksToAdvertise6) {
//            int i = session.isInRoutingTable6(network);
//            if(i != -1) {
//                rtEntry6 = IPRoutingTable6->getRoute(i);
//                if (session.getType() == EGP) {
//                    BGPEntry6 = new RoutingTableEntry6(rtEntry6);
//                    BGPEntry6->addAS(session._info.ASValue);
//
//                    session.updateSendProcess6(BGPEntry6);
//                    delete BGPEntry6;
//                }
//            }
//        }
        std::vector<RoutingTableEntry6 *> BGPRoutingTable6 = session.getBGPRoutingTable6();
        for (auto & elem : BGPRoutingTable6) {
            Ipv6Address tmp;
            //if (elem->getNextHop() != tmp && elem->getASCount() != 0)
                session.updateSendProcess6((elem));
        }
    } else {
        std::cout << "Established::entry - send an Ipv4 update message" << std::endl;

        const Ipv4Route *rtEntry;
        RoutingTableEntry *BGPEntry;
        IIpv4RoutingTable *IPRoutingTable = session.getIPRoutingTable();
        std::vector<Ipv4Address> networksToAdvertise = session.getNetworksToAdvertise();

//        for (auto network : networksToAdvertise) {
//            int i = session.isInRoutingTable(network);
//            if(i != -1) {
//                rtEntry = IPRoutingTable->getRoute(i);
//
//                if (session.getType() == EGP) {
//                        BGPEntry = new RoutingTableEntry(rtEntry);
//                        BGPEntry->addAS(session._info.ASValue);
//                        session.updateSendProcess(BGPEntry);
//                        delete BGPEntry;
//                }
//            }
//        }

        std::vector<RoutingTableEntry *> BGPRoutingTable = session.getBGPRoutingTable();
        for (auto & elem : BGPRoutingTable) {
            //if(elem->getGateway() != Ipv4Address::UNSPECIFIED_ADDRESS && elem->getASCount() != 0)
                session.updateSendProcess((elem));
        }
    }
    //when all EGP Session is in established state, start IGP Session(s)
    SessionId nextSession = session.findAndStartNextSession(EGP);
    if (nextSession == static_cast<SessionId>(-1)) {
        session.findAndStartNextSession(IGP);
    }
}

void Established::ConnectRetryTimer_Expires()
{
    EV_TRACE << "Processing Established::ConnectRetryTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    //In response to any other event (Events 9, 12-13, 20-22), the local system:
// deletes all routes associated with this connection,
    if (session.isMultiAddress()) {
        Ipv6Route *rtEntry;
        for (auto & network : session.getNetworksFromPeer6())
        {
           int i = session.isInRoutingTable6(network);
           if (i != -1) {
               rtEntry = session.getIPRoutingTable6()->getRoute(i);
               session.updateSendWithdrawnProcess6(rtEntry);
           }
        }
    } else {
        Ipv4Route *rtEntry;
        for (auto & network : session.getNetworksFromPeer())
        {
           int i = session.isInRoutingTable(network);
           if (i != -1) {
               rtEntry = session.getIPRoutingTable()->getRoute(i);
               session.updateSendWithdrawnProcess(rtEntry);
           }
        }
    }
    //- sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- releases all BGP resources,
    //- drops the TCP connection,
    session._info.socket->abort();
    //- increments the ConnectRetryCounter by 1,
    session._connectRetryCounter++;
    //- changes its state to Idle.
    setState<Idle>();
}

void Established::HoldTimer_Expires()
{
    EV_TRACE << "Processing Established::HoldTimer_Expires" << std::endl;

    std::cout<< "Established::holdTimer_Expires"<< std::endl;

    BgpSession& session = TopState::box().getModule();
    //If the HoldTimer_Expires event occurs (Event 10), the local system:
    //- sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- releases all BGP resources,

    if (session.isMultiAddress()) {
        Ipv6Route *rtEntry;
        for (auto & network : session.getNetworksFromPeer6())
        {
           int i = session.isInRoutingTable6(network);
           if (i != -1) {
               rtEntry = session.getIPRoutingTable6()->getRoute(i);
               session.updateSendWithdrawnProcess6(rtEntry);
           }
        }
    } else {
        Ipv4Route *rtEntry;
        for (auto & network : session.getNetworksFromPeer())
        {
           int i = session.isInRoutingTable(network);
           if (i != -1) {
               rtEntry = session.getIPRoutingTable()->getRoute(i);
               session.updateSendWithdrawnProcess(rtEntry);
           }
        }
    }
    //- drops the TCP connection,
    session._info.socket->abort();
    //- increments the ConnectRetryCounter by 1,
    session._connectRetryCounter++;
    //- changes its state to Idle.
    setState<Idle>();
}

void Established::KeepaliveTimer_Expires()
{
    EV_TRACE << "Processing Established::KeepaliveTimer_Expires" << std::endl;
    BgpSession& session = TopState::box().getModule();
    //If the KeepaliveTimer_Expires event occurs (Event 11), the local system:
    //- sends a KEEPALIVE message, and
    session.sendKeepAliveMessage();
    //- restarts the KeepaliveTimer, unless the negotiated HoldTime value is zero.
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
    //If the local system receives a KEEPALIVE message (Event 26), the local system:
    //- restarts its HoldTimer, if the negotiated HoldTime value is non-zero, and

//!!!!!!!!!!!! testing
    const char * ipv41 = "10.0.12.1";
    const char * ipv61 = "fd00:12:12::0";

    Ipv6Address tmpipv6;
    tmpipv6 = Ipv6Address(ipv61);

    Ipv4Address tmpipv4;
    tmpipv4.set(ipv41);

    session.restartsHoldTimer();

    //- remains in the Established state.
}

void Established::UpdateMsgEvent()
{
    EV_TRACE << "Processing Established::UpdateMsgEvent" << std::endl;
    BgpSession& session = TopState::box().getModule();
    session._updateMsgRcv++;
    //If the local system receives an UPDATE message (Event 27), the local system:
    //- processes the message,
    //- restarts its HoldTimer, if the negotiated HoldTime value is non-zero, and
    session.restartsHoldTimer();
    //- remains in the Established state.
}

void Established::exit()
{
    std::cout << "Established::exit" << std::endl;
}

} // namespace fsm

} // namespace bgp

} // namespace inet

