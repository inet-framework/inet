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
 * This interface is implemented by network protocols which want provide netfilter
 * hooks to customize their behavior. For example, implementing a reactive routing
 * protocol can be done using this interface.
 */
class INET_API INetfilter
{
  public:
    /**
     * This interface is used by the network protocol during processing datagrams.
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
            ACCEPT, ///< allows the datagram to pass to the next hook
            DROP,   ///< doesn't allow the datagram to pass to the next hook, will be deleted
            QUEUE,  ///< queues the datagram for later re-injection (e.g. when route discovery completes)
            STOLEN  ///< doesn't allow datagram to pass to next hook, but won't be deleted
        };

        virtual ~IHook() {};

        /**
         * This is the first hook called by the network protocol before it routes
         * a datagram that was received from the lower layer. The nextHopAddress
         * is ignored when the outputInterfaceEntry is nullptr.
         */
        virtual Result datagramPreRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress) = 0;

        /**
         * This is the second hook called by the network protocol before it sends
         * a datagram to the lower layer. This is done after the datagramPreRoutingHook
         * or the datagramLocalInHook is called and the datagram is routed.
         */
        virtual Result datagramForwardHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress) = 0;

        /**
         * This is the last hook called by the network protocol before it sends
         * a datagram to the lower layer.
         */
        virtual Result datagramPostRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress) = 0;

        /**
         * This is the last hook called by the network protocol before it sends
         * a datagram to the upper layer. This is done after the datagramPreRoutingHook
         * is called and the datagram is routed.
         */
        virtual Result datagramLocalInHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry) = 0;

        /**
         * This is the first hook called by the network protocol before it routes
         * a datagram that was received from the upper layer. The nextHopAddress
         * is ignored when the outputInterfaceEntry is a nullptr. After this is done
         */
        virtual Result datagramLocalOutHook(INetworkDatagram *datagram, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress) = 0;
    };

    virtual ~INetfilter() {}

    /**
     * Adds the provided hook to the list of registered hooks that will be called
     * by the network layer when it processes datagrams.
     */
    virtual void registerHook(int priority, IHook *hook) = 0;

    /**
     * Removes the provided hook from the list of registered hooks.
     */
    virtual void unregisterHook(int priority, IHook *hook) = 0;

    /**
     * Requests the network layer to drop the datagram, because it's no longer
     * needed. This function may be used by a reactive routing protocol when it
     * cancels the route discovery process.
     */
    virtual void dropQueuedDatagram(const INetworkDatagram *daragram) = 0;

    /**
     * Requests the network layer to restart the processing of the datagram. This
     * function may be used by a reactive routing protocol when it completes the
     * route discovery process.
     */
    virtual void reinjectQueuedDatagram(const INetworkDatagram *datagram) = 0;
};

} // namespace inet

#endif // ifndef __INET_INETFILTER_H

