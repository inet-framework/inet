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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#ifndef __BLACKBOARD_H
#define __BLACKBOARD_H

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <omnetpp.h>
#include <map>
#include <vector>


class BlackboardAccess;


/**
 * The Blackboard works as entity to enable inter-layer/inter-process communication.
 *
 * Blackboard makes it possible for several modules (representing e.g. protocol
 * layers) to share information, in a publish-subscribe fashion.
 *
 * Anyone can publish data items on the blackboard. Every item is published with
 * a unique string label. The Blackboard makes no assumption about the format
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
 *
 * Blackboard items have to be subclassed from cPolymorphic, in order to
 * allow for some type safety via dynamic_cast.
 *
 * Clients may browse blackboard contents and subscribe to items. After
 * subscription, clients will receive notifications whenever the subscribed
 * item changes. Clients may also subscribe to get notified whenever
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
 * void Foo::initialize()
 * {
 *     ref = blackboard()->subscribe(this,"routingTable");
 * }
 *
 * void Foo::blackboardItemChanged(BBItemRef item)
 * {
 *     if (item==ref)
 *     {
 *         RoutingTable *rt = check_and_cast<RoutingTable *>(ref->data());
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
 * void Bar::initialize()
 * {
 *     blackboard()->registerClient(this);
 *     // make sure we get what's already on the blackboard
 *     blackboard()->invokePublishedForAllBBItems(this);
 * }
 *
 * void Bar::blackboardItemPublished(BBItemRef item)
 * {
 *     // if label begins with "nic." and it's a NetworkInterfaceData, subscribe
 *     if (!strncmp(item->label(),"nic.",4) &&
 *         dynamic_cast<NetworkInterfaceData *>(item->data()))
 *     {
 *         // new network interface appeared, subscribe to it and do whatever
 *         // other actions are necessary
 *         blackboard()->subscribe(this, item);
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
 *     Blackboard *bb = blackboard();
 *     for (Blackboard::iterator i=bb->begin(); i!=bb->end(); ++i)
 *         if (dynamic_cast<NetworkInterfaceData *>((*i)->data()))
 *             bb->subscribe(this, *i);
 * }
 * </pre>
 *
 * @author Andras Varga
 */
class Blackboard : public cSimpleModule
{
  public:
    typedef std::vector<BlackboardAccess *> SubscriberVector;

    /**
     * Represents a blackboard items.
     */
    class BBItem
    {
      private:
        friend class Blackboard;
        cPolymorphic *_item;
        std::string _label;
        SubscriberVector subscribers;
      public:
        const char *label()  {return _label.c_str();}
        cPolymorphic *data()  {return _item;}
    };

  protected:
    class Iterator;
    friend class Iterator;

    // blackboard items, with their subscriber lists
    typedef std::map<std::string, BBItem*> ContentsMap;
    ContentsMap contents;

    // those who have subscribed to publish()/withdraw() requests
    SubscriberVector addRemoveSubscribers;

  public:
    /**
     * "Handle" to blackboard items. To get the data or the label, write
     * bbref->data() and bbref->label(), respectively. BBItemRefs become
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
    Module_Class_Members(Blackboard, cSimpleModule, 0);
    virtual ~Blackboard();

    /**
     * Initialize BB.
     */
    virtual void initialize();

    /**
     * Does nothing.
     */
    virtual void handleMessage(cMessage *msg);

    /** @name Methods for publishers */
    //@{
    /**
     * Publish new item on the BB, with the given label.
     */
    BBItemRef publish(const char *label, cPolymorphic *item);

    /**
     * Withdraw (unpublish) item from the BB (typically called by publisher).
     */
    void withdraw(BBItemRef bbItem);

    /**
     * Tell BB that an item has changed (typically called by publisher).
     * When item pointer is omitted, it is assumed that the item object
     * was updated "in place" (as opposed to being replaced by another object).
     */
    void changed(BBItemRef bbItem, cPolymorphic *item=NULL);
    //@}

    /** @name Methods for subscribers */
    /**
     * Subscribe to a BB item identified by a label
     */
    BBItemRef subscribe(BlackboardAccess *bbClient, const char *label);

    /**
     * Find item with given label on the BB
     */
    BBItemRef find(const char *label);

    /**
     * Subscribe to a BB item identified by item reference
     */
    BBItemRef subscribe(BlackboardAccess *bbClient, BBItemRef bbItem);

    /**
     * Unsubcribe module from change notifications
     */
    void unsubscribe(BlackboardAccess *bbClient, BBItemRef bbItem);

    /**
     * Start to receive notifications about items being published
     * to/withdrawn from BB.
     */
    void registerClient(BlackboardAccess *bbClient);

    /**
     * The pair of registerClient().
     */
    void deregisterClient(BlackboardAccess *bbClient);

    /**
     * Utility function: the client gets immediate notification with
     * all items currently on clipboard (as if they were added just now).
     * This may simplify initialization code in a subscribe-when-published
     * style client.
     */
    void invokePublishedForAllBBItems(BlackboardAccess *bbClient);

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
 * Gives subscribe access to the blackboard.
 */
class BlackboardAccess
{
  private:
    Blackboard *bb;
  public:
    BlackboardAccess() {bb=NULL;}
    Blackboard *blackboard();

    /** @name Callbacks invoked by the blackboard */
    //@{
    virtual void blackboardItemChanged(BBItemRef item) = 0;
    virtual void blackboardItemPublished(BBItemRef item) = 0;
    virtual void blackboardItemWithdrawn(BBItemRef item) = 0;
    //@}
};

#endif



