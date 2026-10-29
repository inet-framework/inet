//
// Protocol Test Framework for INET -- an application whose TCP sockets a test program drives.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocoltest;

import inet.applications.contract.IApp;

//
// An application with TCP sockets that a protocol test program drives. It makes no call by
// itself: an action of the program calls its sockets, in the context of this module, for
// example act(action().at(0.1).run<TcpTestApp>("host1.app[0]", ...)). It emits each
// indication of its sockets as a signal, so that the steps of the program can observe the
// application level. The data that it receives and sends go out as packetReceivedFromLower
// and packetSentToLower, which the tester observes as packets; the other indications carry
// the id of the socket, and socketFailure the code of the failure.
//
simple TcpTestApp like IApp
{
    parameters:
        @class(TcpTestApp);
        @display("i=block/app");
        @signal[packetReceivedFromLower](type=inet::Packet);
        @signal[packetSentToLower](type=inet::Packet);
        @signal[socketEstablished](type=long);
        @signal[socketPeerClosed](type=long);
        @signal[socketClosed](type=long);
        @signal[socketFailure](type=long);
        @signal[socketSendReady](type=long);
    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}
