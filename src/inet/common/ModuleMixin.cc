//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/StringFormat.h"

#include <algorithm>
#include <type_traits>
#include <vector>

namespace inet {

namespace {

class cCollectObjectsVisitor : public cVisitor
{
  public:
    const char *name;
    std::vector<cObject*> objects;

  public:
    cCollectObjectsVisitor(const char *name): name(name) { }

  protected:
    virtual bool visit(cObject *object) override {
        if (object->isName(name))
            objects.push_back(object);
        return true;
    }
};

} // unnamed namespace

namespace internal {

void refreshDisplayString(cModule *thisModule, const StringFormat::IResolver *thisModuleAsResolver)
{
    if (thisModule->hasPar("displayStringTextFormat")) {
        try {
            auto displayStringTextFormat = thisModule->par("displayStringTextFormat").stringValue();
            if (!opp_isempty(displayStringTextFormat)) {
                auto text = StringFormat::formatString(displayStringTextFormat, thisModuleAsResolver);
                thisModule->getDisplayString().setTagArg("t", 0, text.c_str());
            }
        }
        catch (cException& e) {
            e.prependMessage("While processing displayStringTextFormat: ");
            throw;
        }
    }
}

std::string doResolveExpression(cModule *thisModule, const char *expression)
{
    const char *lastDot = strrchr(expression, '.');

    cModule *targetModule = thisModule;
    const char *fieldName = expression;

    if (lastDot != nullptr) {
        // Extract submodule path (everything before the last dot)
        std::string submodulePath(expression, lastDot - expression);
        targetModule = thisModule->getModuleByPath(submodulePath.c_str());
        fieldName = lastDot + 1;
    }

    cCollectObjectsVisitor visitor(fieldName);
    visitor.processChildrenOf(targetModule);

    if (visitor.objects.empty())
        throw cRuntimeError("Unknown expression: %s", expression);

    if (visitor.objects.size() > 1) {
        std::stable_sort(visitor.objects.begin(), visitor.objects.end(), [] (const cObject *o1, const cObject *o2) {
            return dynamic_cast<const cWatchBase *>(o1) != nullptr && dynamic_cast<const cWatchBase *>(o2) == nullptr;
        });
    }

    // special case so that strings are displayed without quotes
    if (auto *par = dynamic_cast<cPar *>(visitor.objects[0])) {
        if (par->getType() == cPar::STRING)
            return par->stdstringValue();
    }
    if (auto *watchBase = dynamic_cast<cWatchBase *>(visitor.objects[0])) {
        any_ptr ptr = watchBase->getValuePointer();
        if (ptr.contains<std::string>())
            return *ptr.get<std::string>();
    }

    return visitor.objects[0]->str();
}

} // namespace internal

} // namespace inet
