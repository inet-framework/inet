//
// Copyright (C) 2006 Levente Meszaros
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
package inet.linklayer.ethernet;

import inet.linklayer.IWiredNic;


//
// Ethernet network interface which supports full-duplex operation only.
// Complements ~EtherMACFullDuplex and ~EtherEncap with an output queue for QoS and
// RED support.
//
// @see EthernetInterface, EthernetInterfaceNoQueue
//
module EthernetInterfaceFullDuplex extends EthernetInterface like IWiredNic
{
    parameters:
        macType = "EtherMACFullDuplex";
}
