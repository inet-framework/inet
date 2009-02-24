// -*- mode:c++ -*-
//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#ifndef __INET_BLACKBOARD_H
#define __INET_BLACKBOARD_H

#include <map>
#include <vector>
#include <omnetpp.h>
#include "INETDefs.h"


class BlackboardAccess;


/**
 * The Blackboard works as entity to enable inter-layer/inter-process communication.
 *
 * NOTE: BLACKBOARD IS NO LONGER USED BY THE INET FRAMEWORK -- SUCCESSOR IS
 * NotificationBoard.
 *
 * Blackboard makes it possible for several modules (representing e.g. protocol
 * layers) to share information, in a publish-subscribe fashion.
 * Participating modules or classes have to implement the BlackboardAccess
 * interface (=have this abstract class as base class).
 *
 * Anyone can publish data items on the blackboard. In order to allow for some
 * type safety via dynamic_cast, items have to be subclassed from cPolymorphic.
 *
 * Every item is published with a unique string label.
 * The Blackboard makes no assumption about the format
 * of the label, but it's generally a good idea to make it structured,
 * e.g. with a dotted format ("nic1.linkstatus"). The label can be used by
 * subscribers to identify an item ("I want to subscribe to nic1.linkstatus").
 * There are other ways to subscribe too, not only by label, e.g. one can
 * browse through all Blackboard items, examine each (its label, C++ class,
 * actual contents etc.) and decide individually whether to subscribe to it
 * or not.
 *
 * Labels are only used when publishing or subscribing to items. After that,
 * items can be referred to using BBItemRefs. (BBItemRefs are sort of "handles"
 * to blackboard items; think of file handles (fd's) or FILE* variables in Unix
 * and C programming, or window handles (HWND) in the Windows API.)
 * BBItemRefs allow very efficient (constant-time) access to Blackboard items.

 * Clients may browse blackboard contents and subscribe to items. After subscription,
 * clients will receive notifications via BlackboardAccess callbacks whenever
 * the subscribed item changes. Clients may also subscribe to get notified whenever
 * items are published to or withdrawn from the blackboard.
 *
 * Notifications are done with callback-like mechanism. Participating modules
 * or classes have to subclass from the abstract base class BlackboardAccess
 * and implement its methods (in Java, one would say clients should implement
 * the BlackboardAccess interface.) This usually means multiple inheritance,
 * but without the problems of multiple inheritance (Multiple inheritance
 * is sometimes considered "evil", but never if used to do "mixins" as here.)
 *
 * Some examples for usage. The first one, Foo demonstrates subscribe-by-name,
 * the second one, Bar shows subscribe-when-published, and the third one,
 * Zak does subscribe-by-browse.
 *
 * <pre>
 * // subscribe-by-name
 * class Foo : public cSimpleModule, public BlackboardAccess
 * {
 *   protected:
 *     BBItemRef ref;
 *     ...
 * };
 *
 * void Foo::initialize(int stage)
 * {
 *     // to avoid a subscribe-BEFORE-publish a two stage
 *     // initialization should be used and all publish calls should
 *     // go into the first stage (stage 0) whereas you should subscribe
 *     // in the second stage (stage 1)
 *     if(stage==0)
 *     {
 *         ...
 *     }
 *     else if(stage==1)
 *     {
 *         ref = getBlackboard()->subscribe(this,"routingTable");
 *         ...
 *     }
 * }
 *
 * void Foo::blackboardItemChanged(BBItemRef item)
 * {
 *     if (item==ref)
 *     {
 *         IRoutingTable *rt = check_and_cast<IRoutingTable *>(ref->getData());
 *         ...
 *     }
 *     else ...
 * }
 *
 * // subscribe-when-published
 * class Bar : public cSimpleModule, public BlackboardAccess
 * {
 *    ...
 * };
 *
 * void Bar::initialize(int stage)
 * {
 *     if(stage==0)
 *     {
 *         getBlackboard()->registerClient(this);
 *         // make sure we get what's already on the blackboard
 *         getBlackboard()->getBlackboardContent(this);
 *     }
 * }
 *
 * void Bar::blackboardItemPublished(BBItemRef item)
 * {
 *     // if label begins with "nic." and it's a NetworkInterfaceData, subscribe
 *     if (!strncmp(item->getLabel(),"nic.",4) &&
 *         dynamic_cast<NetworkInterfaceData *>(item->getData()))
 *     {
 *         // new network interface appeared, subscribe to it and do whatever
 *         // other actions are necessary
 *         getBlackboard()->subscribe(this, item);
 *         ...
 *     }
 * }
 *
 * // subscribe-by-browse
 * class Zak : public cSimpleModule, public BlackboardAccess
 * {
 *    ...
 * };
 *
 * void Zak::letsCheckWhatIsOnTheBlackboard()
 * {
 *     // subscribe to all NetworkInterfaceData
 *     Blackboard *bb = getBlackboard();
 *     for (Blackboard::iterator i=bb->begin(); i!=bb->end(); ++i)
 *         if (dynamic_cast<NetworkInterfaceData *>((*i)->getData()))
 *             bb->subscribe(this, *i);
 * }
 * </pre>
 *
 * @author Andras Varga
 * @ingroup blackboard
 */
class INET_API Blackboard : public cSimpleModule
{
  public:
    typedef std::vector<BlackboardAccess *> SubscriberVector;

    /**
     * Represents a blackboard item.
     */
    class BBItem
    {
      private:
        friend class Blackboard;
        cPolymorphic *_item;
        std::string _label;
        SubscriberVector subscribers;
      public:
        /** Return the label of this data item*/
        const char *getLabel()  {return _label.c_str();}
        /** Return the data item*/
        cPolymorphic *getData()  {return _item;}
        /** Return the data item*/
        const cPolymorphic *getData() const  {return _item;}
    };

  protected:
    /** @brief Set debugging for the basic module*/
    bool coreDebug;

    class Iterator;
    friend class Iterator;

    // blackboard items, with their subscriber lists
    typedef std::map<std::string, BBItem*> ContentsMap;
    ContentsMap contents;

    // those who have subscribed to publish()/withdraw() requests
    SubscriberVector registeredClients;

  public:
    /**
     * "Handle" to blackboard items. To get the data or the label, write
     * bbref->getData() and bbref->getLabel(), respectively. BBItemRefs become
     * stale when the item gets withdrawn (unpublished) from the blackboard.
     */
    typedef BBItem *BBItemRef;

    /**
     * Iterates through blackboard contents. Models a C++ standard
     * bidirectional iterator.
     */
    class iterator
    {
      private:
        ContentsMap::iterator it;
      public:
        iterator(ContentsMap::iterator it0) {it==it0;}
        BBItemRef operator*()  {return (*it).second;}
        iterator& operator++()  {++it; return *this;}
        iterator operator++(int)  {iterator x=iterator(it); ++it; return x;}
        iterator& operator--()  {--it; return *this;}
        iterator operator--(int)  {iterator x=iterator(it); --it; return x;}
        bool operator==(const iterator& i2)  {return it==i2.it;}
        bool operator!=(const iterator& i2)  {return it!=i2.it;}
    };

  public:
    Blackboard();
    virtual ~Blackboard();

  protected:
    /**
     * Initialize BB.
     */
    virtual void initialize();

    /**
     * Does nothing.
     */
    virtual void handleMessage(cMessage *msg);

  public:
    /** @name Methods for publishers */
    //@{
    /**
     * Publish new item on the BB, with the given label.
     */
    virtual BBItemRef publish(const char *label, cPolymorphic *item);

    /**
     * Withdraw (unpublish) item from the BB (typically called by publisher).
     */
    virtual void withdraw(BBItemRef bbItem);

    /**
     * Tell BB that an item has changed (typically called by publisher).
     * When item pointer is omitted, it is assumed that the item object
     * was updated "in place" (as opposed to being replaced by another object).
     */
    virtual void changed(BBItemRef bbItem, cPolymorphic *item=NULL);
    //@}

    /** @name Methods for subscribers */
    //@{
    /**
     * Subscribe to a BB item identified by a label
     */
    virtual BBItemRef subscribe(BlackboardAccess *bbClient, const char *label);

    /**
     * Find item with given label on the BB
     */
    virtual BBItemRef find(const char *label);

    /**
     * Subscribe to a BB item identified by item reference
     */
    virtual BBItemRef subscribe(BlackboardAccess *bbClient, BBItemRef bbItem);

    /**
     * Unsubcribe module from change notifications
     */
    virtual void unsubscribe(BlackboardAccess *bbClient, BBItemRef bbItem);

    /**
     * Generally subscribe to notifications about items being published
     * to/withdrawn from BB.
     */
    virtual void registerClient(BlackboardAccess *bbClient);

    /**
     * Cancel subscription initiated by registerClient().
     */
    virtual void removeClient(BlackboardAccess *bbClient);

    /**
     * Utility function: the client gets immediate notification with
     * all items currently on clipboard (as if they were added just now).
     * This may simplify initialization code in a subscribe-when-published
     * style client.
     */
    virtual void getBlackboardContent(BlackboardAccess *bbClient);

    /**
     * As with standard C++ classes.
     */
    iterator begin()  {return iterator(contents.begin());}

    /**
     * As with standard C++ classes.
     */
    iterator end()  {return iterator(contents.end());}
    //@}
};

typedef Blackboard::BBItemRef BBItemRef;


/**
 * Gives subscribe access to the Blackboard.
 *
 * @author Andras Varga
 * @ingroup blackboard
 */
class INET_API BlackboardAccess
{
  protected:
    Blackboard *bb;

  public:
    BlackboardAccess() {bb=NULL;}
    virtual ~BlackboardAccess() {}

    /** Returns a pointer to the Blackboard*/
    virtual Blackboard *getBlackboard();

    /** @name Callbacks invoked by the blackboard */
    //@{
    /** Called whenever an already published item changes*/
    virtual bool blackboardItemChanged(BBItemRef item) = 0;
    /** Called whenever a new item is published on the Blackboard*/
    virtual bool blackboardItemPublished(BBItemRef item) = 0;
    /** Called whenever an item is removed from the Blackboard*/
    virtual bool blackboardItemWithdrawn(BBItemRef item) = 0;
    //@}
};

#endif




