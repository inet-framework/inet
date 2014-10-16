//
// Copyright (C) 2011 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.examples.inet.pcaprecorder;


import inet.common.misc.NetAnimTrace;
import inet.common.packet.PcapRecorder;
import inet.common.queue.Sink;
import inet.common.serializer.PcapParser;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.ipv6.StandardHost6;
import inet.node.wireless.AccessPoint;
import ned.DatarateChannel;


network PcapRecorderTest
{
    @display("bgb=306,193");
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
        }
    submodules:
        client: StandardHost {
            parameters:
                @display("p=164,136;i=device/pc3");
        }
        server: StandardHost {
            parameters:
                @display("p=260,87;i=device/pc2");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=46,28");
        }
        netAnimTrace: NetAnimTrace {
            @display("p=46,105");
        }
    connections:
        client.ethg++ <--> C <--> server.ethg++;
}


network PcapRecorderTestIPv6
{
    @display("bgb=306,193");
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
        }
    submodules:
        client: StandardHost6 {
            parameters:
                @display("p=164,136;i=device/pc3");
        }
        server: StandardHost6 {
            parameters:
                @display("p=260,87;i=device/pc2");
        }
        configurator: FlatNetworkConfigurator6 {
            parameters:
                @display("p=46,28");
        }
    connections:
        client.ethg++ <--> C <--> server.ethg++;
}

network PcapRecorderTest80211
{
    submodules:
        client: WirelessHost {
            parameters:
                @display("p=164,136");
                wlan[*].mgmtType = "Ieee80211MgmtSTASimplified";
        }
        server: WirelessHost {
            parameters:
                @display("p=260,87");
                wlan[*].mgmtType = "Ieee80211MgmtSTASimplified";
        }
        ap: AccessPoint {
            parameters:
                @display("p=239,178");
                wlan[*].mgmtType = "Ieee80211MgmtAPSimplified";
        }
        radioChannel: SimplifiedRadioChannel {
            parameters:
                numChannels = 2;
                @display("p=61,46");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                @display("p=61,108");
        }
}

network PcapParserTest
{
    submodules:
        PCAP: PcapParser {
            @display("p=58,56");
        }
        Sink: Sink {
            @display("p=212,56");
        }
    connections:
        PCAP.packageOut --> Sink.in++;
}

network PcapToPcap extends PcapParserTest
{
    submodules:
        Recorder: PcapRecorder {
            @display("p=134,119");
        }
}
