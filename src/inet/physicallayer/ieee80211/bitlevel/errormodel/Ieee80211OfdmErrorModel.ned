//
// Copyright (C) 2014 OpenSim Ltd.
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

package inet.physicallayer.ieee80211.bitlevel.errormodel;

import inet.physicallayer.contract.bitlevel.ILayeredErrorModel;
import inet.physicallayer.ieee80211.packetlevel.errormodel.Ieee80211NistErrorModel;

// This is the error model for the layered IEEE 802.11
// OFDM PHY infrastructure (IEEE 802.11-2012, Clause 18).
// OFDM means that the physical layer uses OFDM modulation.
//
// The Ieee80211OfdmErrorModel, depending of the level of detail
// (see: ~Ieee80211LayeredOfdmTransmitter and ~Ieee80211LayeredOfdmReceiver),
// produces erroneous bits, symbols, samples or just simply
// determines whether a packet is erroneous or not.
//
// The error model has built-in methods to approximate the
// corresponding error rates as a function of signal-to-noise
// ratio, bandwidth and other characteristics of the radio
// channel and the OFDM subcarrier modulations.
//
// If you don't want to use these approximations you can set
// your own error rates for the signal (IEEE 802.11-2012,
// 18.3.4) and data field (18.3.5).
//
//
module Ieee80211OfdmErrorModel extends Ieee80211NistErrorModel like ILayeredErrorModel
{
    parameters:
        double dataSymbolErrorRate = default(nan);
        double dataBitErrorRate = default(nan);
        double signalSymbolErrorRate = default(nan);
        double signalBitErrorRate = default(nan);
        @class(Ieee80211OfdmErrorModel);
        @display("i=block/broadcast");
}

