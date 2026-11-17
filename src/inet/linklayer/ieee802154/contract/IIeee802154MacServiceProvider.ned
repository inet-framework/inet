// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee802154.contract;

// Gate-free provider role. A concrete provider declares an empty default only
// to make an unconfigured instance explicit, then must bind a valid typed
// clientModule peer before it can run.
moduleinterface IIeee802154MacServiceProvider
{
    parameters:
        string clientModule;
}
