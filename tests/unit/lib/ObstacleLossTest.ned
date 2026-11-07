//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.test.unittest.lib;

//
// Computes the obstacle loss of a radio medium once for each frequency, along
// the straight path from the transmission position to the reception position,
// and prints the loss in dB.
//
simple ObstacleLossTest
{
    parameters:
        string radioMediumModule = default("radioMedium");
        string frequencies; // space-separated list of frequencies
        double txX @unit(m);
        double txY @unit(m);
        double txZ @unit(m);
        double rxX @unit(m);
        double rxY @unit(m);
        double rxZ @unit(m);
        @class(::inet::ObstacleLossTest);
}
