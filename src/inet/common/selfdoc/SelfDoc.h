//
// Copyright (C) 2019 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// @author: Zoltan Bojthe
//

#ifndef __INET_SELFDOC_H
#define __INET_SELFDOC_H

namespace inet {

class INET_API SelfDoc
{
  protected:
    std::set<std::string> textSet;

  public:
    static OPP_THREAD_LOCAL bool generateSelfdoc;

  public:
    SelfDoc() {}
    ~SelfDoc() noexcept(false);
    void insert(const std::string& text) { if (generateSelfdoc) textSet.insert(text); }
    static bool notInInitialize() { return true; }
    static bool notInInitialize(const char *methodFmt, ...) { return methodFmt != nullptr && (0 != strcmp(methodFmt, "initialize(%d)")); }
    static const char *enterMethodInfo() { return ""; }
    static const char *enterMethodInfo(const char *methodFmt, ...);

    static std::string kindToStr(int kind, cProperties *properties1, const char *propName1, cProperties *properties2, const char *propName2);
    static std::string val(const char *str);
    static std::string val(const std::string& str) { return val(str.c_str()); }
    static std::string keyVal(const std::string& key, const std::string& value) { return val(key) + " : " + val(value); }
    static std::string tagsToJson(const char *key, cMessage *msg);
    static std::string gateInfo(cGate *gate);
};

extern OPP_THREAD_LOCAL SelfDoc globalSelfDoc;

class INET_API SelfDocTempOffClass
{
    bool flag;
  public:
    SelfDocTempOffClass() { flag = SelfDoc::generateSelfdoc; SelfDoc::generateSelfdoc = false; }
    ~SelfDocTempOffClass() { SelfDoc::generateSelfdoc = flag; }
};

// for declare a local SelfDocTempOffClass variable:
#define SelfDocTempOff  SelfDocTempOffClass selfDocTempOff_ ## __LINE__;

#undef Enter_Method
#undef Enter_Method_Silent

#define __Enter_Method_SelfDoc(...) \
        if (SelfDoc::notInInitialize(__VA_ARGS__) && (getSimulation()->getSimulationStage() != STAGE(CLEANUP))) { \
            auto __from = __ctx.getCallerContext(); \
            std::string fromModuleName = __from ? __from->getParentModule() ? __from->getComponentType()->getFullName() : "-=Network=-" : "-=unknown=-"; \
            std::string toModuleName = getSimulation()->getContext()->getComponentType()->getFullName(); \
            std::string keyValFunction = SelfDoc::keyVal("function", std::string(opp_typename(typeid(*this))) + "::" + __func__); \
            std::string keyValInfo = SelfDoc::keyVal("info", SelfDoc::enterMethodInfo(__VA_ARGS__)); \
            { \
                std::ostringstream os; \
                os << "=SelfDoc={ " << SelfDoc::keyVal("module", fromModuleName) \
                   << ", " << SelfDoc::keyVal("action","CALL") \
                   << ", " << SelfDoc::val("details") << " : { " << SelfDoc::keyVal("call to", toModuleName) << ", " << keyValFunction << ", " << keyValInfo << " } }"; \
                globalSelfDoc.insert(os.str()); \
            } \
            { \
                std::ostringstream os; \
                os << "=SelfDoc={ " << SelfDoc::keyVal("module", toModuleName) \
                   << ", " << SelfDoc::keyVal("action","CALLED") \
                   << ", " << SelfDoc::val("details") << " : { " << SelfDoc::keyVal("call from", fromModuleName) << ", " << keyValFunction << ", " << keyValInfo << " } }"; \
                globalSelfDoc.insert(os.str()); \
            } \
        }

// TODO add module relative path when caller and call to are in same networkNode
#define Enter_Method(...) \
        omnetpp::cMethodCallContextSwitcher __ctx(this); __ctx.methodCall(__VA_ARGS__); \
        __Enter_Method_SelfDoc(__VA_ARGS__)

// TODO add module relative path when caller and call to are in same networkNode
#define Enter_Method_Silent(...) \
        omnetpp::cMethodCallContextSwitcher __ctx(this); __ctx.methodCallSilent(__VA_ARGS__); \
        __Enter_Method_SelfDoc(__VA_ARGS__)

} // namespace inet

#endif

