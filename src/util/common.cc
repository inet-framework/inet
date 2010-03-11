//
// Copyright (C) 2004 Andras Varga
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

#include <sstream>

#include "common.h"

std::string intToString(int i)
{
  std::ostringstream stream;
  stream << i << std::flush;
  std::string str(stream.str());
  return str;
}

std::string vectorToString(IPAddressVector vec)
{
    return vectorToString(vec, ", ");
}

std::string vectorToString(IPAddressVector vec, const char *delim)
{
  std::ostringstream stream;
  for(unsigned int i = 0; i < vec.size(); i++)
  {
      stream << vec[i];
      if(i < vec.size() - 1)
        stream << delim;
  }
  stream << std::flush;
  std::string str(stream.str());
  return str;
}

std::string vectorToString(EroVector vec)
{
    return vectorToString(vec, ", ");
}

std::string vectorToString(EroVector vec, const char *delim)
{
    std::ostringstream stream;
    for(unsigned int i = 0; i < vec.size(); i++)
    {
        stream << vec[i].node;

        if(i < vec.size() - 1)
            stream << delim;
    }
    stream << std::flush;
    std::string str(stream.str());
    return str;
}

EroVector routeToEro(IPAddressVector rro)
{
    EroVector ero;

    for(unsigned int i = 0; i < rro.size(); i++)
    {
        EroObj_t hop;
        hop.L = false;
        hop.node = rro[i];
        ero.push_back(hop);
    }

    return ero;
}
uint32 getLevel(IPvXAddress addr)
{
    if (addr.isIPv6())
    {
        if (addr.get6().getScope()==IPv6Address::UNSPECIFIED || addr.get6().getScope()==IPv6Address::MULTICAST)
            return 0;
        if (addr.get6().getScope()==IPv6Address::LOOPBACK)
            return 1;
        if (addr.get6().getScope()==IPv6Address::LINK)
            return 2;
        if (addr.get6().getScope()==IPv6Address::SITE)
            return 3;
    }
    else
    {
        //Addresses usable with SCTP, but not as destination or source address
        if (addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("0.0.0.0"), IPAddress("255.0.0.0")) ||
            addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("224.0.0.0"), IPAddress("240.0.0.0")) ||
            addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("198.18.0.0"), IPAddress("255.255.255.0")) ||
            addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("192.88.99.0"), IPAddress("255.255.255.0")))
            return 0;

        //Loopback
        if (addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("127.0.0.0"), IPAddress("255.0.0.0")))
        return 1;

        //Link-local
        if (addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("169.254.0.0"), IPAddress("255.255.0.0")))
            return 2;

        //Private
        if (addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("10.0.0.0"), IPAddress("255.0.0.0")) ||
            addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("172.16.0.0"), IPAddress("255.240.0.0")) ||
            addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("192.168.0.0"), IPAddress("255.255.0.0")))
            return 3;
     }
    //Global
    return 4;
}
