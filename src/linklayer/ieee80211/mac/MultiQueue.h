//
// Copyright (C) 2011 Alfonso Ariza, Universidad de Malaga
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

// Default Max Size is 100 packets and the default number of queues are 1
#ifndef MULTIQUEUE_H_
#define MULTIQUEUE_H_
#include <vector>
#include <list>
#include <utility>
#include "IQoSClassifier.h"

class INET_API MultiQueue : public cObject
{
    protected:
        typedef std::pair<simtime_t, cMessage *> Data;
        typedef std::list<Data> Queue;
        Queue::iterator position;
        unsigned int exploreQueue;

        std::pair<int, cMessage *> firstPk;
        cMessage * lastPk;
        std::vector<Queue> queues;
        std::vector<int> basePriority;
        std::vector<int> priority;
        IQoSClassifier * classifier;
        unsigned int maxSize;
        unsigned int numStrictQueuePriorities;
        bool isFirst;
    public:
        MultiQueue();
        virtual ~MultiQueue();
        void setNumQueues(int num);
        void setNumStrictPrioritiesQueue(int num)
        {
            numStrictQueuePriorities = num;
        }
        unsigned int getNumStrictPrioritiesQueues()
        {
            return numStrictQueuePriorities;
        }
        unsigned int getNumQueques()
        {
            return queues.size();
        }
        void setMaxSize(unsigned int i)
        {
            maxSize = i;
        }
        unsigned int getMaxSize()
        {
            return maxSize;
        }
        unsigned int size(int i = -1);
        bool empty(int i = -1);
        cMessage* front(int i = -1);
        cMessage* back(int i = -1);
        void push_front(cMessage* val, int i = -1);
        void pop_front(int i = -1);
        void push_back(cMessage* val, int i = -1);
        void pop_back(int i = -1);
        void createClassifier(const char * classifierClass)
        {
            classifier = check_and_cast<IQoSClassifier*>(createOne(classifierClass));
            setNumQueues(classifier->getNumQueues());
        }
        int getCost(int i)
        {
            if (i >= (int) queues.size())
                throw cRuntimeError(this, "Queue doens't exist");
            return basePriority[i];
        }
        void setCost(int i, int cost)
        {
            if (i >= (int) queues.size())
                throw cRuntimeError(this, "Queue doens't exist");
            basePriority[i] = priority[i] = cost;
        }
        cMessage * initIterator();
        cMessage * next();
        bool isEnd();
};

#endif /* MULTIQUEUE_H_ */
