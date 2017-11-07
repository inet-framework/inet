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
#include "inet/physicallayer/common/packetlevel/Signal.h"
}}

namespace inet::physicallayer;

cplusplus {{
typedef const ITransmission ConstITransmission;
typedef const IArrival ConstIArrival;
typedef const IListening ConstIListening;
typedef const IReception ConstIReception;
}}

class ConstITransmission extends void;
class ConstIArrival extends void;
class ConstIListening extends void;
class ConstIReception extends void;

packet Signal
{
    @existingClass(true);
    @descriptor(readonly);
    ConstITransmission *transmission;
    ConstIArrival *arrival;
    ConstIListening *listening;
    ConstIReception *reception;
}

