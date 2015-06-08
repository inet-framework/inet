//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
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
// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

package inet.routing.pim.tables;

//
// The PIMNeighborTable module contains information about neighboring PIM routers.
//
simple PIMNeighborTable
{
      @display("i=block/table");
      @signal[NF_PIM_NEIGHBOR_ADDED](type=PIMNeighbor);
    @signal[NF_PIM_NEIGHBOR_DELETED](type=PIMNeighbor);
    @signal[NF_PIM_NEIGHBOR_CHANGED](type=PIMNeighbor);
}
