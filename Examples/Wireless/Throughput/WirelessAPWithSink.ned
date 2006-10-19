//
// Copyright (C) 2006 Andras Varga
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
    "Sink",
    "Ieee80211NicAPSimplified",
    "NullMobility",
    "NotificationBoard";

//
// Well, it Models a 802.11 Access Point with a Sink.
//
module WirelessAPWithSink
    gates:
        in: radioIn;
    submodules:
        sink: Sink;
            display: "i=block/sink;p=169,124";
        wlan: Ieee80211NicAPSimplified;
            display: "p=89,225;q=queue;i=block/ifcard,#ff8000";
        notificationBoard: NotificationBoard;
            display: "p=78,60;i=block/control";
        mobility: NullMobility;
            display: "p=71,121;i=block/cogwheel_s";
    connections nocheck:
        radioIn --> wlan.radioIn display "m=s";
        wlan.uppergateOut --> sink.in++;
endmodule

