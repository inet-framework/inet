//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.tunapp;

import inet.common.SimpleModule;
import inet.applications.contract.IApp;

simple TunLoopbackApp extends SimpleModule like IApp
{
    parameters:
        @class(TunLoopbackApp);
        string interfaceTableModule;
        string tunInterface;
        @display("i=block/app");
    gates:
        input socketIn @labels(TunInterface/up);
        output socketOut @labels(TunInterface/down);
}

