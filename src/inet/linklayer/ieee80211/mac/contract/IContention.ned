//
// Copyright (C) 2016 OpenSim Ltd.
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

package inet.linklayer.ieee80211.mac.contract;

//
// Interface for modules that implement contention-based channel access.
// For each frame, Contention listens on the channel for a DIFS (AIFS) period
// then for a random backoff period before transitting the frame, and defers when
// busy channel is sensed. After receiving a corrupted frame, EIFS is used instead
// of the original DIFS (AIFS).
//
moduleinterface IContention
{
    parameters:
        @display("i=block/timer");
}

