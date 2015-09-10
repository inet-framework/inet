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

#ifndef __INET_ISYMBOL_H
#define __INET_ISYMBOL_H

#include "inet/common/INETDefs.h"

namespace inet {

namespace physicallayer {

class INET_API ISymbol
{
  public:
    virtual ~ISymbol() {}
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_ISYMBOL_H

