//
// Copyright (C) 2017 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.linklayer.lmac;

import inet.linklayer.common.WirelessInterface;

//
// This module implements a wireless network interface using the L-MAC protocol.
//
module LMacInterface extends WirelessInterface
{
    parameters:
        radio.typename = default("ApskScalarRadio");
        mac.typename = "LMac";
}

