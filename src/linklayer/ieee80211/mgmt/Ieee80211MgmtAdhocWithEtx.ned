//
// Copyright (C) 2010 Alfonso Ariza Quintana
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

package inet.linklayer.ieee80211.mgmt;
//
// 802.11 management module used for ad-hoc mode.
// Relies on the MAC layer (Ieee80211Mac) for reception and transmission of frames.
//
// This implementation never sends control or management frames, and discards
// any such frame received. Distributed beacon generation is not modelled.
// Only data frames are understood or sent at any time.
//
// This module never switches channels, that is, it will operate on the channel
// the physical layer is configured for (see channelNumber in Ieee80211Radio).
//
// @author Andras Varga
//
simple Ieee80211MgmtAdhocWithEtx extends Ieee80211MgmtAdhoc
{
    parameters:
        @class(Ieee80211MgmtAdhocWithEtx);
        @display("i=block/cogwheel");
        bool ETXEstimate = default(false);
    gates:
        input ETXProcIn;
        output ETXProcOut;
}
