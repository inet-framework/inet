//
// Copyright (C) 2011 Juan Luis Garrote Molinero
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

package inet.linklayer.rstp.tester;

//
//TesterObject. Testing message generator
//
simple TesterObject
{
    parameters:
        @display("i=block/network2");	
        xml Messages = default(xmldoc("P1.xml"));  
        bool verbose = default(true);
        bool testing = default(false);
        bool detailedRSTP = default(false);
        bool detailedMVRP = default(false);
        bool detailedEthII = default(false);  //Print the last  EthernetII received message
        bool detailedEth1Q = default(false); //Print the last Ethernet IEEE 802.1Q received message
        bool detailedEth1ad = default(false); //Print the last Ethernet IEEE 802.1ad received message
        bool detailedEth1ah = default(false); //Print the last Ethernet IEEE 802.1ah received message
     gates:
      	input In;
      	output Out;
}
