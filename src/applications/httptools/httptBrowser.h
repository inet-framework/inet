
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


#ifndef __httptBrowserDirect_H_
#define __httptBrowserDirect_H_

#include "TCPSocket.h"
#include "TCPSocketMap.h"
#include "IPAddressResolver.h"
#include "httptBrowserBase.h"

/**
 * @brief Data structure used to keep state for each opened socket
 *
 * An instance of this struct is created for each opened socket and assigned to
 * it as a myPtr. See the TCPSocket::CallbackInterface methods of httptBrowser for more
 * details.
 *
 * @see httptBrowser
 */
struct SOCK_DATA_STRUCT
{
	MESSAGE_QUEUE_TYPE messageQueue; 	//> Queue of pending messages.
	TCPSocket *socket;					//> A reference to the socket object.
	int pending;						//> A counter for the number of outstanding replies.
};

/**
 * @short Browser module for OMNeT++ simulations
 *
 * A simulated browser module for OMNeT++.
 *
 * The component is designed to plug into the existing INET StandardHost module as a
 * tcpApp. See the INET documentation and examples for details.
 *
 * This component uses the TCP/IP modeling of the INET framework for transport.
 * Specifically, the TCPSocket class is used to interface with the TCP component from the INET framework.
 * A light-weight module which uses direct message passing is also available (httptBrowserDirect).
 *
 * @author Kristjan V. Jonsson (kristjanvj@gmail.com)
 * @version 1.0
 *
 * @see httptBrowserBase
 * @see httptBrowserDirect
 */
class INET_API httptBrowser : public httptBrowserBase, public TCPSocket::CallbackInterface
{
	protected:
		TCPSocketMap sockCollection; 	//> List of active sockets
		unsigned long numBroken;	 	//> Counter for the number of broken connections
		unsigned long socketsOpened;	//> Counter for opened sockets

		SOCK_DATA_STRUCT *pendingSocket;

	public:
		httptBrowser(); //* Constructor. Initializes counters */
		virtual ~httptBrowser();

	/** @name cSimpleModule redefinitions */
	//@{
	protected:
		/** @brief Initialization of the component and startup of browse event scheduling */
		virtual void initialize(int stage);

		/** @brief Report final statistics */
		virtual void finish();

		/** @brief Handle incoming messages */
		virtual void handleMessage(cMessage *msg);

		/** @brief Returns the number of initialization stages. Two required. */
//		int numInitStages() const {return 2;}
	//@}

	/** @name Implementations of pure virtual send methods */
	//@{
	protected:
		/** @brief Sends a scripted browse event to a specific server */
		virtual void sendRequestToServer( BROWSE_EVENT_ENTRY be );

		/** Send a request to server. Uses the recipient stamped in the request. */
		virtual void sendRequestToServer( httptRequestMessage *request );

		/** @brief Sends a generic request to a randomly chosen server */
		virtual void sendRequestToRandomServer();

		/** @brief Sends a number of queued messages to the specified server */
		virtual void sendRequestsToServer( string www, MESSAGE_QUEUE_TYPE queue );
	//@}

	/** @name TCPSocket::CallbackInterface callback methods */
	//@{
	protected:
		/** @brief Handler for socket established event.
		 *	Called by the socket->processMessage(msg) handler call in handleMessage.
         *  The pending messages for the socket are transmitted in the order queued. The socket remains
         *  open after the handler has completed. A counter for pending messages is incremented for each
         *  request sent.
		 */
		virtual void socketEstablished(int connId, void *yourPtr);

		/** @brief Handler for socket data arrival.
		 *	Called by the socket->processMessage(msg) handler call in handleMessage.
		 *  virtual method of the parent class. The counter for pending replies is decremented for each one handled.
         *  Close is called on the socket once the counter reaches zero.
		 */
		virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);

		/** @brief Handler for the socket closed by peer event
		 *	Called by the socket->processMessage(msg) handler call in handleMessage.
		 */
		virtual void socketPeerClosed(int connId, void *yourPtr);

		/** @brief socket closed handler
		 *	Called by the socket->processMessage(msg) handler call in handleMessage.
		 */
		virtual void socketClosed(int connId, void *yourPtr);

		/** @brief socket failure handler
		 *  This method does nothing but reporting and statistics collection at this time.
		 *  @todo Implement reconnect if necessary. See the INET demos.
		 */
		virtual void socketFailure(int connId, void *yourPtr, int code);

		/** @brief socket status arrived handler
		 *	Called by the socket->processMessage(msg) handler call in handleMessage.
		 */
		virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status);
	//@}

	/** @name Socket establishment and data submission */
	//@{
	protected:
		/** @brief Establishes a socket and queues a single message for transmisson.
		 *  A new socket is created and added to the collection. The message is assigned to a data structure
 		 *  stored as a myPtr with the socket. The message is transmitted once the socket is established, signaled
		 *  by a call to socketEstablished.
		 */
		void submitToSocket( const char* moduleName, int connectPort, cMessage *msg );

		/** @brief Establishes a socket and assigns a queue of messages to be transmitted.
		 *  Same as the overloaded version, except a number of messages are queued for transmission. The same socket
		 *  instance is used for all the queued messages.
		 */
		void submitToSocket( const char* moduleName, int connectPort, MESSAGE_QUEUE_TYPE &queue );
	//@}
};

#endif


