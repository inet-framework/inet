//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;

cplusplus{{
#include "inet/common/Units.h"
}}

namespace inet;

cplusplus{{
using namespace units::values;
}}

//namespace inet::units::values;

class b    { @existingClass; @primitive; @fromString(b(string2long($)));      @toString(unit2string($));   @defaultValue(b(-1)); }
class B    { @existingClass; @primitive; @fromString(B(string2long($)));      @toString(unit2string($));   @defaultValue(B(-1)); }
class bps  { @existingClass; @primitive; @fromString(bps(string2double($)));  @toString(unit2string($)); @defaultValue(bps(NaN)); }
class Hz   { @existingClass; @primitive; @fromString(Hz(string2double($)));   @toString(unit2string($)); @defaultValue(Hz(NaN)); }
class m    { @existingClass; @primitive; @fromString(m(string2double($)));    @toString(unit2string($)); @defaultValue(m(NaN)); }
class Ohmm { @existingClass; @primitive; @fromString(Ohmm(string2double($))); @toString(unit2string($)); @defaultValue(Ohmm(NaN)); }
class rad  { @existingClass; @primitive; @fromString(rad(string2double($)));  @toString(unit2string($)); @defaultValue(rad(NaN)); }
class W    { @existingClass; @primitive; @fromString(W(string2double($)));    @toString(unit2string($)); @defaultValue(W(NaN)); }

