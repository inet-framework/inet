//
// Copyright (C) 2006 Christoph Sommer
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

package inet.routing.extras.dymo_fau;

//
// This Queue goes between transport and network layer. It intercepts data packets for which we do not have a route for yet. Operations are supervised and controlled by the user-space module called "dymo".
//
simple DYMO_DataQueue
{
    parameters:
        double BUFFER_SIZE_PACKETS = default(50); // maximum queue size in packets, -1 for no limit
        double BUFFER_SIZE_BYTES @unit("B") = default(75000B); // maximum queue size in bytes, -1 for no limit
    gates:
        input transportIn[]; // from transport layer
        output networkOut[]; // to network layer
}
