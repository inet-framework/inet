//
// Protocol Test Framework for INET.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Putting the framework's NED types in their own package with an explicit @namespace keeps
// them insulated from a consumer's package namespace -- e.g. an opp_test `.test` case sets
// @namespace on the root package, which would otherwise rewrite the @class of any
// default-package NED type loaded alongside it.
//
package inet.protocoltest;

@namespace(inet::protocoltest);
