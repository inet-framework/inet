//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/base/EthernetModes.h"

#include "inet/linklayer/ethernet/common/Ethernet.h"

namespace inet {

const EthernetModes::EthernetMode EthernetModes::nullEthernetMode = {
    0.0,
    0,
    B(0),
    B(0),
    B(0),
    0.0,
    0.0
};

const EthernetModes::EthernetMode EthernetModes::ethernetModes[NUM_OF_ETHERNETMODES] = {
    {
        ETHERNET_TXRATE,
        0,
        B(0),
        MIN_ETHERNET_FRAME_BYTES,
        MIN_ETHERNET_FRAME_BYTES,
        512 / ETHERNET_TXRATE,
        2500 /*m*/ / SPEED_OF_LIGHT_IN_CABLE
    },
    {
        FAST_ETHERNET_TXRATE,
        0,
        B(0),
        MIN_ETHERNET_FRAME_BYTES,
        MIN_ETHERNET_FRAME_BYTES,
        512 / FAST_ETHERNET_TXRATE,
        250 /*m*/ / SPEED_OF_LIGHT_IN_CABLE
    },
    {
        GIGABIT_ETHERNET_TXRATE,
        MAX_PACKETBURST,
        GIGABIT_MAX_BURST_BYTES,
        GIGABIT_MIN_FRAME_BYTES_WITH_EXT,
        MIN_ETHERNET_FRAME_BYTES,
        4096 / GIGABIT_ETHERNET_TXRATE,
        250 /*m*/ / SPEED_OF_LIGHT_IN_CABLE
    },
    {
        TWOANDHALFGIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        FIVEGIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        FAST_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        TWENTYFIVE_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        FOURTY_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        HUNDRED_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        TWOHUNDRED_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        FOURHUNDRED_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    }
};

const EthernetModes::EthernetMode& EthernetModes::getEthernetMode(double txRate)
{
    for (auto& ethernetMode : ethernetModes) {
        if (txRate == ethernetMode.bitrate)
            return ethernetMode;
    }
    throw cRuntimeError("Invalid ethernet transmission rate %g bps", txRate);
}

} // namespace inet
