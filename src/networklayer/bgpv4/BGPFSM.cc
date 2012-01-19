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

#include "BGPFSM.h"
#include "BGPSession.h"

using namespace BGPFSM;

//TopState
void TopState::init()
{
    setState<Idle>();
}

//RFC 4271 - 8.2.2.  Finite State Machine - IdleState
void Idle::ManualStart()
{
    EV << "Processing Idle::event1" << std::endl;
    BGPSession& session = TopState::box().getModule();
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
    EV << "Processing Connect::ConnectRetryTimer_Expires" << std::endl;
    BGPSession& session = TopState::box().getModule();
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
    EV << "Processing Connect::HoldTimer_Expires" << std::endl;
    BGPSession& session = TopState::box().getModule();
    //In response to any other events (Events 8, 10-11, 13, 19, 23, 25-28), the local system:
    //- if the ConnectRetryTimer is running, stops and resets the ConnectRetryTimer (sets to zero),
    if (session._ptrConnectRetryTimer->isScheduled())
    {
        session.restartsConnectRetryTimer(false);
    }
    //- if the DelayOpenTimer is running, stops and resets the DelayOpenTimer (sets to zero),
    //- releases all BGP resources,
    //- drops the TCP connection,
    session._info.socket->abort();
    //- increments the ConnectRetryCounter by 1,
    ++ session._connectRetryCounter;
    //- performs peer oscillation damping if the DampPeerOscillations attribute is set to True, and
    //- changes its state to Idle.
    setState<Idle>();
}
void Connect::KeepaliveTimer_Expires()
{
    EV << "Processing Connect::KeepaliveTimer_Expires" << std::endl;
    Connect::HoldTimer_Expires();
}
void Connect::TcpConnectionConfirmed()
{
    EV << "Processing Connect::TcpConnectionConfirmed" << std::endl;
    BGPSession& session = TopState::box().getModule();
    //If the TCP connection succeeds (Event 16 or Event 17), the local
    //system checks the DelayOpen attribute prior to processing.
    //If the DelayOpen attribute is set to FALSE, the local system:
    //- stops the ConnectRetryTimer (if running) and sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
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
    setState<Active>();
}
void Connect::KeepAliveMsgEvent()
{
    EV << "Processing Connect::KeepAliveMsgEvent" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._keepAliveMsgRcv ++;
    Connect::HoldTimer_Expires();
}
void Connect::UpdateMsgEvent()
{
    EV << "Processing Connect::UpdateMsgEvent" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._updateMsgRcv ++;
    Connect::HoldTimer_Expires();
}

//RFC 4271 - 8.2.2.  Finite State Machine - ActiveState
void Active::ConnectRetryTimer_Expires()
{
    EV << "Processing Active::ConnectRetryTimer_Expires" << std::endl;
    BGPSession& session = TopState::box().getModule();
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
    EV << "Processing Active::HoldTimer_Expires" << std::endl;
    BGPSession& session = TopState::box().getModule();
    //In response to any other event (Events 8, 10-11, 13, 19, 23, 25-28), the local system:
    //- sets the ConnectRetryTimer to zero,
    session._connectRetryTime = 0;
    session.restartsConnectRetryTimer(false);
    //- releases all BGP resources,
    //- drops the TCP connection,
    session._info.socket->abort();
    //- increments the ConnectRetryCounter by one,
    ++ session._connectRetryCounter;
    //- changes its state to Idle.
    setState<Idle>();
}
void Active::KeepaliveTimer_Expires()
{
    EV << "Processing Active::KeepaliveTimer_Expires" << std::endl;
    Active::HoldTimer_Expires();
}
void Active::TcpConnectionConfirmed()
{
    EV << "Processing Active::TcpConnectionConfirmed" << std::endl;
    BGPSession& session = TopState::box().getModule();
    //In response to the success of a TCP connection (Event 16 or Event
    //17), the local system :
    //- sets the ConnectRetryTimer to zero,
    session._connectRetryTime = 0;
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
    setState<Idle>();
}
void Active::OpenMsgEvent()
{
    EV << "Processing Active::BGPOpen" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._openMsgRcv ++;
    Active::HoldTimer_Expires();
}
void Active::KeepAliveMsgEvent()
{
    EV << "Processing Active::KeepAliveMsgEvent" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._keepAliveMsgRcv ++;
    Active::HoldTimer_Expires();
}
void Active::UpdateMsgEvent()
{
    EV << "Processing Active::UpdateMsgEvent" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._updateMsgRcv ++;
    Active::HoldTimer_Expires();
}

//RFC 4271 - 8.2.2.  Finite State Machine - OpenSentState
void OpenSent::ConnectRetryTimer_Expires()
{
    EV << "Processing OpenSent::ConnectRetryTimer_Expires" << std::endl;
    BGPSession& session = TopState::box().getModule();
    //In response to any other event (Events 9, 11-13, 20, 25-28), the local system:
    //TODO- sends the NOTIFICATION with the Error Code Finite State Machine Error,
    //- sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- releases all BGP resources,
    //- drops the TCP connection,
    session._info.socket->abort();
    //- increments the ConnectRetryCounter by one,
    ++ session._connectRetryCounter;
    //- (optionally) performs peer oscillation damping if the DampPeerOscillations attribute is set to TRUE, and
    //- changes its state to Idle.
    setState<Idle>();
}
void OpenSent::HoldTimer_Expires()
{
    EV << "Processing OpenSent::HoldTimer_Expires" << std::endl;
    BGPSession& session = TopState::box().getModule();
    //If the HoldTimer_Expires (Event 10), the local system:
    //- sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- releases all BGP resources,
    //- drops the TCP connection,
    session._info.socket->abort();
    //- increments the ConnectRetryCounter,
    ++ session._connectRetryCounter;
    //- (optionally) performs peer oscillation damping if the DampPeerOscillations attribute is set to TRUE, and
    //- changes its state to Idle.
    setState<Idle>();
}
void OpenSent::KeepaliveTimer_Expires()
{
    EV << "Processing OpenSent::KeepaliveTimer_Expires" << std::endl;
    OpenSent::ConnectRetryTimer_Expires();
}
void OpenSent::TcpConnectionFails()
{
    EV << "Processing OpenSent::BGPOpen" << std::endl;
    BGPSession& session = TopState::box().getModule();
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
    EV << "Processing OpenSent::BGPOpen" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._openMsgRcv ++;
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
    EV << "Processing OpenSent::KeepAliveMsgEvent" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._keepAliveMsgRcv ++;
    OpenSent::ConnectRetryTimer_Expires();
}

void OpenSent::UpdateMsgEvent()
{
    EV << "Processing OpenSent::UpdateMsgEvent" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._updateMsgRcv ++;
    OpenSent::ConnectRetryTimer_Expires();
}

//RFC 4271 - 8.2.2.  Finite State Machine - OpenConfirmState
void OpenConfirm::ConnectRetryTimer_Expires()
{
    std::cout << "OpenConfirm::ConnectRetryTimer_Expires" << std::endl;
    EV << "Processing OpenConfirm::ConnectRetryTimer_Expires" << std::endl;
    //In response to any other event (Events 9, 12-13, 20, 27-28), the local system:
    BGPSession& session = TopState::box().getModule();
    //- sends a NOTIFICATION with a Cease,
    //- sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- releases all BGP resources,
    //- drops the TCP connection (send TCP FIN),
    session._info.socket->abort();
    //- increments the ConnectRetryCounter by 1,
    ++ session._connectRetryCounter;
    //- changes its state to Idle.
    setState<Idle>();
}
void OpenConfirm::HoldTimer_Expires()
{
    std::cout << "OpenConfirm::HoldTimer_Expires" << std::endl;
    EV << "Processing OpenConfirm::HoldTimer_Expires" << std::endl;
    BGPSession& session = TopState::box().getModule();
    //If the HoldTimer_Expires event (Event 10) occurs before a KEEPALIVE message is received, the local system:
    //- sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- releases all BGP resources,
    //- drops the TCP connection,
    session._info.socket->abort();
    //- increments the ConnectRetryCounter by 1,
    ++ session._connectRetryCounter;
    //- changes its state to Idle.
    setState<Idle>();
}
void OpenConfirm::KeepaliveTimer_Expires()
{
    std::cout << "OpenConfirm::KeepaliveTimer_Expires" << std::endl;
    EV << "Processing OpenConfirm::KeepaliveTimer_Expires" << std::endl;
    BGPSession& session = TopState::box().getModule();
    //If the local system receives a KeepaliveTimer_Expires event (Event 11), the local system:
    //- sends a KEEPALIVE message,
    session.sendKeepAliveMessage();
    //- restarts the KeepaliveTimer, and
    session.restartsKeepAliveTimer();
    //- remains in the OpenConfirmed state.
}
void OpenConfirm::TcpConnectionFails()
{
    std::cout << "OpenConfirm::TcpConnectionFails" << std::endl;
    setState<Idle>();
}
void OpenConfirm::OpenMsgEvent()
{
    EV << "Processing OpenConfirm::BGPOpen - collision" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._openMsgRcv ++;
    //If the local system receives a valid OPEN message (BGPOpen (Event 19)),
    //the collision detect function is processed per Section 6.8.
//TODO session._bgpRouting.collisionDetection();
    //If this connection is to be dropped due to connection collision, the local system:
    OpenConfirm::ConnectRetryTimer_Expires();
}
void OpenConfirm::KeepAliveMsgEvent()
{
    std::cout << "OpenConfirm::KeepAliveMsgEvent" << std::endl;
    EV << "Processing OpenConfirm::KeepAliveMsgEvent" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._keepAliveMsgRcv ++;
    //If the local system receives a KEEPALIVE message (KeepAliveMsg Event 26)), the local system:
    //- restarts the HoldTimer and
    session.restartsHoldTimer();
    //- changes its state to Established.
    setState<Established>();
}
void OpenConfirm::UpdateMsgEvent()
{
    std::cout << "OpenConfirm::UpdateMsgEvent" << std::endl;
    EV << "Processing OpenConfirm::UpdateMsgEvent" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._updateMsgRcv ++;
    OpenConfirm::ConnectRetryTimer_Expires();
}

//RFC 4271 - 8.2.2.  Finite State Machine - Established State
void Established::entry()
{
    std::cout << "Established::entry - send an update message" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._info.sessionEstablished = true;

    //if it's an EGP Session, send update messages with all routing information to BGP peer
    //if it's an IGP Session, send update message with only the BGP routes learned by EGP
    const IPv4Route*          rtEntry;
    BGP::RoutingTableEntry* BGPEntry;
    IRoutingTable*          IPRoutingTable = session.getIPRoutingTable();

    for (int i=1; i<IPRoutingTable->getNumRoutes(); i++)
    {
        rtEntry = IPRoutingTable->getRoute(i);
        if (rtEntry->getNetmask() == IPv4Address::ALLONES_ADDRESS ||
            rtEntry->getSource() == IPv4Route::IFACENETMASK ||
            rtEntry->getSource() == IPv4Route::MANUAL ||
            rtEntry->getSource() == IPv4Route::BGP)
        {
            continue;
        }

        if (session.getType() == BGP::EGP)
        {
            if (rtEntry->getSource() == IPv4Route::OSPF && session.checkExternalRoute(rtEntry))
            {
                continue;
            }
            BGPEntry = new BGP::RoutingTableEntry(rtEntry);
            std::string entryh = rtEntry->getDestination().str();
            std::string entryn = rtEntry->getNetmask().str();
            BGPEntry->addAS(session._info.ASValue);
            session.updateSendProcess(BGPEntry);
        }
    }

    std::vector<BGP::RoutingTableEntry*> BGPRoutingTable = session.getBGPRoutingTable();
    for (std::vector<BGP::RoutingTableEntry*>::iterator it = BGPRoutingTable.begin(); it != BGPRoutingTable.end(); it++)
    {
        session.updateSendProcess((*it));
    }

    //when all EGP Session is in established state, start IGP Session(s)
    BGP::SessionID nextSession = session.findAndStartNextSession(BGP::EGP);
    if (nextSession == (BGP::SessionID)-1)
    {
        session.findAndStartNextSession(BGP::IGP);
    }
}
void Established::ConnectRetryTimer_Expires()
{
    EV << "Processing Established::ConnectRetryTimer_Expires" << std::endl;
    BGPSession& session = TopState::box().getModule();
    //In response to any other event (Events 9, 12-13, 20-22), the local system:
//TODO- deletes all routes associated with this connection,
    //- sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- releases all BGP resources,
    //- drops the TCP connection,
    session._info.socket->abort();
    //- increments the ConnectRetryCounter by 1,
    ++ session._connectRetryCounter;
    //- changes its state to Idle.
    setState<Idle>();
}
void Established::HoldTimer_Expires()
{
    EV << "Processing Established::HoldTimer_Expires" << std::endl;
    BGPSession& session = TopState::box().getModule();
    //If the HoldTimer_Expires event occurs (Event 10), the local system:
    //- sets the ConnectRetryTimer to zero,
    session.restartsConnectRetryTimer(false);
    //- releases all BGP resources,
    //- drops the TCP connection,
    session._info.socket->abort();
    //- increments the ConnectRetryCounter by 1,
    ++ session._connectRetryCounter;
    //- changes its state to Idle.
    setState<Idle>();
}
void Established::KeepaliveTimer_Expires()
{
    EV << "Processing Established::KeepaliveTimer_Expires" << std::endl;
    BGPSession& session = TopState::box().getModule();
    //If the KeepaliveTimer_Expires event occurs (Event 11), the local system:
    //- sends a KEEPALIVE message, and
    session.sendKeepAliveMessage();
    //- restarts the KeepaliveTimer, unless the negotiated HoldTime value is zero.
    if (session._holdTime != 0)
    {
        session.restartsKeepAliveTimer();
    }
}
void Established::TcpConnectionFails()
{
    EV << "Processing Established::TcpConnectionFails" << std::endl;
}
void Established::OpenMsgEvent()
{
    EV << "Processing Established::BGPOpen" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._openMsgRcv ++;
}
void Established::KeepAliveMsgEvent()
{
    EV << "Processing Established::KeepAliveMsgEvent" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._keepAliveMsgRcv ++;
    //If the local system receives a KEEPALIVE message (Event 26), the local system:
    //- restarts its HoldTimer, if the negotiated HoldTime value is non-zero, and
    session.restartsHoldTimer();
    //- remains in the Established state.
}
void Established::UpdateMsgEvent()
{
    EV << "Processing Established::UpdateMsgEvent" << std::endl;
    BGPSession& session = TopState::box().getModule();
    session._updateMsgRcv ++;
    //If the local system receives an UPDATE message (Event 27), the local system:
    //- processes the message,
    //- restarts its HoldTimer, if the negotiated HoldTime value is non-zero, and
    session.restartsHoldTimer();
    //- remains in the Established state.
}

