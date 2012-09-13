//
// Copyright (C) 2010 Andras Varga
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

package inet.util;

//
// Records a NetAnim trace, using network-level modules and their connections.
// See http://www.nsnam.org/wiki/index.php/NetAnim
//
// Note: NetAnim does not have a built-in layouter, and requires explicit
// node coordinates to be written into the file. This module takes node
// coordinates from module display strings; if there are no coordinates
// there, random coordinates will be generated. Thus, for good results
// it is recommended that you assign explicit coordinates to all network
// nodes.
//
// @author Andras
//
simple NetAnimTrace
{
    parameters:
        bool enabled = default(true);
        string filename = default("netanim-trace.txt");
        @display("i=block/control_s");
        @labels(node);
}
