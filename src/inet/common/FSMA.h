//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FSMA_H
#define __INET_FSMA_H

#include "inet/common/INETDefs.h"

namespace inet {

/*
    This is an alternative FSM implementation.

    Here is an example:

    FSMA_Switch(fsm)
    {
        FSMA_State(X)
        {
            FSMA_Event_Transition(XY, isFoo, Y,
                doFoo);
            FSMA_No_Event_Transition(XZ, isFooBar, Z,
                doFooBar);
        }
        FSMA_State(Y)
        {
            FSMA_Event_Transition(YX, isBar, X,
                doBar);
        }
        FSMA_State(Z)
        {
            FSMA_Event_Transition(ZX, isBaz, X,
                doBaz);
        }
    }

    After macro expansion, a state machine code looks like something along
    these lines:

    bool ___is_event = true;
    bool ___exit = false;
    int ___c = 0;
    Fsm& ___fsm = fsm;
    while (!___exit && (___c++ < FSM_MAXT || (throw cRuntimeError(E_INFLOOP, ___fsm.getStateName())))
    {
        if (transition_seen = false, ___exit = true, ___fsm.getState() == X)
        {
            if (!___is_event)
            {
                if (transition_seen)
                    throw cRuntimeError("...");
                // enter code
            }
            transition_seen = true; if (isFoo && ___is_event)
            {
                EV_DEBUG << "firing " << XY << " transition for " << ___fsm.getName() << endl;
                doFoo;
                ___fsm.setState(Y, "Y");
                ___is_event = false;
                ___exit = false;
                continue;
            }
            transition_seen = true; if (isFooBar && !___is_event)
            {
                EV_DEBUG << "firing " << XZ << " transition for " << ___fsm.getName() << endl;
                doFooBar;
                ___fsm.setState(Z, "Z");
                ___exit = false;
                continue;
            }
        }
        if (transition_seen = false, ___exit = true, ___fsm.getState() == Y)
        {
            transition_seen = true; if (isBar && ___is_event)
            {
                EV_DEBUG << "firing " << YX << " transition for " << ___fsm.getName() << endl;
                doVar;
                ___fsm.setState(X, "X");
                ___is_event = false;
                ___exit = false;
                continue;
            }
        }
    }
 */

class SIM_API Fsm : public cOwnedObject
{
  private:
    //
    // About state codes:
    //  initial state is number 0
    //  negative state codes are transient states
    //  positive state codes are steady states
    //
    int state = 0;
    const char *stateName = "INIT";   // just a ptr to an external string
    simsignal_t stateChangedSignal = -1;

  public:
    std::vector<std::function<void ()>> delayedActions;
    bool busy = false;

  private:
    void copy(const Fsm& other) {
        stateName = other.stateName;
        state = other.state;
    }

    virtual void parsimPack(cCommBuffer *) const override {throw cRuntimeError(this, E_CANTPACK);}
    virtual void parsimUnpack(cCommBuffer *) override {throw cRuntimeError(this, E_CANTPACK);}

  public:
    /** @name Constructors, destructor, assignment. */
    //@{

    /**
     * Constructor.
     */
    explicit Fsm(const char *name=nullptr) :
        cOwnedObject(name)
    {
    }

    /**
     * Copy constructor.
     */
    Fsm(const Fsm& other) : cOwnedObject(other) {copy(other);}

    /**
     * Assignment operator. The name member is not copied;
     * see cOwnedObject's operator=() for more details.
     */
    Fsm& operator=(const Fsm& vs) {
        if (this == &vs)
            return *this;
        cOwnedObject::operator=(vs);
        copy(vs);
        return *this;
    }
    //@}

    /** @name Redefined cObject member functions. */
    //@{

    /**
     * Creates and returns an exact copy of this object.
     * See cObject for more details.
     */
    virtual Fsm *dup() const override  {return new Fsm(*this);}

    /**
     * Produces a one-line description of the object's contents.
     * See cObject for more details.
     */
    virtual std::string str() const override {
        std::stringstream out;
        if (!stateName)
            out << "state: " << state;
        else
            out << "state: " << stateName << " (" << state << ")";
        return out.str();
    }
    //@}

    /** @name FSM functions. */
    //@{

    void setStateChangedSignal(simsignal_t stateChangedSignal) {
        this->stateChangedSignal = stateChangedSignal;
    }

    /**
     * Returns the state the FSM is currently in.
     */
    int getState() const  {return state;}

    /**
     * Returns the name of the state the FSM is currently in.
     */
    const char *getStateName() const {return stateName?stateName:"";}

    /**
     * Sets the state of the FSM. This method is usually invoked through
     * the FSM_Goto() macro.
     *
     * The first arg is the state code. The second arg is the name of the state.
     * setState() assumes this is pointer to a string literal (the string is
     * not copied, only the pointer is stored).
     *
     * @see FSM_Goto
     */
    void setState(int state, const char *stateName) {
        this->state = state;
        this->stateName = stateName;
        if (stateChangedSignal != -1)
            cSimulation::getActiveSimulation()->getContextModule()->emit(stateChangedSignal, state);
    }

    void insertDelayedAction(std::function<void ()> action) {
        delayedActions.push_back(action);
    }

    void executeDelayedActions() {
        auto actions = delayedActions;
        delayedActions.clear();
        for (auto action : actions)
            action();
    }
    //@}
};

class FsmContext
{
  private:
    Fsm& fsm;

  public:
    FsmContext(Fsm& fsm) : fsm(fsm) {
        ASSERT(!fsm.busy);
        fsm.busy = true;
    }
    ~FsmContext() {
        fsm.busy = false;
    }
};

#define FSMA_Switch(fsm) \
    Fsm& ___fsm = (fsm); \
    ASSERT(___fsm.delayedActions.empty()); \
    FsmContext fsmContext(___fsm); \
    bool ___is_event = true; \
    bool ___exit = false; \
    bool ___transition_seen = false; (void)___transition_seen; \
    int ___c = 0; \
    bool ___logging = true; \
    if (___logging) EV_DEBUG << "FSM " << ___fsm.getName() << ": processing event in state " << ___fsm.getStateName() << "\n"; \
    while (!___exit && (___c++ < FSM_MAXT || (throw cRuntimeError(E_INFLOOP, ___fsm.getStateName()), 0)))

#define FSMA_SetLogging(enabled) \
        ___logging = enabled;

#define FSMA_Print(exiting) \
    if (___logging) EV_DEBUG << "FSM " << ___fsm.getName() << ((exiting) ? ": leaving state " : ": entering state ") << ___fsm.getStateName() << endl

#define FSMA_State(s)    if (___transition_seen = false, ___exit = true, ___fsm.getState() == (s))

#define FSMA_Enter(action) \
    if (!___is_event) \
    { \
        if (___transition_seen) \
            throw cRuntimeError(&___fsm, "FSMA_Enter() must precede all FSMA_*_Transition()'s in the code"); \
        action; \
    }

#define FSMA_Stay(condition, action) \
    ___transition_seen = true; if ((condition) && ___is_event) \
    { \
        if (___logging) EV_DEBUG << "FSM " << ___fsm.getName() << ": condition \"" << #condition << "\" holds, staying in current state" << endl; \
        action; \
        if (___logging) EV_DEBUG << "FSM " << ___fsm.getName() << ": done processing associated actions\n"; \
        ___is_event = false; \
    }

#define FSMA_Event_Transition(transition, condition, target, action) \
    ___transition_seen = true; if ((condition) && ___is_event) \
    { \
        ___is_event = false; \
        FSMA_Transition_Internal(transition, (condition), target, action)

#define FSMA_No_Event_Transition(transition, condition, target, action) \
    ___transition_seen = true; if ((condition) && !___is_event) \
    { \
        FSMA_Transition_Internal(transition, (condition), target, action)

#define FSMA_Transition(transition, condition, target, action) \
    ___transition_seen = true; if ((condition)) \
    { \
        ___is_event = false; \
        FSMA_Transition_Internal(transition, (condition), target, action)

#define FSMA_Transition_Internal(transition, condition, target, action) \
    FSMA_Print(true); \
    if (___logging) EV_DEBUG << "FSM " << ___fsm.getName() << ": condition \"" << #condition << "\" holds, taking transition \"" << #transition << "\" to state " << #target << endl; \
    action; \
    ___fsm.setState(target, #target); \
    if (___logging) EV_DEBUG << "FSM " << ___fsm.getName() << ": done processing associated actions\n"; \
    FSMA_Print(false); \
    ___exit = false; \
    continue; \
    }

#define FSMA_Ignore_Event(condition) \
    ___transition_seen = true; if ((condition) && ___is_event) \
    { \
        if (___logging) EV_DEBUG << "FSM " << ___fsm.getName() << ": condition \"" << #condition << "\" holds, ignoring in current state" << endl; \
        ___is_event = false; \
    }

#define FSMA_Fail_On_Unhandled_Event() \
    ___transition_seen = true; if (___is_event) \
    { \
        throw cRuntimeError(&___fsm, "Unhandled event in state %s", ___fsm.getStateName()); \
    }

#define FSMA_Delay_Action(body) \
    ___fsm.insertDelayedAction([=] () { body; });

} // namespace inet

#endif

