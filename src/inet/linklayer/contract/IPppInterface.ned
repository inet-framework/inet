//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Module interface for Point-to-Point Protocol (PPP) network interfaces that provide
// wired serial connectivity between network nodes. PPP interfaces handle the encapsulation
// of network datagrams into PPP frames and decapsulation of incoming PPP frames, while
// managing transmission queues and traffic conditioning. These interfaces are commonly
// used in routers and other devices requiring simple point-to-point connections.
//
// @see PppInterface, Ppp
//
moduleinterface IPppInterface extends IWiredInterface
{
    parameters:
        @display("i=block/ifcard");
}
