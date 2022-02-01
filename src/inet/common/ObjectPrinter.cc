//
// Copyright (C) 2006-2019 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "ObjectPrinter.h"

#include "MatchableField.h"
#include "MatchableObject.h"

namespace inet {

#define MAXIMUM_OBJECT_PRINTER_LEVEL    20

#define QUOTE(txt)   (needsQuotes(txt) ? quoteString(txt).c_str() : (txt))

static bool needsQuotes(const char *txt)
{
    if (!txt[0])
        return true;
    for (const char *s = txt; *s; s++)
        if (*s == ' ' || *s == '\\' || *s == '"' || *s < ' ')
            return true;
    return false;
}

static std::string quoteString(const std::string& txt)
{
    char *buf = new char[4 * txt.length() + 3]; // a conservative guess
    char *d = buf;
    *d++ = '"';
    const char *s = txt.c_str();
    while (*s) {
        switch (*s) {
            case '\b': *d++ = '\\'; *d++ = 'b'; s++; break;
            case '\f': *d++ = '\\'; *d++ = 'f'; s++; break;
            case '\n': *d++ = '\\'; *d++ = 'n'; s++; break;
            case '\r': *d++ = '\\'; *d++ = 'r'; s++; break;
            case '\t': *d++ = '\\'; *d++ = 't'; s++; break;
            case '"':  *d++ = '\\'; *d++ = '"'; s++; break;
            case '\\': *d++ = '\\'; *d++ = '\\'; s++; break;
            default: if (*s < ' ') {*d++='\\'; *d++='x'; sprintf(d,"%2.2X",*s++); d+=2; }
                     else {*d++ = *s++; }
        }
    }
    *d++ = '"';
    *d = '\0';

    std::string ret = buf;
    delete[] buf;
    return ret;
}

static ObjectPrinterRecursionControl defaultRecurseIntoMessageFields(any_ptr object, cClassDescriptor *descriptor, int fieldIndex, any_ptr fieldValue, any_ptr *parents, int level)
{
    const char *fieldName = descriptor->getFieldName(fieldIndex);
    return strcmp(fieldName, "owner") ? RECURSE : SKIP;
}

ObjectPrinter::ObjectPrinter(ObjectPrinterRecursionPredicate recursionPredicate,
        const std::vector<cMatchExpression *>& objectMatchExpressions,
        const std::vector<std::vector<cMatchExpression *>>& fieldNameMatchExpressionsList,
        int indentSize)
{
    ASSERT(objectMatchExpressions.size() == fieldNameMatchExpressionsList.size());
    this->recursionPredicate = recursionPredicate ? recursionPredicate : defaultRecurseIntoMessageFields;
    this->objectMatchExpressions = objectMatchExpressions;
    this->fieldNameMatchExpressionsList = fieldNameMatchExpressionsList;
    this->indentSize = indentSize;
}

ObjectPrinter::ObjectPrinter(ObjectPrinterRecursionPredicate recursionPredicate, const char *objectFieldMatcherPattern, int indentSize)
{
    std::vector<cMatchExpression *> objectMatchExpressions;
    std::vector<std::vector<cMatchExpression *>> fieldNameMatchExpressionsList;

    cStringTokenizer tokenizer(objectFieldMatcherPattern, "|;");
    std::vector<std::string> patterns = tokenizer.asVector();

    for (auto& pattern : patterns) {
        char *objectPattern = (char *)pattern.c_str();
        char *fieldNamePattern = strchr(objectPattern, ':');

        if (fieldNamePattern) {
            *fieldNamePattern = '\0';
            cStringTokenizer fieldNameTokenizer(fieldNamePattern + 1, ",");
            std::vector<std::string> fieldNamePatterns = fieldNameTokenizer.asVector();
            std::vector<cMatchExpression *> fieldNameMatchExpressions;

            for (auto& fieldNamePattern : fieldNamePatterns)
                fieldNameMatchExpressions.push_back(new cMatchExpression(fieldNamePattern.c_str(), false, true, true));

            fieldNameMatchExpressionsList.push_back(fieldNameMatchExpressions);
        }
        else {
            std::vector<cMatchExpression *> fieldNameMatchExpressions;
            fieldNameMatchExpressions.push_back(new cMatchExpression("*", false, true, true));
            fieldNameMatchExpressionsList.push_back(fieldNameMatchExpressions);
        }

        objectMatchExpressions.push_back(new cMatchExpression(objectPattern, false, true, true));
    }

    ASSERT(objectMatchExpressions.size() == fieldNameMatchExpressionsList.size());
    this->recursionPredicate = recursionPredicate ? recursionPredicate : defaultRecurseIntoMessageFields;
    this->objectMatchExpressions = objectMatchExpressions;
    this->fieldNameMatchExpressionsList = fieldNameMatchExpressionsList;
    this->indentSize = indentSize;
}

ObjectPrinter::~ObjectPrinter()
{
    for (int i = 0; i < (int)objectMatchExpressions.size(); i++) {
        delete objectMatchExpressions[i];
        std::vector<cMatchExpression *>& fieldNameMatchExpressions = fieldNameMatchExpressionsList[i];
        for (auto& fieldNameMatchExpression : fieldNameMatchExpressions)
            delete fieldNameMatchExpression;
    }
}

void ObjectPrinter::printObjectToStream(std::ostream& ostream, cObject *object)
{
    any_ptr parents[MAXIMUM_OBJECT_PRINTER_LEVEL];
    cClassDescriptor *descriptor = cClassDescriptor::getDescriptorFor(object);
    ostream << "class " << descriptor->getName() << " {\n";
    printObjectToStream(ostream, toAnyPtr(object), descriptor, parents, 0);
    ostream << "}\n";
}

std::string ObjectPrinter::printObjectToString(cObject *object)
{
    std::stringstream out;
    printObjectToStream(out, object);
    return out.str();
}

void ObjectPrinter::printObjectToStream(std::ostream& ostream, any_ptr object, cClassDescriptor *descriptor, any_ptr *parents, int level)
{
    if (level == MAXIMUM_OBJECT_PRINTER_LEVEL) {
        printIndent(ostream, level);
        ostream << "<pruned>\n";
        return;
    }
    else {
        for (int i = 0; i < level; i++) {
            if (parents[i] == object) {
                printIndent(ostream, level);
                ostream << "<recursion>\n";
                return;
            }
        }
    }
    if (!descriptor) {
        printIndent(ostream, level);
        if (level == 0)
            ostream << "{...}\n";
        else
            ostream << "...\n";
    }
    else {
        parents[level] = object;
        for (int fieldIndex = 0; fieldIndex < descriptor->getFieldCount(); fieldIndex++) {
            bool isArray = descriptor->getFieldIsArray(fieldIndex);
            bool isPointer = descriptor->getFieldIsPointer(fieldIndex);
            bool isCompound = descriptor->getFieldIsCompound(fieldIndex);
            bool isCObject = descriptor->getFieldIsCObject(fieldIndex);
            const char *fieldType = descriptor->getFieldTypeString(fieldIndex);
            const char *fieldName = descriptor->getFieldName(fieldIndex);

            int size = isArray ? descriptor->getFieldArraySize(object, fieldIndex) : 1;
            for (int elementIndex = 0; elementIndex < size; elementIndex++) {
                any_ptr fieldValue = isCompound ? descriptor->getFieldStructValuePointer(object, fieldIndex, elementIndex) : any_ptr(nullptr);

                ObjectPrinterRecursionControl result = RECURSE;
                if (recursionPredicate)
                    result = recursionPredicate(object, descriptor, fieldIndex, fieldValue, parents, level);
                if (result == SKIP || (descriptor->extendsCObject() && !matchesObjectField(fromAnyPtr<cObject>(object), fieldIndex)))
                    continue;

                printIndent(ostream, level + 1);
                ostream << fieldType << " ";

                if (isPointer)
                    ostream << "*";
                ostream << fieldName;

                if (isArray)
                    ostream << "[" << elementIndex << "]";
                ostream << " = ";

                if (isCompound) {
                    if (fieldValue != nullptr) {
                        cClassDescriptor *fieldDescriptor = isCObject ? cClassDescriptor::getDescriptorFor(fromAnyPtr<cObject>(fieldValue)) :
                            cClassDescriptor::getDescriptorFor(descriptor->getFieldStructName(fieldIndex));

                        if (isCObject && result == FULL_NAME)
                            ostream << fromAnyPtr<cObject>(fieldValue)->getFullName() << "\n";
                        else if (isCObject && result == FULL_PATH)
                            ostream << fromAnyPtr<cObject>(fieldValue)->getFullPath() << "\n";
                        else if (fieldDescriptor) {
                            if (isCObject)
                                ostream << "class " << fromAnyPtr<cObject>(fieldValue)->getClassName() << " ";
                            else
                                ostream << "struct " << descriptor->getFieldStructName(fieldIndex) << " ";

                            ostream << "{\n";
                            printObjectToStream(ostream, fieldValue, fieldDescriptor, parents, level + 1);
                            printIndent(ostream, level + 1);
                            ostream << "}\n";
                        }
                        else {
                            std::string value = descriptor->getFieldValueAsString(object, fieldIndex, elementIndex);
                            ostream << QUOTE(value.c_str()) << "\n";
                        }
                    }
                    else
                        ostream << "nullptr\n";
                }
                else {
                    std::string value = descriptor->getFieldValueAsString(object, fieldIndex, elementIndex);
                    ostream << QUOTE(value.c_str()) << "\n";
                }
            }
        }
    }
}

void ObjectPrinter::printIndent(std::ostream& ostream, int level)
{
    int count = level * indentSize;
    for (int i = 0; i < count; i++)
        ostream << " ";
}

bool ObjectPrinter::matchesObjectField(cObject *object, int fieldIndex)
{
    const MatchableObject matchableObject(MatchableObject::ATTRIBUTE_CLASSNAME, object);

    for (int i = 0; i < (int)objectMatchExpressions.size(); i++) {
        cMatchExpression *objectMatchExpression = objectMatchExpressions[i];

        if (objectMatchExpression->matches(&matchableObject)) {
            std::vector<cMatchExpression *>& fieldNameMatchExpressions = fieldNameMatchExpressionsList[i];

            for (auto fieldNameMatchExpression : fieldNameMatchExpressions) {
                const MatchableField matchableField(object, fieldIndex);

                if (fieldNameMatchExpression->matches(&matchableField))
                    return true;
            }
        }
    }

    return false;
}

} // namespace inet

