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
