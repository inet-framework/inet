//
// Copyright 2004 Andras Varga
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


#include "TCPGenericCliAppBase.h"
#include "IPAddressResolver.h"
#include "GenericAppMsg_m.h"



/** Redefined to schedule a connect(). */
void TelnetApp::initialize()
{
}

void TelnetApp::handleTimer(cMessage *msg)
{
}

void TelnetApp::socketEstablished(int, void *)
{
}

void TelnetApp::socketDataArrived(int, void *, cMessage *msg, bool urgent)
{
}

/** Redefined to start another session after a delay. */
void TelnetApp::socketClosed(int, void *)
{
}

/** Redefined to reconnect after a delay. */
void TelnetApp::socketFailure(int, void *, int)
{
}

