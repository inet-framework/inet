//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.queueing.base;

//
// This is a base module for various token generator modules.
//
simple TokenGeneratorBase
{
    parameters:
        string serverModule; // module path of the packet server where the tokens are generated via C++ method call
        string displayStringTextFormat = default("generated %t tk\nto %s"); // determines display string text above the submodule
        @class(TokenGeneratorBase);
        @display("i=block/control");
}
