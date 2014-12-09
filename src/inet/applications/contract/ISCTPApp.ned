//
// Copyright (C) 2008 Irene Ruengeler
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.applications.contract;

//
// Not an actual module type. It only shows which gates an SCTP app should have
// to be able to be used in ~StandardHost etc.
//
moduleinterface ISCTPApp
{
    parameters:
        @display("i=block/app");
    gates:
        input sctpIn @labels(SCTPCommand/up);
        output sctpOut @labels(SCTPCommand/down);
}
