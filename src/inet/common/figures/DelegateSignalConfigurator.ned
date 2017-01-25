//
// Copyright (C) 2016 Opensim Ltd.
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


package inet.common.figures;

//
// Implements @delegatesignal properties. @delegatesignal arranges a
// source signal to be emitted as a target signal, possibly on another
// module. 
//
// A @delegatesignal property needs two attributes:
// - source is the signal to subscribe to, in <signalName> or 
//   <modulePath>.<signalName> format
// - target is the signal to emit, in <signalName> or 
//   <modulePath>.<signalName> format
//
// Example:
// <code>
// @delegatesignal[rcvdPk](source=hostB.udpApp[0].rcvdPk; target=hostBrcvdPk);
// </code>
//
// This module runs at initialization time. It finds @delegatesignal properties
// in all modules, and adds the appropriate signal listeners.
//
// This is a helper for instrument figures, as @statistic can only listen on
// local signals. It can be probably removed in OMNeT++ 5.1. 
//
simple DelegateSignalConfigurator
{
    parameters:
        @display("i=block/cogwheel_s");
}
