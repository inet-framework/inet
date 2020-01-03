//
// Copyright (C) 2013 Opensim Ltd.
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

package inet.test.moduletest.lib;

//
// An ISriptable module for dumping routing tables into a file.
//
// Use the following command in the script of the ScenarioManager: 
//
//   <dump-routes module="..." nodes="..." dest="..."/>
//
// where
//   - @module is the full name of this logger module
//   - @nodes is the full name of the nodes whose routing tables to be dumped
//   - @dest is a list of address/prefix_length pairs for filtering the routes (optional)
//
simple RoutingTableLogger
{
    parameters:
        string outputFile = default("");
        @display("i=block/cogwheel_s");
}

