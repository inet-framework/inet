//
// Copyright (C) 2010 Helene Lageber
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

package inet.common.misc;

//
// Emits double-valued signals in the specified interval.
// May be used for testing indicator figures.
//
simple SignalSource
{
    parameters:
        @display("i=block/cogwheel");
        @signal[signalSourceOutput];
        double startTime @unit(s) = default(0s);
        double endTime @unit(s) = default(-1s);
        volatile double interval @unit(s) = default(1s);
        string signalName = "signalSourceOutput";
        volatile double value;
}

