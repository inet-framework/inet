
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

#include "httptBrowserBase.h"

/**
 * @short Browser module for OMNeT++ simulations
 *
 * A simulated browser module for OMNeT++ simulations.
 *
 * This module implements direct message passing between modules.
 *
 * @see httptBrowserBase
 * @see httptBrowser
 *
 * @author Kristjan V. Jonsson (kristjanvj@gmail.com)
 * @version 1.0
 */
class INET_API httptBrowserDirect : public httptBrowserBase
{
	/** @name cSimpleModule redefinitions */
	//@{
	protected:
		/** Initialization of the component and startup of browse event scheduling */
		virtual void initialize(int stage);

		/** Report final statistics */
		virtual void finish();

		/** Handle incoming messages. See the parent class for details. */
		virtual void handleMessage(cMessage *msg);

		/** @brief Returns the number of initialization stages. Two required. */
//		int numInitStages() const {return 2;}
	//@}

	/** @name Implementation of methods for sending requests to a server. See parent class for details. */
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
};

#endif


