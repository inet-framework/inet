//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IETHERNETCSMAMAC_H
#define __INET_IETHERNETCSMAMAC_H

#include "inet/physicallayer/wired/ethernet/EthernetSignal_m.h"

namespace inet {

using namespace inet::physicallayer;

class INET_API IEthernetCsmaMac
{
  public:
    // IEEE 802.3-2018 22.2.2.11 CRS (carrier sense)
    /**
     * Called when the PHY starts transmitting or receiving a signal and the carrier can be detected on the channel.
     */
    virtual void handleCarrierSenseStart() = 0; // MII CRS signal
    /**
     * Called when the PHY ends transmitting or receiving any signal and the carrier can no longer be detected on the
     * channel.
     */
    virtual void handleCarrierSenseEnd() = 0; // MII CRS signal

    // IEEE 802.3-2018 22.2.2.12 COL (collision detected)
    /**
     * Called when the PHY starts receiving a signal while transmitting, or when the PHY starts transmitting a signal
     * while receiving.
     */
    virtual void handleCollisionStart() = 0; // MII COL signal

    /**
     * Called when the PHY ends both transmitting and receiving any signal.
     */
    virtual void handleCollisionEnd() = 0; // MII COL signal

    /**
     * Called when a reception successfully starts. The packet argument is nullptr for non-data signals.
     */
    virtual void handleReceptionStart(EthernetSignalType signalType, Packet *packet) = 0; // MII RX_DV and RXD signals (rx_cmd)

    /**
     * Called when a reception ends with an error. The packet argument is nullptr for non-data signals.
     */
    virtual void handleReceptionError(EthernetSignalType signalType, Packet *packet) = 0;

    /**
     * Called when a reception successfully ends. The packet argument is nullptr for non-data signals.
     */
    virtual void handleReceptionEnd(EthernetSignalType signalType, Packet *packet, EthernetEsdType esd1) = 0; // MII RX_DV and RXD signals (rx_cmd)
};

} // namespace inet

#endif

