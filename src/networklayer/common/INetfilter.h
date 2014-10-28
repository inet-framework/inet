//
// Copyright (C) 2012 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_INETFILTER_H_
#define __INET_INETFILTER_H_

#include <omnetpp.h>
#include "InterfaceEntry.h"
#include "IPv4Datagram.h"

/**
 * TODO
 */
class INET_API INetfilter {
  public:
    /**
     * TODO
     */
    class INET_API IHook {
        public:
            enum Type {
                PREROUTING,
                LOCALIN,
                FORWARD,
                POSTROUTING,
                LOCALOUT
            };

            enum Result {
                ACCEPT, /**< allow datagram to pass to next hook */
                DROP,   /**< do not allow datagram to pass to next hook, delete it */
                QUEUE,  /**< queue datagram for later re-injection */
                STOLEN  /**< do not allow datagram to pass to next hook, but do not delete it */
            };

            virtual ~IHook() {};

            /**
             * TODO
             * nextHopAddress ignored when outputInterfaceEntry is NULL
             */
            virtual Result datagramPreRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHopAddress) = 0;

            /**
             * TODO
             */
            virtual Result datagramForwardHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHopAddress) = 0;

            /**
             * TODO
             */
            virtual Result datagramPostRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHopAddress) = 0;

            /**
             * TODO
             */
            virtual Result datagramLocalInHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry) = 0;

            /**
             * TODO
             * nextHopAddress ignored when outputInterfaceEntry is NULL
             */
            virtual Result datagramLocalOutHook(IPv4Datagram * datagram, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHopAddress) = 0;
    };

    virtual ~INetfilter() { }

    /**
     * TODO
     */
    virtual void registerHook(int priority, IHook * hook) = 0;

    /**
     * TODO
     */
    virtual void unregisterHook(int priority, IHook * hook) = 0;

    /**
     * TODO
     */
    virtual void dropQueuedDatagram(const IPv4Datagram * daragram) = 0;

    /**
     * TODO
     */
    virtual void reinjectQueuedDatagram(const IPv4Datagram * datagram) = 0;
};

#endif
