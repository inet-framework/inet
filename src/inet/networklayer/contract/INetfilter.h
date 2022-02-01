//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INETFILTER_H
#define __INET_INETFILTER_H

#include "inet/common/packet/Packet.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/NetworkHeaderBase_m.h"

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
            ACCEPT, ///< allows the datagram to pass to the next hook
            DROP,   ///< doesn't allow the datagram to pass to the next hook, will be deleted
            QUEUE,  ///< queues the datagram for later re-injection (e.g. when route discovery completes)
            STOLEN  ///< doesn't allow datagram to pass to next hook, but won't be deleted
        };

        virtual ~IHook() {}

        /**
         * This is the first hook called by the network protocol before it routes
         * a datagram that was received from the lower layer. The nextHopAddress
         * is ignored when the outputNetworkInterface is nullptr.
         */
        virtual Result datagramPreRoutingHook(Packet *datagram) = 0;

        /**
         * This is the second hook called by the network protocol before it sends
         * a datagram to the lower layer. This is done after the datagramPreRoutingHook
         * or the datagramLocalInHook is called and the datagram is routed.
         */
        virtual Result datagramForwardHook(Packet *datagram) = 0;

        /**
         * This is the last hook called by the network protocol before it sends
         * a datagram to the lower layer.
         */
        virtual Result datagramPostRoutingHook(Packet *datagram) = 0;

        /**
         * This is the last hook called by the network protocol before it sends
         * a datagram to the upper layer. This is done after the datagramPreRoutingHook
         * is called and the datagram is routed.
         */
        virtual Result datagramLocalInHook(Packet *datagram) = 0;

        /**
         * This is the first hook called by the network protocol before it routes
         * a datagram that was received from the upper layer. The nextHopAddress
         * is ignored when the outputNetworkInterface is a nullptr. After this is done
         */
        virtual Result datagramLocalOutHook(Packet *datagram) = 0;
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
    virtual void unregisterHook(IHook *hook) = 0;

    /**
     * Requests the network layer to drop the datagram, because it's no longer
     * needed. This function may be used by a reactive routing protocol when it
     * cancels the route discovery process.
     */
    virtual void dropQueuedDatagram(const Packet *daragram) = 0;

    /**
     * Requests the network layer to restart the processing of the datagram. This
     * function may be used by a reactive routing protocol when it completes the
     * route discovery process.
     */
    virtual void reinjectQueuedDatagram(const Packet *datagram) = 0;
};

class INET_API NetfilterBase : public INetfilter
{
  public:
    class INET_API HookBase : public INetfilter::IHook {
        friend class NetfilterBase;

      protected:
        std::vector<INetfilter *> netfilters;

      public:
        virtual ~HookBase() { for (auto netfilter : netfilters) netfilter->unregisterHook(this); }

        void registeredTo(INetfilter *nf) { ASSERT(!isRegisteredHook(nf)); netfilters.push_back(nf); }
        void unregisteredFrom(INetfilter *nf) { ASSERT(isRegisteredHook(nf)); remove(netfilters, nf); }
        bool isRegisteredHook(INetfilter *nf) { return contains(netfilters, nf); }
    };

  protected:
    std::multimap<int, IHook *> hooks;

  public:
    virtual ~NetfilterBase() {
        for (auto hook : hooks)
            check_and_cast<HookBase *>(hook.second)->unregisteredFrom(this);
    }

    virtual void registerHook(int priority, INetfilter::IHook *hook) override {
        hooks.insert(std::pair<int, INetfilter::IHook *>(priority, hook));
        check_and_cast<HookBase *>(hook)->registeredTo(this);
    }

    virtual void unregisterHook(INetfilter::IHook *hook) override {
        for (auto iter = hooks.begin(); iter != hooks.end(); iter++) {
            if (iter->second == hook) {
                hooks.erase(iter);
                check_and_cast<HookBase *>(hook)->unregisteredFrom(this);
                return;
            }
        }
    }
};

} // namespace inet

#endif

