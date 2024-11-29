//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IETHERNETCSMAPHY_H
#define __INET_IETHERNETCSMAPHY_H

#include "inet/physicallayer/wired/ethernet/EthernetSignal_m.h"

namespace inet {

namespace physicallayer {

class INET_API IEthernetCsmaPhy
{
  public:
    virtual b getEsdLength() = 0;

    virtual void startSignalTransmission(EthernetSignalType signalType) = 0; // MII TX_EN and TXD signals (tx_cmd BEACON, COMMIT, JAM)
    virtual void endSignalTransmission(EthernetEsdType esd1) = 0; // MII TX_EN and TXD signals (tx_cmd NONE)

    virtual void startFrameTransmission(Packet *packet, EthernetEsdType esd1) = 0; // MII TX_EN and TXD signals
    virtual void endFrameTransmission() = 0; // MII TX_EN and TXD signals
};

} // namespace physicallayer

} // namespace inet

#endif

