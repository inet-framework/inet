//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.test.moduletest.lib.ieee80211;

module Ieee80211BitDomainTest
{
    parameters:
        string convolutionalCoderModule = default("");
        string interleaverModule = default("");
        string scramblerModule = default("");
        int numberOfRandomErrors = default(0);
        string testType; // interleaver, scrambler, convCoder or all
        string testFile;
        @class(Ieee80211BitDomainTest);
}
