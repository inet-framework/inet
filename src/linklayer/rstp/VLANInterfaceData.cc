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

#include <sstream>

#include "VLANInterfaceData.h"

VLANInterfaceData::VLANInterfaceData()
{
    tagged = true;
    cost = 1;
    priority = 5;
}

std::string VLANInterfaceData::info() const
{
    //FIXME
    std::stringstream out;
    out <<"VLAN info\n";
    return out.str();
}

std::string VLANInterfaceData::detailedInfo() const
{
    //FIXME
    std::stringstream out;
    out <<"VLAN detailedInfo\n";
    return out.str();
}
