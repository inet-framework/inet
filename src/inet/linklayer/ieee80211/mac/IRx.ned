//
// Copyright (C) 2015 Andras Varga
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

package inet.linklayer.ieee80211.mac;

//
// Interface for Rx processes. The Rx process checks received frames for
// errors, manages the NAV, and notifies Tx processes about the channel state
// (free or busy). The channel is free only if it is free according to both
// the physical (CCA) and the virtual (NAV-based) carrier sense algorithms.
// Correctly received frames are sent up to UpperMac (see IUpperMac), corrupted
// frames are discarded. Tx processes are also notified about corrupted and
// correctly received frames. so they can switch between using DIFS/AIFS and EIFS
// according to the channel access procedure.
//
moduleinterface IRx
{
}
