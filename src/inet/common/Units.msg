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

class b    { @existingClass; @byvalue; @opaque; @fromstring(b(string2long($)));      @tostring(long2string($.get()));   @defaultvalue(b(-1)); }
class B    { @existingClass; @byvalue; @opaque; @fromstring(B(string2long($)));      @tostring(long2string($.get()));   @defaultvalue(B(-1)); }
class bps  { @existingClass; @byvalue; @opaque; @fromstring(bps(string2double($)));  @tostring(double2string($.get())); @defaultvalue(bps(NaN)); }
class Hz   { @existingClass; @byvalue; @opaque; @fromstring(Hz(string2double($)));   @tostring(double2string($.get())); @defaultvalue(Hz(NaN)); }
class m    { @existingClass; @byvalue; @opaque; @fromstring(m(string2double($)));    @tostring(double2string($.get())); @defaultvalue(m(NaN)); }
class Ohmm { @existingClass; @byvalue; @opaque; @fromstring(Ohmm(string2double($))); @tostring(double2string($.get())); @defaultvalue(Ohmm(NaN)); }
class W    { @existingClass; @byvalue; @opaque; @fromstring(W(string2double($)));    @tostring(double2string($.get())); @defaultvalue(W(NaN)); }

