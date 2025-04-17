//
// Copyright (C) 2020 OpenSim Ltd and Marcel Marek
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.networklayer.ipv4.ipsec;

import inet.common.SimpleModule;

//
// Represents the IPsec Security Policy Database (SPD).
// The database is filled by the IPsec module.
//
simple SecurityPolicyDatabase extends SimpleModule
{
    parameters:
        @class(SecurityPolicyDatabase);
        @display("i=block/table2;is=s");
}
