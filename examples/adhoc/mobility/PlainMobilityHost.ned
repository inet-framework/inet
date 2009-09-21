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




package inet.examples.adhoc.mobility;

import inet.base.NotificationBoard;
import inet.mobility.BasicMobility;


//
// A host for demonstrating mobility models only -- it contains
// no protocol layers at all.
//
module PlainMobilityHost
{
    parameters:
        @node();
        string mobilityType;
        @display("i=device/pocketpc_s");
    gates:
        input radioIn @directIn;
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=70,67");
        }
        mobility: <mobilityType> like BasicMobility {
            parameters:
                @display("p=132,141");
        }
   connections allowunconnected:
}

