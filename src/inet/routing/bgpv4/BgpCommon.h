//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_BGPCOMMON_H
#define __INET_BGPCOMMON_H

#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/routing/bgpv4/BgpCommon_m.h"

namespace inet {

// Forward declarations:
class TcpSocket;

namespace bgp {

const unsigned char TCP_PORT = 179;

const unsigned char START_EVENT_KIND = 81;
const unsigned char CONNECT_RETRY_KIND = 82;
const unsigned char HOLD_TIME_KIND = 83;
const unsigned char KEEP_ALIVE_KIND = 89;
const unsigned char NB_TIMERS = 4;
const unsigned char NB_STATS = 6;
const unsigned char DEFAULT_COST = 1;
const unsigned char NB_SESSION_MAX = 255;

const unsigned char ROUTE_DESTINATION_CHANGED = 90;
const unsigned char NEW_ROUTE_ADDED = 91;
const unsigned char NEW_SESSION_ESTABLISHED = 92;
const unsigned char ASLOOP_NO_DETECTED = 93;
const unsigned char ASLOOP_DETECTED = 94;

static const int BGP_TCP_CONNECT_VALID = 71;
static const int BGP_TCP_CONNECT_CONFIRM = 72;
static const int BGP_TCP_CONNECT_FAILED = 73;
static const int BGP_TCP_CONNECT_OPEN_RCV = 74;
static const int BGP_TCP_KEEP_ALIVE_RCV = 75;

static const int BGP_RETRY_TIME = 120;
static const int BGP_HOLD_TIME = 180;
static const int BGP_KEEP_ALIVE = 60; // 1/3 of BGP_HOLD_TIME
static const int NB_SEC_START_EGP_SESSION = 1;

typedef Ipv4Address RouterId;
typedef Ipv4Address NextHop;
typedef unsigned short AsId;
typedef unsigned long SessionId;

struct SessionInfo
{
    SessionId sessionID = 0;
    BgpSessionType sessionType = INCOMPLETE;
    AsId ASValue = 0;
    Ipv4Address routerID;
    Ipv4Address peerAddr;
    Ipv4Address myAddr;
    bool nextHopSelf = false;
    int localPreference = 0;
    bool checkConnection = false;
    int ebgpMultihop = 0;
    NetworkInterface *linkIntf = nullptr;
    TcpSocket *socket = nullptr;
    TcpSocket *socketListen = nullptr;
    bool sessionEstablished = false;
};

} // namespace bgp

} // namespace inet

#endif

