//
// Copyright (C) 2005 Andras Varga
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

import
    "WirelessAPWithSink";

module ThroughputClient
    gates:
        in: radioIn;

    submodules:
        notificationBoard: NotificationBoard;
            display: "p=52,70;i=block/control";
        cli: EtherAppCli;
            parameters:
                registerSAP = false;
            display: "b=40,24;p=180,60,col";
        wlan: Ieee80211NicSTASimplified;
            display: "p=112,134;q=queue;i=block/ifcard";
        mobility: NullMobility;
            display: "p=50,141;i=block/cogwheel_s";
    connections nocheck:
        wlan.radioIn <-- radioIn;
        cli.out --> wlan.uppergateIn;
endmodule

module Throughput
    parameters:
        numCli: numeric const,
        playgroundSizeX: numeric const,
        playgroundSizeY: numeric const;
    submodules:
        cliHost: ThroughputClient[numCli];
            display: "i=device/wifilaptop;r=,,#707070";
        ap: WirelessAPWithSink;
            display: "p=200,200;i=device/accesspoint;r=,,#707070";
        channelcontrol: ChannelControl;
            parameters:
                playgroundSizeX = playgroundSizeX,
                playgroundSizeY = playgroundSizeY;
            display: "p=61,46;i=misc/sun";
    connections nocheck:
    display: "b=297,203";
endmodule

network throughput : Throughput
endnetwork
