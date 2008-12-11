//
// Copyright (C) 2005 Andras Varga
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

#ifndef OPP_UTILS_H
#define OPP_UTILS_H

#include <string>
#include <omnetpp.h>
#include "INETDefs.h"



namespace OPP_Global
{
  /**
   *  Converts an integer to string.
   */
  std::string ltostr(long i);          //XXX make an ultostr as well, to be consistent with atoul

  /**
   *  Converts a double to string
   */
  std::string dtostr(double d);

  /**
   *  Converts string to double
   */
  double atod(const char *s);

  /**
   *  Converts string to unsigned long
   */
  unsigned long atoul(const char *s);
}

#endif

