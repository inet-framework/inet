//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

package inet.linklayer;

//
// This is the interface of traffic conditioner modules.
//
// Traffic conditioners implement the policing and shaping actions
// of a Diffserv router. They should be added to the input
// or output path of interface modules of routers at the
// edge of the Diffserv Domain.
//
// Traffic conditioners perform the following actions:
//  - classify the incoming packets
//  - meter the traffic in each class
//  - marks/drops packets depending on the result of metering
//  - shape the traffic to conform to the desired traffic profile
//
// They can be implemented as a compound module, using the classifier,
// meter, and marker modules defined in this package.
//
// See RFC 3290 for a detailed desciption of the architecture of traffic
// conditioners.
//
moduleinterface ITrafficConditioner
{
    parameters:
        @display("i=block/classifier");
    gates:
        input in;
        output out;
}

