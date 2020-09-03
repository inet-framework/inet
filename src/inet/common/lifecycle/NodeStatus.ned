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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.common.lifecycle;

//
// Keeps track of the status of network node (up, down, etc.) for other
// modules, and also displays it as a small overlay icon on this module
// and on the module of the network node.
//
simple NodeStatus
{
    parameters:
        //TODO @signal, @statistic
        string initialStatus = default("UP"); // initial status of the network node
        @display("i=block/cogwheel;is=s");
        @signal[nodeStatusChanged](type=inet::NodeStatus);
}

