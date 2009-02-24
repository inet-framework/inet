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


#include <algorithm>
#include "Blackboard.h"
#include "ModuleAccess.h"

Define_Module(Blackboard);


#define coreEV (ev.isDisabled()||!coreDebug) ? ev : ev <<getParentModule()->getName()<<"["<<getParentModule()->getIndex()<<"]::Blackboard: "


std::ostream& operator<<(std::ostream& os, const Blackboard::BBItem& bbi)
{
    os << bbi.getData()->info();
    return os;
}

Blackboard::Blackboard()
{
}

Blackboard::~Blackboard()
{
    while (!contents.empty())
    {
        ContentsMap::iterator i = contents.begin();
        delete (*i).second;
        contents.erase(i);
    }
}

void Blackboard::initialize()
{
  if(hasPar("coreDebug"))
    coreDebug = par("coreDebug").boolValue();
  else
    coreDebug = false;
    WATCH_PTRMAP(contents);
}

void Blackboard::handleMessage(cMessage *msg)
{
    error("Blackboard doesn't handle messages, it can be accessed via direct method calls");
}

BBItemRef Blackboard::publish(const char *label, cPolymorphic *item)
{
    Enter_Method("publish(\"%s\", %s *ptr)", label, item->getClassName());

    // check uniqueness of label
    ContentsMap::iterator k = contents.find(std::string(label));
    if (k!=contents.end())
        error("publish(): blackboard already contains an item with label `%s'", label);

    // add to BB contents
    BBItem *bbitem = new BBItem();
    bbitem->_item = item;
    bbitem->_label = label;
    contents[bbitem->_label] = bbitem;

    coreEV <<"publish "<<label<<" on bb\n";
    coreEV <<"bbItem->label: "<<bbitem->getLabel()<<endl;
    // notify
    SubscriberVector& vec = registeredClients;
    for (SubscriberVector::iterator i=vec.begin(); i!=vec.end(); ++i){
        (*i)->blackboardItemPublished(bbitem);
    }
    return bbitem;
}

void Blackboard::withdraw(BBItemRef bbItem)
{
    Enter_Method("withdraw(\"%s\")", bbItem->getLabel());

    // find on BB
    ContentsMap::iterator k = contents.find(bbItem->_label);
    if (k==contents.end())
      error("withdraw(): item labelled `%s' is not on clipboard (BBItemRef stale?)", bbItem->_label.c_str());

    // notify subscribers
    SubscriberVector& vec = bbItem->subscribers;
    for (SubscriberVector::iterator i=vec.begin(); i!=vec.end(); ++i)
        (*i)->blackboardItemWithdrawn(bbItem);

    // remove
    contents.erase(k);
    bbItem->_item = NULL; // may make bogus code crash sooner
    delete bbItem;
}

void Blackboard::changed(BBItemRef bbItem, cPolymorphic *item)
{
  coreEV <<"enter changed; item: "<<bbItem->getLabel()<<" changed -> notify subscribers\n";

  Enter_Method("changed(\"%s\", %s *ptr)", bbItem->getLabel(), item->getClassName());

    // update data pointer
    if (item)
        bbItem->_item = item;
    // notify subscribers
    SubscriberVector& vec = bbItem->subscribers;
    for (SubscriberVector::iterator i=vec.begin(); i!=vec.end(); ++i){
        (*i)->blackboardItemChanged(bbItem);
    }
}

BBItemRef Blackboard::subscribe(BlackboardAccess *bbClient, const char *label)
{
    Enter_Method("subscribe(this,\"%s\")", label);

    // look up item by label
    BBItemRef item = find(label);
    if (!item)
       error("subscribe(): item labelled `%s' not on blackboard", label);

    // subscribe
    coreEV <<"subscribe for "<<label<<" on bb\n";
    subscribe(bbClient, item);
    return item;
}

BBItemRef Blackboard::find(const char *label)
{
    ContentsMap::iterator k = contents.find(std::string(label));
    return k==contents.end() ? NULL : (*k).second;
}

BBItemRef Blackboard::subscribe(BlackboardAccess *bbClient, BBItemRef bbItem)
{
    Enter_Method("subscribe(this,\"%s\")", bbItem->getLabel());

    // check if already subscribed
    SubscriberVector& vec = bbItem->subscribers;
    if (std::find(vec.begin(), vec.end(), bbClient)!=vec.end())
        return bbItem; // already subscribed

    // add subscriber
    vec.push_back(bbClient);

    coreEV <<"sucessfully subscribed for item: "<<bbItem->getLabel()<<endl;
    return bbItem;
}

void Blackboard::unsubscribe(BlackboardAccess *bbClient, BBItemRef bbItem)
{
    Enter_Method("unsubscribe(this,\"%s\")", bbItem->getLabel());

    // check if already subscribed
    SubscriberVector& vec = bbItem->subscribers;
    SubscriberVector::iterator k = std::find(vec.begin(), vec.end(), bbClient);
    if (k==vec.end())
        return; // ok, not subscribed

    // remove subscriber
    vec.erase(k);
}

void Blackboard::registerClient(BlackboardAccess *bbClient)
{
    Enter_Method("registerClient(this)");

    // check if already subscribed
    SubscriberVector& vec = registeredClients;
    if (std::find(vec.begin(), vec.end(), bbClient)!=vec.end())
        return; // ok, already subscribed

    // add subscriber
    vec.push_back(bbClient);
}

void Blackboard::removeClient(BlackboardAccess *bbClient)
{
    Enter_Method("removeClient(this)");

    // check if subscribed
    SubscriberVector& vec = registeredClients;
    SubscriberVector::iterator k = std::find(vec.begin(), vec.end(), bbClient);
    if (k==vec.end())
        return; // ok, not subscribed

    // remove subscriber
    vec.erase(k);
}

void Blackboard::getBlackboardContent(BlackboardAccess *bbClient)
{
    Enter_Method("getBlackboardContent(this)");

    for (ContentsMap::iterator i=contents.begin(); i!=contents.end(); ++i)
        bbClient->blackboardItemPublished((*i).second);
}

//----

Blackboard *BlackboardAccess::getBlackboard()
{
    if (!bb)
    {
        bb = ModuleAccess<Blackboard>("blackboard").get();
    }
    return bb;
}


