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

#include <string.h>
#include <omnetpp.h>
#include "utils.h"

char *fgetline (FILE *fp)
{
    // alloc buffer and read a line
    char *line = new char[MAX_LINE];
    if (fgets(line,MAX_LINE,fp)==NULL)
        return NULL;

    // chop CR/LF
    line[MAX_LINE-1] = '\0';
    int len = strlen(line);
    while (len>0 && (line[len-1]=='\n' || line[len-1]=='\r'))
        line[--len]='\0';

    return line;
}

//---

MessageTracer MessageTracer::trc;
unsigned int MessageId::nextId;
unsigned int MessageId::nextTreeId;

MessageTracer::MessageTracer()
{
    f = NULL;
    lastId = NULL;
}

MessageTracer::~MessageTracer()
{
    if (f)
        fclose(f);
}

void MessageTracer::open()
{
    f = fopen("msgtrace.out","w");
}

void MessageTracer::created(MessageId *m)
{
    if (!f) open();
    if (lastId) fprintf(f,"C I:%d T:%d P:%d E:%ld %s\n", lastId->id, lastId->treeId, lastId->parentId,
                        simulation.eventNumber(), simulation.contextModule()->fullPath());
    lastId = m;
}

void MessageTracer::cloned(MessageId *m)
{
    if (!f) open();
    fprintf(f,"L I:%d T:%d P:%d E:%ld %s\n", m->id, m->treeId, m->parentId,
            simulation.eventNumber(), simulation.contextModule()->fullPath());
}

void MessageTracer::assigned(MessageId *m)
{
    if (!f) open();
    fprintf(f,"A I:%d T:%d P:%d E:%ld %s\n", m->id, m->treeId, m->parentId,
            simulation.eventNumber(), simulation.contextModule()->fullPath());
    if (m==lastId) lastId = NULL;
}

void MessageTracer::deleted(MessageId *m)
{
    fprintf(f,"D I:%d T:%d P:%d E:%ld %s\n", m->id, m->treeId, m->parentId,
            simulation.eventNumber(), simulation.contextModule()->fullPath());
}




