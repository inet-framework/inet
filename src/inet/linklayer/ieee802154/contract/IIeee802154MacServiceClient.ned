// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee802154.contract;

// Gate-free client role. A concrete client declares an empty default only to
// make an unconfigured instance explicit, then must bind a valid typed
// serviceModule peer before it can run.
moduleinterface IIeee802154MacServiceClient
{
    parameters:
        string serviceModule;
}
