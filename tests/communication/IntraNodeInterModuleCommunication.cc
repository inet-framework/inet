//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "IntraNodeInterModuleCommunication.h"

namespace inet {
namespace communication {

Define_Module(Dispatcher);
Define_Module(Ethernet);
Define_Module(EthernetMac);
Define_Module(Ipv4);
Define_Module(Ipv6);
Define_Module(NetworkInterface);
Define_Module(PacketQueue);
Define_Module(PacketSink);
Define_Module(PcapRecorder);
Define_Module(Ppp);
Define_Module(Tcp);
Define_Module(TcpApp);
Define_Module(TcpProcessingDelay);
Define_Module(Udp);

} // namespace communication
} // namespace inet

