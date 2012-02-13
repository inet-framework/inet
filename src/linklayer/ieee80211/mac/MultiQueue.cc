//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "MultiQueue.h"

MultiQueue::MultiQueue()
{
    classifier = NULL;
    firstPk.second = NULL;
    lastPk = NULL;
    queues.resize(1);
    basePriority.resize(1);
    priority.resize(1);
    basePriority[0] = 1;
    priority[0] = 1;
    maxSize = 1000000;
    numStrictQueuePriorities = 0;
    exploreQueue = 1;
}

MultiQueue::~MultiQueue()
{
    // TODO Auto-generated destructor stub
    while (queues.empty())
    {
        while (!queues.back().empty())
        {
            delete queues.back().back().second;
            queues.back().pop_back();
        }
        queues.pop_back();
    }
    if (firstPk.second != NULL)
        delete firstPk.second;
}

void MultiQueue::setNumQueues(int num)
{
    exploreQueue = num;  // set iterator to null
    basePriority.resize(num);
    priority.resize(num);
    if (num > (int) queues.size())
    {

        for (int i = (int) queues.size() - 1; i <= num; i++)
        {
            basePriority[0] = 1;
            priority[0] = 1;
        }
        queues.resize(num);
    }
    else if (num < (int) queues.size())
    {
        while ((int) queues.size() > num)
        {
            while (!queues.back().empty())
            {
                delete queues.back().back().second;
                queues.back().pop_back();
            }
            queues.pop_back();
        }
    }
}

unsigned int MultiQueue::size(int i)
{
    if (i >= (int) queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");
    if (firstPk.second == NULL)
        return 0;

    if (i != -1)
    {
        if (firstPk.first == i)
            return queues[i].size() + 1;
        return queues[i].size();
    }

    unsigned int total = 1; // the first packet
    for (unsigned int j = 0; j < queues.size(); j++)
        total += queues[j].size();
    return total;
}

bool MultiQueue::empty(int i)
{
    if (i >= (int) queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");

    if (firstPk.second == NULL)
        return true;
    // if here firstPk.second != NULL
    if (i == -1)
        return false;
    if (firstPk.first == i)
        return false;
    return queues[i].empty();
}

cMessage* MultiQueue::front(int i)
{
    if (i >= (int) queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");
    if (firstPk.second == NULL)
        return NULL; // empty queue
    if (i != -1)
    {
        if (firstPk.first == i)
            return firstPk.second;
        if (!queues[i].empty())
            return queues[i].front().second;
        else
            return NULL;
    }
    return firstPk.second;
}

cMessage* MultiQueue::back(int i)
{
    if (i >= (int) queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");
    if (firstPk.second == NULL)
        return NULL;
    if (i != -1)
        return queues[i].back().second;
    if (lastPk != NULL)
        return lastPk;
    return firstPk.second;
}

void MultiQueue::push_front(cMessage* val, int i)
{
    std::pair<simtime_t, cMessage*> value;

    opp_error("this method doesn't be used");

    if (i >= (int) queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");

    if (size() > getMaxSize())
    {
        // first make space
        // delete the oldest, the fist can't be deleted because can be in process.
        simtime_t max;
        int nQueue = -1;
        for (unsigned int j = 0; j < queues.size(); j++)
        {
            if (queues[j].front().first > max)
            {
                max = queues[j].front().first;
                nQueue = j;
            }
        }
        if (nQueue >= 0)
        {
            delete queues[nQueue].front().second;
            queues[nQueue].pop_front();
        }
    }

    if (i != -1)
    {
        if (firstPk.second == NULL)
        {
            firstPk.second = val;
            firstPk.first = i;
            return;
        }
        value = std::make_pair(simTime(), val);
        queues[i].push_front(value);
    }
    else if (!classifier)
    {
        if (firstPk.second == NULL)
        {
            firstPk.second = val;
            firstPk.first = classifier->classifyPacket(val);
            return;
        }
        value = std::make_pair(simTime(), val);
        queues[classifier->classifyPacket(val)].push_front(value);
    }
    else
    {
        if (firstPk.second == NULL)
        {
            firstPk.second = val;
            firstPk.first = 0;
            return;
        }
        queues[0].push_front(value);
    }
}

void MultiQueue::pop_front(int i)
{
    std::pair<simtime_t, cMessage*> value;
    if (i >= (int) queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");
    if (firstPk.second == NULL)
        return;
    if (i != -1 && i != firstPk.first)
    {
        queues[i].pop_front();
        priority[i] = basePriority[i];
        return;
    }
    else
    {
        firstPk.second = NULL;
        // First, search in the strict priority queue
        // decrease all priorities
        for (unsigned int j = 0; j < numStrictQueuePriorities; j++)
        {
            if (!queues[j].empty())
            {
                firstPk.second = queues[j].front().second;
                queues[j].pop_front();
                firstPk.first = j;
                return;
            }
        }

        // if strict priority queues are empty search in the others
        for (unsigned int j = numStrictQueuePriorities; j < queues.size(); j++)
        {
            priority[i]--;
            if (priority[j] < 0 && queues.empty())
                priority[j] = 0;
        }

        priority[firstPk.first] = basePriority[firstPk.first];
        // select the next packet
        int min = 10000000;
        int nQueue = -1;

        for (int j = numStrictQueuePriorities; j < (int) queues.size(); j++)
        {
            if (queues[j].empty())
                continue;
            if (priority[j] < min)
            {
                min = priority[j];
                nQueue = j;
            }
        }

        if (nQueue >= 0)
        {
            firstPk.second = queues[nQueue].front().second;
            queues[nQueue].pop_front();
            firstPk.first = nQueue;
        }
    }
}

void MultiQueue::push_back(cMessage* val, int i)
{
    std::pair<simtime_t, cMessage*> value;
    if (i >= (int) queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");

    if (firstPk.second == NULL)
    {
        firstPk.second = val;
        if (i!=-1)
            firstPk.first = i;
        else
            firstPk.first = classifier->classifyPacket(val);
        lastPk = val;
        return;
    }

    if (size() > getMaxSize())
    {
        // first make space
        // delete the oldest, the fist can't be deleted because can be in process.
        simtime_t max;
        int nQueue = -1;
        for (unsigned int j = 0; j < queues.size(); j++)
        {
            if (queues[j].front().first > max)
            {
                max = queues[j].front().first;
                nQueue = j;
            }
        }
        if (nQueue >= 0)
        {
            delete queues[nQueue].front().second;
            queues[nQueue].pop_front();
        }
    }

    value = std::make_pair(simTime(), val);
    if (i != -1)
    {
        queues[i].push_back(value);
    }
    else if (classifier)
    {
        queues[classifier->classifyPacket(val)].push_back(value);
    }
    else
    {
        queues[0].push_back(value);
    }
    lastPk = val;
}

void MultiQueue::pop_back(int i)
{
    std::pair<simtime_t, cMessage*> value;
    if (i >= (int) queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");

    if (firstPk.second == NULL)
        return;

    if (i != -1)
    {
        if (lastPk == queues[i].back().second)
            lastPk = NULL;
        queues[i].pop_back();
        if (lastPk != NULL)
            return; // nothing to do
        if (this->empty())
            return;

    }
    else
    {
        for (unsigned int j = 0; j < queues.size(); j++)
        {
            if (queues[j].empty())
                continue;
            if (lastPk == queues[j].back().second)
                lastPk = NULL;
        }

    }
    if (this->empty())
        return;
    simtime_t max;
    int nQueue = -1;
    for (unsigned int j = 0; j < queues.size(); j++)
    {
        if (queues[j].back().first > max)
        {
            max = queues[j].back().first;
            nQueue = j;
        }
    }
    if (nQueue >= 0)
    {
        lastPk = queues[nQueue].back().second;
    }
}

cMessage* MultiQueue::initIterator()
{

    isFirst = true;
    exploreQueue = 0;
    position = queues[0].begin();

    if (firstPk.second != NULL)
    {
        return firstPk.second;
    }
    return NULL;
}

cMessage* MultiQueue::next()
{
    // if strict priority queues are empty search in the others
    if (isFirst)
    {
        isFirst = false;
        for (unsigned int j = 0; j < queues.size(); j++)
        {
            if (queues[j].empty())
                continue;
            exploreQueue = j;
            position = queues[j].begin();
            return queues[j].front().second;
        }
        return NULL; // if arrive here the queue is empty return NULL
    }

    while (position==queues[exploreQueue].end())
    {
        exploreQueue++;
        if (exploreQueue < queues.size())
        {
            position = queues[exploreQueue].begin();
            if (position!=queues[exploreQueue].end())
                return position->second;
        }
        else
            return NULL;
    }

    position++;
    while (position==queues[exploreQueue].end())
    {
         if (exploreQueue < queues.size())
         {
             exploreQueue++;
             position = queues[exploreQueue].begin();
             if (position!=queues[exploreQueue].end())
                 return position->second;
         }
         else
             return NULL;
    }
    return position->second;
}

bool  MultiQueue::isEnd()
{
    if (exploreQueue >= queues.size())
        return true;
    if (position==queues[exploreQueue].end())
    {
        // check next queues has data
        for (unsigned int i = exploreQueue+1; i<queues.size(); i++)
        {
            if (!queues[i].empty())
                return false; // there are more data
        }
        position=queues[queues.size()].end(); // the rest of queues are empty, force position to he end of the last queue
        exploreQueue = queues.size();
        return true;
    }
    return false;
}
