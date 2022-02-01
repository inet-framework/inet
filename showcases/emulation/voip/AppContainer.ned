//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.emulation.voip;

import inet.applications.contract.IApp;
import inet.emulation.transportlayer.udp.ExtLowerUdp;

network AppContainer
{
    submodules:
        app: <> like IApp;
        udp: ExtLowerUdp;
    connections:
        app.socketOut --> udp.appIn;
        app.socketIn <-- udp.appOut;
}
