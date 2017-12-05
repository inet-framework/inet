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

class b    { @primitive; @existingClass; @fromstring(b(string2long($)));      @tostring(long2string($.get()));   @defaultvalue(b(-1)); }
class B    { @primitive; @existingClass; @fromstring(B(string2long($)));      @tostring(long2string($.get()));   @defaultvalue(B(-1)); }
class bps  { @primitive; @existingClass; @fromstring(bps(string2double($)));  @tostring(double2string($.get())); @defaultvalue(bps(NaN)); }
class Hz   { @primitive; @existingClass; @fromstring(Hz(string2double($)));   @tostring(double2string($.get())); @defaultvalue(Hz(NaN)); }
class m    { @primitive; @existingClass; @fromstring(m(string2double($)));    @tostring(double2string($.get())); @defaultvalue(m(NaN)); }
class Ohmm { @primitive; @existingClass; @fromstring(Ohmm(string2double($))); @tostring(double2string($.get())); @defaultvalue(Ohmm(NaN)); }
class W    { @primitive; @existingClass; @fromstring(W(string2double($)));    @tostring(double2string($.get())); @defaultvalue(W(NaN)); }

