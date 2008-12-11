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



package inet.util;

//
// When placed inside a host or router, it writes a NAM trace, based on
// notifications it receives via the NotificationBoard. It expects to find
// a NAMTrace module named "nam" at network level. If the "nam" module
// is missing, this module does not do anything and has no runtime
// overhead.
//
// This module has no gates and does not process messages.
//
// @see NAMTrace, NotificationBoard
//
simple NAMTraceWriter
{
    parameters:
        int namid = default(-1);  // -1 means auto
        @display("i=block/sink");
}


