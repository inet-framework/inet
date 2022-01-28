//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2010 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.transportlayer.contract;

//
// Interface for SCTP protocol.
//
moduleinterface ISctp
{
    @display("i=block/wheelbarrow");
    gates:
        input appIn @labels(SctpCommand/down);
        input ipIn;
        output appOut @labels(SctpCommand/up);
        output ipOut;
}

