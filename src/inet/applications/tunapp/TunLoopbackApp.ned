//
// Copyright (C) 2015 Irene Ruengeler
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

package inet.applications.tunapp;

import inet.applications.contract.IApp;

simple TunLoopbackApp like IApp
{
    parameters:
        string interfaceTableModule;
        string tunInterface;
        @display("i=block/app");
    gates:
        input socketIn @labels(TunInterface/up);
        output socketOut @labels(TunInterface/down);
}

