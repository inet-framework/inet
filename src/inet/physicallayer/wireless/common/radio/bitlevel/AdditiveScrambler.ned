//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.radio.bitlevel;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.bitlevel.IScrambler;

// The Scrambler module converts an input string into a seemingly random
// output string of the same length.
//
// The seed parameter represents the initial contents of the shift
// registers which is a binary sequence, for example, seed = "1011101".
//
// The generator polynomial describes how the registers connect to the
// modulo 2 adder in the scrambler's hardware implementation.
// The links between the shift registers and a modulo 2 adder can be
// described with a polynomial in the following way: the x^7 + x^4 + 1
// polynomial selects two registers: the 7th register and the 4th register,
// and gives their contents as an input to the modulo 2 adder. The string
// representation of this polynomial is "0001001". The length of this
// string equals the degree of the `generatorPolynomial` and each 1 bit at the ith
// position indicates that the x^i term has a 1 coefficient. Similarly,
// a 0 bit indicates a zero coefficient.
//
//
module AdditiveScrambler extends Module like IScrambler
{
    parameters:
        string seed;
        string generatorPolynomial;
        @class(AdditiveScramblerModule);
        @display("i=block/tunnel");
}

