//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Module interface for external network interfaces that connect the simulation to the
// network stack of the host operating system. External interfaces act as bridges between
// the simulated and real domains, allowing packets to flow between the simulation and
// real networks. This enables hardware-in-the-loop simulations, testing of simulated
// protocols with real-world counterparts, and other hybrid simulation scenarios.
//
// @see ExtInterface, ExtLowerEthernetInterface, ExtUpperEthernetInterface
//
moduleinterface IExternalInterface extends INetworkInterface
{
    parameters:
        @display("i=block/ifcard");
}
