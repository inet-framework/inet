//
// Copyright (C) 2005 Andras Varga
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


package inet.world;

//
// Replaces channel objects in the network.
//
// This module is a temporary solution until the NED infrastructure
// gets extended to accomodate channel classes.
//
simple ChannelInstaller
{
    parameters:
        string channelClass; // e.g. "ThruputMeteringChannel"
        string channelAttrs; // params for the channel class, e.g. "format=N (U)"
        @display("i=block/cogwheel_s");
}


