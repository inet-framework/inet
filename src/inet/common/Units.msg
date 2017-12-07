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

class b    { @primitive; @existingClass; @fromString(b(string2long($)));      @toString(long2string($.get()));   @defaultValue(b(-1)); }
class B    { @primitive; @existingClass; @fromString(B(string2long($)));      @toString(long2string($.get()));   @defaultValue(B(-1)); }
class bps  { @primitive; @existingClass; @fromString(bps(string2double($)));  @toString(double2string($.get())); @defaultValue(bps(NaN)); }
class Hz   { @primitive; @existingClass; @fromString(Hz(string2double($)));   @toString(double2string($.get())); @defaultValue(Hz(NaN)); }
class m    { @primitive; @existingClass; @fromString(m(string2double($)));    @toString(double2string($.get())); @defaultValue(m(NaN)); }
class Ohmm { @primitive; @existingClass; @fromString(Ohmm(string2double($))); @toString(double2string($.get())); @defaultValue(Ohmm(NaN)); }
class W    { @primitive; @existingClass; @fromString(W(string2double($)));    @toString(double2string($.get())); @defaultValue(W(NaN)); }

