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
//

package inet.linklayer.ieee80211.mac.channelaccess;

import inet.linklayer.ieee80211.mac.contention.Contention;

//
// IEEE 802.11 Enhanced Distributed Channel Access Function
//
module Edcaf
{
    parameters:
        string accessCategory @enum("AC_BK","AC_BE","AC_VI","AC_VO");
        int aifsn = default(-1);
        int cwMin = default(-1);
        int cwMax = default(-1);

        string rateSelectionModule;
        string rxModule;
        string collisionControllerModule;

        @class(Edcaf);
        @display("i=block/queue");

    submodules:
        contention: Contention {
            parameters:
                @display("p=100,100");
        }
}
