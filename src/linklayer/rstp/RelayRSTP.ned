//
// Copyright (C) 2011 Juan Luis Garrote Molinero
// Copyright (C) 2013 Zsolt Prontvai
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

package inet.linklayer.rstp;

// Relay unit. Uses RSTP modul and MACAddressTable information to determine the correct output gate for the incoming frames.

simple RelayRSTP
{
    parameters:
        @display("i=block/broadcast;bgb=425,114");
        bool verbose = default(false);
        
    gates:
        output GatesOut[];  // 
        input GatesIn[];	// 
        inout RSTPGate;	// To the RSTP module
}
