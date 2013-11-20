// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// @file pimSM.ned
// @date 29.10.2011
// @author: Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief File implements compbound module pimSplitter
// @details pimSplitter is module which split PIM-DM and PIM-SM packets to simple modules.

package inet.networklayer.pim;


simple PIMSplitter
{
    parameters:
    	@display("i=block/dispatch");
    	string configFile;
        string hostname;
        @signal[NF_IPv4_NEW_MULTICAST_DENSE](type=cObject); // addRemoveAddr or PIMMulticastRoute
        @signal[NF_IPv4_NEW_MULTICAST_SPARSE](type=PIMMulticastRoute);
        @signal[NF_IPv4_NEW_IGMP_ADDED](type=addRemoveAddr);
        @signal[NF_IPv4_NEW_IGMP_ADDED_PISM](type=addRemoveAddr);
        @signal[NF_IPv4_NEW_IGMP_REMOVED](type=addRemoveAddr);
        @signal[NF_IPv4_NEW_IGMP_REMOVED_PIMSM](type=addRemoveAddr);

    gates:
        input pimDMIn;
        input pimSMIn;
        output pimDMOut;
        output pimSMOut;
        input transportIn;
        output transportOut;
}
