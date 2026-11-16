//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.contract;

// The datagram transport used by a DTLS engine to reach its peers.
moduleinterface IDtls
{
    gates:
        input socketIn;
        output socketOut;
}
