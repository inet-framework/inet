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
// Interface for UpperMacs. UpperMacs deal with exchanging frames, and build
// on the services of IContention, ITx and IRx for accessing the
// channel. Responsibilities of UpperMacs include queueing, ACK generation
// and retransmissions, RTS/CTS, duplicate detection, fragmentation,
// aggregation, block acknowledgement, rate selection, and more.
//
moduleinterface IUpperMac
{
}
