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

#include "inet/networklayer/contract/clns/CLNSAddress.h"

namespace inet{

static const int CLNSADDRESS_STRING_SIZE = 25;
const CLNSAddress CLNSAddress::UNSPECIFIED_ADDRESS;



CLNSAddress::CLNSAddress()
{


    areaID = 0;
    systemID = 0;
    nsel = 0;

}

CLNSAddress::CLNSAddress(uint64 areaID, uint64 systemID)
{

      this->areaID = areaID;
      this->systemID = systemID;
      nsel = 0;

}

void CLNSAddress::set(uint64 areaID, uint64 systemID)
{

      this->areaID = areaID;
      this->systemID = systemID;
      nsel = 0;

}

CLNSAddress::CLNSAddress(std::string net)
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
//                    cout << "BEZ ATOI" << net.substr(0, 2).c_str() << endl;
        break;
      case 7:

        areaID += strtoul(net.substr(3, 2).c_str(), NULL, 16);
        areaID += strtoul(net.substr(5, 2).c_str(), NULL, 16) << 8;
        dots++;
        break;
      case 12:
        dots++;
        systemID += strtoul(net.substr(8, 2).c_str(), NULL, 16);
        systemID += strtoul(net.substr(10, 2).c_str(), NULL, 16) << 8;
        break;
      case 17:
        dots++;
        systemID += strtoul(net.substr(13, 2).c_str(), NULL, 16) << 16;
        systemID += strtoul(net.substr(15, 2).c_str(), NULL, 16) << 24;
        break;
      case 22:
        dots++;
        systemID += strtoul(net.substr(18, 2).c_str(), NULL, 16) << 32;
        systemID +=strtoul(net.substr(20, 2).c_str(), NULL, 16) << 36;
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

  nsel =  strtoul(net.substr(23, 2).c_str(), NULL, 16);



  //49.0001.1921.6801.2003.00

//        this->nickname = this->sysId[ISIS_SYSTEM_ID - 1] + this->sysId[ISIS_SYSTEM_ID - 2] * 0xFF;



}

CLNSAddress::~CLNSAddress() {
    // TODO Auto-generated destructor stub
}

bool CLNSAddress::isUnspecified() const {
    return systemID == 0 && areaID == 0;
}

std::string CLNSAddress::str(bool printUnspec    /* = true */) const
{
    if (printUnspec && isUnspecified())
        return std::string("<unspec>");

    char buf[CLNSADDRESS_STRING_SIZE];
    sprintf(buf, "%02lX.%04lX.%04lX.%04lX.%04lX.%02X", (areaID >> 16) & (0xFF), areaID & (0xFFFF), (systemID >> 32) & (0xFFFF), (systemID >> 16) & (0xFFFF), systemID & (0xFFFF), nsel & 255);
    return std::string(buf);
}



uint64 CLNSAddress::getAreaId() const
{
  return areaID;
}

uint8 CLNSAddress::getNsel() const
{
  return nsel;
}

void CLNSAddress::setNsel(uint8 nsel)
{
  this->nsel = nsel;
}

uint64 CLNSAddress::getSystemId() const
{
  return systemID;
}

}  //end of namespace inet
