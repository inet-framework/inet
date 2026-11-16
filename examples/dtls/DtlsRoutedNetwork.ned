// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.examples.dtls;

import inet.networks.base.WiredNetworkBase;
import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import inet.node.ethernet.EthernetLink;

network DtlsRoutedNetwork extends WiredNetworkBase
{
    parameters:
        visualizer.typename = default("");
    submodules:
        client: StandardHost;
        router: Router;
        server: StandardHost;
    connections:
        client.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> router.ethg++;
        router.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> server.ethg++;
}
