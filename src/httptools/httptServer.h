
// ***************************************************************************
//
// HttpTools Project
//// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable
// Web server component.
//
// Maintainer: Kristjan V. Jonsson (LDSS) kristjanvj@gmail.com
// Project home page: code.google.com/p/omnet-httptools
//
// ***************************************************************************
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
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
// ***************************************************************************

#ifndef __httptServerSock_H_
#define __httptServerSock_H_

#include "TCPSocket.h"
#include "TCPSocketMap.h"
#include "httptServerBase.h"

/**
 * @brief httptServerDirect module
 *
 * This module implements a flexible Web server. It is part of the HttpTools project
 * and should be used in conjunction with a number of clients running the httptBrowserDirect.
 *
 * @see httptBrowserDirect
 *
 * @version 1.0
 * @author  Kristjan V. Jonsson
 */
class INET_API httptServer : public httptServerBase, public TCPSocket::CallbackInterface
{
	protected:
		TCPSocket listensocket;
		TCPSocketMap sockCollection;
		unsigned long numBroken;
		unsigned long socketsOpened;

	/** @name cSimpleModule redefinitions */
	//@{
	protected:
		/** @brief Initialization of the component and startup of browse event scheduling */
		virtual void initialize();

		/** @brief Report final statistics */
		virtual void finish();

		/** @brief Handle incoming messages */
		virtual void handleMessage(cMessage *msg);
	//@}

	/** @name TCPSocket::CallbackInterface methods */
	//@{
	protected:
		/**
		 * @brief handler for socket established events.
		 * Only used to update statistics.
		 */
		virtual void socketEstablished(int connId, void *yourPtr);

		/**
		 * @brief handler for socket data arrived events
		 * Dispatces the received message to the message handler in the base class and
 		 * finishes by deleting the received message.
		 */
		virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);

		/**
		 * @brief handler for socket closed by peer event
		 * Does little apart from calling socket->close() to allow the TCPSocket object to close properly.
		 */
		virtual void socketPeerClosed(int connId, void *yourPtr);

		/**
		 * @brief handler for socket closed event
		 * Cleanup the resources for the closed socket.
 		 */
		virtual void socketClosed(int connId, void *yourPtr);

		/**
		 * @brief handler for socket failure event
		 * Very basic handling -- displays warning and cleans up resources.
		 */
		virtual void socketFailure(int connId, void *yourPtr, int code);
	//@}
};

#endif


