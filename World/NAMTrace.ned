//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//
// NAMTrace manages a "nam" trace file which can be played back with ns2's
// Network ANimator, nam. The trace is actually written by NAMTraceWriter
// modules which should occur per host or router.
//
// @see NAMTraceWriter
//
simple NAMTrace
    parameters:
        logfile : string,
        prolog : string;
endsimple


