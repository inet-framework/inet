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

import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.Ipv4Address;

cplusplus {{
#include "inet/networklayer/ipv4/Ipv4Route.h"
}}

namespace inet;

class Ipv4Route extends cNamedObject
{
    @existingClass;
    @descriptor(readonly);
    Ipv4Address destination @byValue;
    Ipv4Address netmask @byValue;
    Ipv4Address gateway @byValue;
    int metric;
    int prefixLength;
    string interfaceName;
}
