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

cplusplus {{
#include "inet/networklayer/base/NetworkLayerNodeBase.h"
#include "inet/networklayer/ipv4/IPv4Route.h"
}}

namespace inet;

class IPv4Route extends cNamedObject;
class LinkLayerNodeBase extends cModule;

class NetworkLayerNodeBase extends LinkLayerNodeBase
{
    @existingClass;
    IPv4Route* ipv4Routes[] @getter(getIPv4Route) @sizeGetter(getNumIPv4Routes);
}

