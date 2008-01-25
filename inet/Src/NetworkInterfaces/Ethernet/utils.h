/*
 * Copyright (C) 2003 CTIE, Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _ETH_UTILS_H_
#define _ETH_UTILS_H_

#include <stdio.h>
#include "INETDefs.h"  // for EV


#define MAX_LINE 100


// Function reads from a file stream pointed to by 'fp' and stores characters until the '\n' or EOF
// character is found, the resultant string is returned.  Note that neither '\n' nor EOF character
// is stored to the resultant string, also note that if on a line containing useful data that EOF occurs,
// then that line will not be read in, hence must terminate file with unused line.
extern char* fgetline(FILE *fp);


class MessageId;

/**
 * Experimental class to help tracing messages.
 */
class INET_API MessageTracer
{
  protected:
    static MessageTracer trc;
    FILE *f;
    MessageId *lastId;
    void open();
  public:
    static inline MessageTracer *instance() {return &trc;}
    MessageTracer();
    ~MessageTracer();
    void created(MessageId *m);
    void cloned(MessageId *m);
    void assigned(MessageId *m);
    void deleted(MessageId *m);
};


/**
 * Experimental class to help tracing messages.
 */
class INET_API MessageId
{
  public:
    static unsigned int nextId;
    static unsigned int nextTreeId;
    unsigned int id;
    unsigned int treeId;
    unsigned int parentId;
  public:
    MessageId() {id=++nextId; treeId=++nextTreeId; parentId=0; MessageTracer::instance()->created(this);}
    MessageId(const MessageId& m) {id=++nextId; treeId=m.treeId; parentId=m.id; MessageTracer::instance()->cloned(this);}
    ~MessageId() {MessageTracer::instance()->deleted(this);}
    MessageId& operator=(const MessageId& m) {id=++nextId; treeId=m.treeId; parentId=m.id; MessageTracer::instance()->assigned(this); return *this;}
};

inline std::ostream& operator<<(std::ostream& out, const MessageId& m)
{
    out << "Id:" << m.id << " TreeId: " << m.treeId << " ParentId:" << m.parentId; return out;
}

#endif

