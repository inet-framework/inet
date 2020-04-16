//
// Copyright (C) 2006-2019 Opensim Ltd
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

#ifndef __INET_OBJECTPRINTER_H
#define __INET_OBJECTPRINTER_H

#include <vector>
#include <iostream>
#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Controls recursion depth in OpbjectPrinter.
 */
enum ObjectPrinterRecursionControl {
    SKIP,      // don't print this field
    RECURSE,   // print this field in detail by recursing down
    FULL_NAME, // print the full name only (applicable to cObject)
    FULL_PATH  // print the full Path only (applicable to cObject)
};

/**
 * This is function type that controls recursion during printing an object.
 * It will be asked what to do whenever recursion occurs at a compound object's field.
 *
 * Parameters: object that has the field, object's class descriptor, field index, field value,
 * parent objects collected during recursion, recursion level.
 */
typedef ObjectPrinterRecursionControl (*ObjectPrinterRecursionPredicate)(void *, cClassDescriptor *, int, void *, void **, int);

/**
 * A utility class to serialize an object in text form.
 */
class INET_API ObjectPrinter
{
    protected:
        int indentSize;
        char buffer[1024];
        std::vector<cMatchExpression*> objectMatchExpressions;
        std::vector<std::vector<cMatchExpression*> > fieldNameMatchExpressionsList;
        ObjectPrinterRecursionPredicate recursionPredicate;

    public:
        /**
         * Accepts the parsed form of the pattern string. The two vectors
         * must be of the same size. The contained MatchExpression objects
         * will be deallocated by this ObjectPrinter.
         */
        ObjectPrinter(ObjectPrinterRecursionPredicate recursionPredicate,
                      const std::vector<cMatchExpression*>& objectMatchExpressions,
                      const std::vector<std::vector<cMatchExpression*> >& fieldNameMatchExpressionsList,
                      int indentSize=4);

        /**
         * Pattern syntax is that of the "eventlog-message-detail-pattern"
         * configuration entry -- see documentation there.
         *
         * Recommended pattern for packet printing:
         * "*: not className and not fullName and not fullPath and not info and not rawBin and not rawHex"
         *
         * Just some examples here:
         * "*":
         *     captures all fields of all messages
         * "*Msg | *Packet":
         *     captures all fields of classes named AnythingMsg or AnythingPacket
         * "*Frame:*Address,*Id":
         *     captures all fields named anythingAddress and anythingId from
         *     objects of any class named AnythingFrame
         * "MyMessage:declaredOn(MyMessage)":
         *     captures instances of MyMessage recording the fields
         *     declared on the MyMessage class
         * "*:(not declaredOn(cMessage) and not declaredOn(cNamedObject) and
         * not declaredOn(cObject))":
         *     records user-defined fields from all objects
         */
        ObjectPrinter(ObjectPrinterRecursionPredicate recursionPredicate=nullptr, const char *pattern="*", int indentSize=4);

        /**
         * Destructor.
         */
        ~ObjectPrinter();

        void printObjectToStream(std::ostream& ostream, cObject *object);

        std::string printObjectToString(cObject *object);

    protected:
        void printIndent(std::ostream& ostream, int level);
        void printObjectToStream(std::ostream& ostream, void *object, cClassDescriptor *descriptor, void **objects, int level);
        bool matchesObjectField(cObject *object, int fieldIndex);
};

}  // namespace inet


#endif

