// Copyright (C) 2012 - 2016 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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

/**
 * @file CLNSAddress.cc
 * @author Marcel Marek (mailto:imarek@fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 10.8.2016
 * @brief Class representing a CLNS Address
 * @detail Class representing a CLNS Address. It should be probably called NSAPAddress or something similar.
 */

#include <iomanip>

#include "inet/networklayer/contract/clns/ClnsAddress.h"

namespace inet{

const ClnsAddress ClnsAddress::UNSPECIFIED_ADDRESS;

ClnsAddress::ClnsAddress()
{
    areaID = 0;
    systemID = 0;
    nsel = 0;
}

ClnsAddress::ClnsAddress(uint64 areaID, uint64 systemID, uint8 nsel) :
    systemID(systemID),
    areaID(areaID),
    nsel(nsel)
{
}

void ClnsAddress::set(uint64 areaID, uint64 systemID, uint8 nsel)
{
    this->areaID = areaID;
    this->systemID = systemID;
    this->nsel = nsel;
}

ClnsAddress::ClnsAddress(std::string net)
{
    areaID = 0;
    systemID = 0;
    nsel = 0;

    unsigned int dots = 0;
    size_t found;

    //net address (in this module - not according to standard O:-) MUST have the following format:
    //49.0001.1921.6800.1001.00
    //IDI: 49 (private addressing)
    //AREA: 0001
    //systemID: 1921.6800.1001 from IP 192.168.1.1
    //NSEL: 00

    found = net.find_first_of(".");
    if (found != 2 || net.length() != 25) {
        return;
    }

    while (found != std::string::npos) {
        switch (found) {
        case 2:
            dots++;
            // area[0] = (unsigned char) (atoi(net.substr(0, 2).c_str()));
            areaID += (uint64_t)(strtoul(net.substr(0, 2).c_str(), NULL, 16)) << 16;
            break;
        case 7:
            areaID += (uint64_t)(strtoul(net.substr(3, 4).c_str(), NULL, 16));
            dots++;
            break;
        case 12:
            dots++;
            systemID += (uint64_t)(strtoul(net.substr(8, 4).c_str(), NULL, 16)) << 32;
            break;
        case 17:
            dots++;
            systemID += (uint64_t)(strtoul(net.substr(13, 4).c_str(), NULL, 16)) << 16;
            break;
        case 22:
            dots++;
            systemID += (uint64_t)(strtoul(net.substr(18, 4).c_str(), NULL, 16));
            break;
        default:
            return;
            break;
        }

        found = net.find_first_of(".", found + 1);
    }

    if (dots != 5) {
        return;
    }

    nsel = strtoul(net.substr(23, 2).c_str(), NULL, 16);

    //49.0001.1921.6801.2003.00
    //        this->nickname = this->sysId[ISIS_SYSTEM_ID - 1] + this->sysId[ISIS_SYSTEM_ID - 2] * 0xFF;
}

ClnsAddress::~ClnsAddress()
{
    // TODO Auto-generated destructor stub
}

bool ClnsAddress::isUnspecified() const
{
    return systemID == 0 && areaID == 0;
}

std::string ClnsAddress::str(bool printUnspec    /* = true */) const
{
    if (printUnspec && isUnspecified())
        return std::string("<unspec>");

    std::ostringstream buf;
    buf << std::hex << std::setfill('0')
        << std::setw(2) << ((areaID >> 16) & (0xFF)) << "."
        << std::setw(4) << (areaID & 0xFFFF) << "."
        << std::setw(4) << ((systemID >> 32) & 0xFFFF) << "."
        << std::setw(4) << ((systemID >> 16) & 0xFFFF) << "."
        << std::setw(4) << (systemID & 0xFFFF) << "."
        << std::setw(2) << (nsel & 0xFF);
    return buf.str();
}

uint64 ClnsAddress::getAreaId() const
{
    return areaID;
}

uint8 ClnsAddress::getNsel() const
{
    return nsel;
}

void ClnsAddress::setNsel(uint8 nsel)
{
    this->nsel = nsel;
}

uint64 ClnsAddress::getSystemId() const
{
    return systemID;
}

}  //end of namespace inet

