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

#ifndef __INET_INETFILTER_H
#define __INET_INETFILTER_H

#include <omnetpp.h>
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/contract/INetworkDatagram.h"

namespace inet {

/**
 * TODO
 */
class INET_API INetfilter
{
  public:
    /**
     * TODO
     */
    class INET_API IHook
    {
      public:
        enum Type {
            PREROUTING,
            LOCALIN,
            FORWARD,
            POSTROUTING,
            LOCALOUT
        };

        enum Result {
            ACCEPT,    /**< allow datagram to pass to next hook */
            DROP,    /**< do not allow datagram to pass to next hook, delete it */
            QUEUE,    /**< queue datagram for later re-injection */
            STOLEN    /**< do not allow datagram to pass to next hook, but do not delete it */
        };

        virtual ~IHook() {};

        /**
         * TODO
         * nextHopAddress ignored when outputInterfaceEntry is NULL
         */
        virtual Result datagramPreRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress) = 0;

        /**
         * TODO
         */
        virtual Result datagramForwardHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress) = 0;

        /**
         * TODO
         */
        virtual Result datagramPostRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress) = 0;

        /**
         * TODO
         */
        virtual Result datagramLocalInHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry) = 0;

        /**
         * TODO
         * nextHopAddress ignored when outputInterfaceEntry is NULL
         */
        virtual Result datagramLocalOutHook(INetworkDatagram *datagram, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress) = 0;
    };

    virtual ~INetfilter() {}

    /**
     * TODO
     */
    virtual void registerHook(int priority, IHook *hook) = 0;

    /**
     * TODO
     */
    virtual void unregisterHook(int priority, IHook *hook) = 0;

    /**
     * TODO
     */
    virtual void dropQueuedDatagram(const INetworkDatagram *daragram) = 0;

    /**
     * TODO
     */
    virtual void reinjectQueuedDatagram(const INetworkDatagram *datagram) = 0;
};

} // namespace inet

#endif // ifndef __INET_INETFILTER_H

