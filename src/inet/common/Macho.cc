// Macho - C++ Machine Objects
//
// The Machine Objects class library (in short Macho) allows the creation of
// state machines based on the "State" design pattern in straight C++. It
// extends the pattern with the option to create hierarchical state machines,
// making it possible to convert the popular UML statechart notation to working
// code in a straightforward way. Other features are entry and exit actions,
// state histories and state variables.
//
// Copyright (c) 2005 by Eduard Hiti (feedback to macho@ehiti.de)
//
// Version 0.9.7 (released 2007-12-1)
//
// See Macho.hpp for more information.

#include "inet/common/Macho.h"

namespace inet {

using namespace Macho;

////////////////////////////////////////////////////////////////////////////////
// Helper functions for tracing.
#ifdef MACHO_TRACE

#include <iostream>

void MACHO_TRC1(const char *msg)
{
    std::cout << msg << std::endl;
}

void MACHO_TRC2(const char *state, const char *msg)
{
    std::cout << "State " << state << ": " << msg << std::endl;
}

void MACHO_TRC3(const char *state, const char *msg1, const char *msg2)
{
    std::cout << "State " << state << ": " << msg1 << " " << msg2 << std::endl;
}

#else // ifdef MACHO_TRACE

#define MACHO_TRC1(MSG)
#define MACHO_TRC2(STATE, MSG)
#define MACHO_TRC3(STATE, MSG1, MSG2)

#endif    //  MACHO_TRACE

////////////////////////////////////////////////////////////////////////////////
// Box for states which don't declare own Box class.
_EmptyBox _EmptyBox::theEmptyBox;

////////////////////////////////////////////////////////////////////////////////
// Helper functions for box creation
template<>
void *Macho::_createBox<_EmptyBox>(void *& place)
{
    return &_EmptyBox::theEmptyBox;
}

template<>
void Macho::_deleteBox<_EmptyBox>(void *& box, void *& place)
{
}

#ifdef MACHO_SNAPSHOTS
template<>
void *Macho::_cloneBox<_EmptyBox>(void *other)
{
    return &_EmptyBox::theEmptyBox;
}

#endif // ifdef MACHO_SNAPSHOTS

////////////////////////////////////////////////////////////////////////////////
// Implementation for Alias
void Alias::setState(_MachineBase& machine) const
{
    machine.setPendingState(key()->instanceGenerator(machine), myInitializer->clone());
}

////////////////////////////////////////////////////////////////////////////////
// Implementation for StateSpecification
_StateInstance& _StateSpecification::_getInstance(_MachineBase& machine)
{
    // Look first in machine for existing StateInstance.
    _StateInstance *& instance = machine.getInstance(0);
    if (!instance)
        instance = new _RootInstance(machine, nullptr);

    return *instance;
}

void _StateSpecification::_shutdown()
{
    _myStateInstance.machine().shutdown();
}

void _StateSpecification::_restore(_StateInstance& current)
{
    _myStateInstance.machine().myCurrentState = &current;
}

void _StateSpecification::setState(const Alias& state)
{
    state.setState(_myStateInstance.machine());
}

#ifdef MACHO_SNAPSHOTS
void _StateSpecification::setState(_StateInstance& current)
{
    _myStateInstance.machine().setPendingState(current, &_theDefaultInitializer);
}

#endif // ifdef MACHO_SNAPSHOTS

////////////////////////////////////////////////////////////////////////////////
// StateInstance implementation
_StateInstance::_StateInstance(_MachineBase& machine, _StateInstance *parent)
    : myMachine(machine)
    , mySpecification(nullptr)
    , myHistory(nullptr)
    , myParent(parent)
    , myBox(nullptr)
    , myBoxPlace(nullptr)
{
}

_StateInstance::~_StateInstance()
{
    if (myBoxPlace)
        ::operator delete(myBoxPlace);

    delete mySpecification;
}

void _StateInstance::entry(_StateInstance& previous, bool first)
{
    // Only Root has no parent
    if (!myParent)
        return;

    // first entry or previous state is not substate -> perform entry
    if (first || !previous.isChild(*this)) {
        myParent->entry(previous, false);

        createBox();

        MACHO_TRC2(name(), "Entry");
        mySpecification->entry();
    }
}

void _StateInstance::exit(_StateInstance& next)
{
    // Only Root has no parent
    if (!myParent)
        return;

    // self transition or next state is not substate -> perform exit
    if (this == &next || !next.isChild(*this)) {
        MACHO_TRC2(name(), "Exit");
        mySpecification->exit();

        // EmptyBox should be most common box, so optimize for this case.
        if (myBox != &_EmptyBox::theEmptyBox)
            mySpecification->_deleteBox(*this);

        myParent->exit(next);
    }
}

void _StateInstance::init(bool history)
{
    if (history && myHistory) {
        MACHO_TRC3(name(), "History transition to", myHistory->name());
        myMachine.setPendingState(*myHistory, &_theDefaultInitializer);
    }
    else {
        MACHO_TRC2(name(), "Init");
        mySpecification->init();
    }

    myHistory = nullptr;
}

#ifdef MACHO_SNAPSHOTS
void _StateInstance::copy(_StateInstance& original)
{
    if (original.myHistory) {
        _StateInstance *history = myMachine.getInstance(original.myHistory->id());
        assert(history);
        setHistory(history);
    }

    if (original.myBox)
        cloneBox(original.myBox);
}

_StateInstance *_StateInstance::clone(_MachineBase& newMachine)
{
    assert(!newMachine.getInstance(id()));

    _StateInstance *parent = 0;
    if (myParent)
        // Tell other machine to clone parent first.
        parent = newMachine.createClone(myParent->id(), myParent);

    _StateInstance *clone = create(newMachine, parent);
    return clone;
}

#endif // ifdef MACHO_SNAPSHOTS

////////////////////////////////////////////////////////////////////////////////
// Base class for Machine objects.
_MachineBase::_MachineBase()
{
}

_MachineBase::~_MachineBase()
{
    assert(!myPendingInit);

    delete[] myInstances;
    delete myPendingEvent;
}

Alias _MachineBase::currentState() const
{
    return Alias(myCurrentState->key());
}

void _MachineBase::setState(_StateInstance& instance, _Initializer *init)
{
    setPendingState(instance, init);
    rattleOn();
}

void _MachineBase::setState(const Alias& state)
{
    state.setState(*this);
    rattleOn();
}

void _MachineBase::start(_StateInstance& instance)
{
    MACHO_TRC1("Starting Machine");

    // Start with Root state
    myCurrentState = &_StateSpecification::_getInstance(*this);
    // Then go to state
    setState(instance, &_theDefaultInitializer);
}

void _MachineBase::start(const Alias& state)
{
    MACHO_TRC1("Starting Machine");

    // Start with Root state
    myCurrentState = &_StateSpecification::_getInstance(*this);
    // Then go to state
    setState(state);
}

void _MachineBase::shutdown()
{
    assert(!myPendingState);

    MACHO_TRC1("Shutting down Machine");

    // Performs exit actions by going to Root (=StateSpecification) state.
    setState(_StateSpecification::_getInstance(*this), &_theDefaultInitializer);

    myCurrentState = nullptr;
}

void _MachineBase::allocate(unsigned int count)
{
    myInstances = new _StateInstance *[count];
    for (unsigned int i = 0; i < count; ++i)
        myInstances[i] = nullptr;
}

void _MachineBase::free(unsigned int count)
{
    // Free from end of list, so that child states are freed first
    unsigned int i = count;
    while (i > 0) {
        --i;
        delete myInstances[i];
        myInstances[i] = nullptr;
    }
}

// Clear history of state and children.
void _MachineBase::clearHistoryDeep(unsigned int count, const _StateInstance& instance)
{
    for (unsigned int i = 0; i < count; ++i) {
        _StateInstance *s = myInstances[i];
        if (s && s->isChild(instance))
            s->setHistory(nullptr);
    }
}

#ifdef MACHO_SNAPSHOTS
void _MachineBase::copy(_StateInstance **others, unsigned int count)
{
    // Create StateInstance objects
    for (ID i = 0; i < count; ++i)
        createClone(i, others[i]);

    // Copy StateInstance object's state
    for (ID i = 0; i < count; ++i) {
        _StateInstance *state = myInstances[i];
        if (state) {
            assert(others[i]);
            state->copy(*others[i]);
        }
    }
}

_StateInstance *_MachineBase::createClone(ID id, _StateInstance *original)
{
    _StateInstance *& clone = getInstance(id);

    // Object already created?
    if (!clone && original)
        clone = original->clone(*this);

    return clone;
}

#endif // ifdef MACHO_SNAPSHOTS

// Performs a pending state transition.
void _MachineBase::rattleOn()
{
    assert(myCurrentState);

    while (myPendingState || myPendingEvent) {
        // Loop here because init actions might change state again.
        while (myPendingState) {
            MACHO_TRC3(myCurrentState->name(), "Transition to", myPendingState->name());

#ifndef NDEBUG
            // Entry/Exit actions may not dispatch events: set dummy event.
            if (!myPendingEvent)
                myPendingEvent = (_IEventBase *)&myPendingEvent;
#endif // ifndef NDEBUG

            // Perform exit actions (which exactly depends on new state).
            myCurrentState->exit(*myPendingState);

            // Store history information for previous state now.
            // Previous state will be used for deep history.
            myCurrentState->setHistorySuper(*myCurrentState);

            _StateInstance *previous = myCurrentState;
            myCurrentState = myPendingState;

            // Deprecated!
            if (myPendingBox) {
                myCurrentState->setBox(myPendingBox);
                myPendingBox = nullptr;
            }

            // Perform entry actions on next state's parents (which exactly depends on previous state).
            myCurrentState->entry(*previous);

            // State transition complete.
            // Clear 'pending' information just now so that setState would assert in exits and entries, but not in init.
            myPendingState = nullptr;

            // Use initializer to call proper "init" action.
            _Initializer *init = myPendingInit;
            myPendingInit = nullptr;

            init->execute(*myCurrentState);
            init->destroy();

            assert("Init may only transition to proper substates" &&
                    (!myPendingState ||
                     (myPendingState->isChild(*myCurrentState) && (myCurrentState != myPendingState)))
                    );

#ifndef NDEBUG
            // Clear dummy event if need be
            if (myPendingEvent == (_IEventBase *)&myPendingEvent)
                myPendingEvent = nullptr;
#endif // ifndef NDEBUG
        }    // while (myPendingState)

        if (myPendingEvent) {
            _IEventBase *event = myPendingEvent;
            myPendingEvent = nullptr;
            event->dispatch(*myCurrentState);
            delete event;
        }
    }    // while (myPendingState || myPendingEvent)
}    // rattleOn

////////////////////////////////////////////////////////////////////////////////
// Implementation for _AdaptingInitializer

Key _AdaptingInitializer::adapt(Key key)
{
    ID id = static_cast<_KeyData *>(key)->id;
    const _StateInstance *instance = myMachine.getInstance(id);
    _StateInstance *history = nullptr;

    if (instance)
        history = instance->history();

    return history ? history->key() : key;
}

////////////////////////////////////////////////////////////////////////////////
// Singleton initializers.
_DefaultInitializer _theDefaultInitializer;
_HistoryInitializer _theHistoryInitializer;

} // namespace inet

