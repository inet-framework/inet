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
// Allows signal values to be displayed in various instruments, meters 
// and gauges.
//
// Instruments should be added with the @figure property, like any other
// figure. The figure's C++ implementation must inherit from IMeterFigure,
// which is basically a wrapper around a setValue() method.
// 
// Signals and figures can be associated using @displaysignal properties.
// A @displaysignal property needs three attributes:
// - source is the signal to subscribe to, in <signalName> or 
//   <modulePath>.<signalName> format
// - filter is a standard result filter (count, min, max, sum, last, mean, etc)
// - figure is the name (path) of the figure, it must be in the same module
//   as the @displaysignal property
//
// Example:
// <code>
// @displaysignal[pkCount](source=hostB.udpA[0].rcvdPk;filter=count;figure=pkCountGauge);
// </code>
//
// This module runs at initialization time, finds @displaysignal properties
// in all modules, and adds the appropriate listeners that update the figures
// when a signal is emitted.
//
simple DelegateSignalConfigurator
{
    parameters:
        @display("i=block/cogwheel_s");
}
