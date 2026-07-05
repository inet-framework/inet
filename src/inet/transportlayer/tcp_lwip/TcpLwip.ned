//
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.tcp_lwip;

import inet.common.SimpleModule;
import inet.transportlayer.contract.ITcp;

//
// TCP model based on the LWIP 1.3.2 project.
// The original LWIP is available on
// <a href="http://savannah.nongnu.org/projects/lwip/" target="_blank">this page</a>.
//
// See ~ITcp for general TCP layer information.
//
// **Region tags are not preserved.** Unlike the ~Tcp module, TcpLwip wraps the
// real lwIP TCP/IP C stack, which operates on raw byte buffers. Outgoing
// application data is serialized and copied into a plain byte buffer for lwIP,
// and incoming data is handed back as raw bytes from which a fresh packet is
// built. No inet::Packet survives this round trip, so all region tags on the
// application data are lost -- for example CreationTimeTag (which breaks
// end-to-end delay measurement) and GenericAppMsgReq (which carries the
// request/reply control of the generic TCP apps). Consequently, applications
// that convey information in region tags do not work over TcpLwip: in
// particular ~TcpGenericServerApp raises an error when it receives request data
// without a GenericAppMsgReq tag. Use the ~Tcp module for such applications, or
// an application that transfers only opaque byte-stream data. The same
// limitation applies to emulation (external interfaces), which likewise deals
// in raw bytes.
//
simple TcpLwip extends SimpleModule like ITcp
{
    parameters:
        @class(TcpLwip);
        string sendQueueClass = default("");    //# TODO Obsolete!!!
        string receiveQueueClass = default(""); //# TODO Obsolete!!!
        string checksumMode @enum("declared", "computed") = default("declared");
        @display("i=block/wheelbarrow");

    gates:
        input appIn @messageKinds(inet::TcpCommandCode);
        input ipIn;
        output appOut @messageKinds(inet::TcpStatusInd);
        output ipOut;
}

