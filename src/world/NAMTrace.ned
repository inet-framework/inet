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
// NAMTrace manages a "nam" trace file which can be played back with ns2's
// Network ANimator, nam. The trace is actually written by NAMTraceWriter
// modules which should occur per host or router.
//
// @see NAMTraceWriter
//
simple NAMTrace
{
    parameters:
        string logfile = default("");  // the name of the logfile ("trace.nam")
        string prolog = default("");
        @display("i=old/floppy1");
        @labels(node);
}


