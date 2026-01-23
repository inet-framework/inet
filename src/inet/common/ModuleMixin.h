//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MODULEMIXIN_H
#define __INET_MODULEMIXIN_H

#include "inet/common/StringFormat.h"

#include <algorithm>
#include <type_traits>
#include <vector>

namespace inet {

/**
 * A base functionality for all INET modules that implements behavior common to all modules.
 *
 * Additional features compared to cSimpleModule / cModule:
 * - Support for displayStringTextFormat parameter
 * - Derived classes can implement resolveDirective() to support %c style directives that can
 *   be used in displayStringTextFormat.
 */
template<typename T>
class INET_API ModuleMixin : public T, public StringFormat::IResolver
{
  static_assert(std::is_base_of<cModule, T>::value, "Type parameter of ModuleMixin must be a subclass of cModule");

  protected:
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

  protected:
    virtual void initialize() override { T::initialize(); }
    virtual void initialize(int stage) override { T::initialize(stage); }

    virtual void refreshDisplay() const override
    {
        if (T::hasPar("displayStringTextFormat")) {
            auto displayStringTextFormat = T::par("displayStringTextFormat").stringValue();
            if (!opp_isempty(displayStringTextFormat)) {
                auto text = StringFormat::formatString(displayStringTextFormat, this);
                T::getDisplayString().setTagArg("t", 0, text.c_str());
            }
        }
        T::refreshDisplay();
    }

  public:
    // Implementation of StringFormat::IResolver interface
    virtual std::string resolveDirective(char directive) const override
    {
        throw cRuntimeError("Unknown directive: %c", directive);
    }

    virtual std::string resolveExpression(const char *expression) const override
    {
        cCollectObjectsVisitor visitor(expression);
        visitor.processChildrenOf(const_cast<ModuleMixin<T>*>(this));
        if (!visitor.objects.empty()) {
            if (visitor.objects.size() > 1) {
                std::stable_sort(visitor.objects.begin(), visitor.objects.end(), [] (const cObject *o1, const cObject *o2) {
                    return dynamic_cast<const cWatchBase *>(o1) != nullptr && dynamic_cast<const cWatchBase *>(o2) == nullptr;
                });
            }
            return visitor.objects[0]->str();
        }
        else
            throw cRuntimeError("Unknown expression: %s", expression);
    }
};

} // namespace inet

#endif
