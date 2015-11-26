//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_MAPPINGUTILS_H
#define __INET_MAPPINGUTILS_H

#include "inet/common/INETDefs.h"
#include "inet/common/mapping/MappingBase.h"

namespace inet {

namespace physicallayer {

class FilledUpMapping;

/**
 * @brief This iterator takes another ConstMappingIterator and does
 * just pipe every method to the passed ConstMappingIterator.
 *
 * This class is meant to be used as base class for Iterators which
 * want to change just several parts without having to implement and pipe every
 * other method of the ConstMappingIteratorInterface.
 *
 * Note: Does take ownership of the passed iterator pointer!
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
template<class Base>
class BaseFilteredIterator : public Base
{
  public:
    typedef typename Base::argument_value_t argument_value_t;
    typedef typename Base::argument_value_cref_t argument_value_cref_t;

  protected:
    Base *origIterator;

  private:
    /** @brief Copy constructor is not allowed.
     */
    BaseFilteredIterator(const BaseFilteredIterator&);
    /** @brief Assignment operator is not allowed.
     */
    BaseFilteredIterator& operator=(const BaseFilteredIterator&);

  public:
    BaseFilteredIterator(Base *orig)
        : Base()
        , origIterator(orig)
    {}

    virtual ~BaseFilteredIterator()
    {
        if (origIterator)
            delete origIterator;
    }

    virtual const Argument& getNextPosition() const { return origIterator->getNextPosition(); }

    virtual void jumpTo(const Argument& pos) { origIterator->jumpTo(pos); }

    virtual void jumpToBegin() { origIterator->jumpToBegin(); }

    virtual void iterateTo(const Argument& pos) { origIterator->iterateTo(pos); }

    virtual void next() { origIterator->next(); }

    virtual bool inRange() const { return origIterator->inRange(); }

    virtual bool hasNext() const { return origIterator->hasNext(); }

    virtual const Argument& getPosition() const { return origIterator->getPosition(); }

    virtual argument_value_t getValue() const { return origIterator->getValue(); }
};

/**
 * @brief Const version of the BaseFilteredIterator. Meant to be used for
 * ConstMappingIterator instances.
 *
 * @sa BaseFilteredIterator *
 * @author Karl Wessel
 * @ingroup mappingDetails
 * */
typedef BaseFilteredIterator<ConstMappingIterator> FilteredConstMappingIterator;

/**
 * @brief Non-Const version of the BaseFilteredIterator. Meant to be used for
 * MappingIterator instances.
 *
 * @sa BaseFilteredIterator
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
class INET_API FilteredMappingIterator : public BaseFilteredIterator<MappingIterator>
{
  public:
    FilteredMappingIterator(MappingIterator *orig) :
        BaseFilteredIterator<MappingIterator>(orig) {}

    virtual ~FilteredMappingIterator() {}

    virtual void setValue(argument_value_cref_t value) override { origIterator->setValue(value); }
};

/**
 * @brief Provides an implementation of the MappingIterator-
 * Interface which is able to iterate over TimeMappings.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
template<template<typename> class Interpolator>
class TimeMappingIterator : public MappingIterator
{
  protected:
    /** @brief The templated InterpolateableMap the underlying Mapping uses std::map as storage type.*/
    typedef InterpolateableMap<Interpolator<std::map<simtime_t, argument_value_t> > >
        interpolator_map_type;
    typedef typename interpolator_map_type::interpolator_type interpolator_type;
    typedef typename interpolator_map_type::mapped_type mapped_type;
    typedef typename interpolator_map_type::iterator_intpl iterator;
    typedef typename interpolator_map_type::const_iterator_intpl const_iterator;

    /** @brief Stores the current position iterator inside the Mapping.*/
    iterator valueIt;

    /** @brief Stores the current position of the iterator.*/
    Argument position;

    /** @brief Stores the next position a call of "next()" would jump to.*/
    Argument nextPosition;

    /**
     * @brief Stores if this mapping represents a step function.
     *
     * Assures that the steps are considered when iterating the mapping
     * by adding a second key-entry as short as possible before every
     * key entry set by the user. The additional key-entry defines the
     * value the mapping has just before the key entry the user added.
     */
    bool isStepMapping;

    bool atPreStep;

  protected:
    void updateNextPos()
    {
        simtime_t t = valueIt.getNextPosition();
        if (isStepMapping && !atPreStep) {
            t.setRaw(t.raw() - 1);
        }
        nextPosition.setTime(t);
    }

  public:

    /**
     * @brief Initializes the Iterator to use the passed InterpolateableMapIterator.
     */
    TimeMappingIterator(const iterator& it)
        : MappingIterator()
        , valueIt(it)
        , position()
        , nextPosition()
        , isStepMapping(it.getInterpolator().isStepping())
        , atPreStep(false)
    {
        interpolator_type UsedInterpolator;

        isStepMapping = UsedInterpolator.isStepping();
        position.setTime(valueIt.getPosition());
        updateNextPos();
    }

    TimeMappingIterator(const TimeMappingIterator<Interpolator>& o)
        : MappingIterator(o)
        , valueIt(o.valueIt)
        , position(o.position)
        , nextPosition(o.nextPosition)
        , isStepMapping(o.isStepMapping)
        , atPreStep(o.atPreStep)
    {}

    virtual ~TimeMappingIterator() {}

    /**
     * @brief Lets the iterator point to the passed position.
     *
     * The passed new position can be at arbitrary places.
     *
     * This method has logarithmic complexity.
     */
    void jumpTo(const Argument& pos) override
    {
        atPreStep = false;
        valueIt.jumpTo(pos.getTime());
        position.setTime(pos.getTime());
        nextPosition.setTime(valueIt.getNextPosition());
        updateNextPos();
    }

    /**
     * @brief Iterates to the specified position. This method
     * should be used if the new position is near the current position.
     *
     * The passed position should compared bigger than the current position.
     *
     * This method has linear complexity over the number of key-entries
     * between the current position and the passed position. So if the
     * passed position is near the current position the complexity is
     * nearly constant.
     */
    void iterateTo(const Argument& pos) override
    {
        atPreStep = false;
        valueIt.iterateTo(pos.getTime());
        position.setTime(pos.getTime());
        nextPosition.setTime(valueIt.getNextPosition());
        updateNextPos();
    }

    /**
     * @brief Iterates to the next position of the function.
     *
     * The next position is the next bigger key entry of the
     * InterpoalteableMap.
     *
     * This method has constant complexity.
     */
    virtual void next() override
    {
        if (isStepMapping && !atPreStep) {
            valueIt.iterateTo(nextPosition.getTime());
            atPreStep = true;
        }
        else {
            valueIt.next();
            atPreStep = false;
        }
        position.setTime(valueIt.getPosition());
        updateNextPos();
    }

    /**
     * @brief Returns true if the current position of the iterator
     * is in range of the function.
     *
     * This method should be used as end-condition when iterating
     * over the function with the "next()" method.
     *
     * THis method has constant complexity.
     */
    virtual bool inRange() const override
    {
        return valueIt.inRange();
    }

    /**
     * @brief Returns the current position of the iterator.
     *
     * This method has constant complexity.
     */
    virtual const Argument& getPosition() const override
    {
        return position;
    }

    /**
     * @brief Returns the next position a call to "next()" would jump to.
     *
     * This method has constant complexity.
     */
    virtual const Argument& getNextPosition() const override
    {
        return nextPosition;
    }

    /**
     * @brief Returns the value of the function at the current
     * position.
     *
     * This method has constant complexity.
     */
    virtual mapped_type getValue() const override
    {
        return *valueIt.getValue();
    }

    /**
     * @brief Lets the iterator point to the begin of the mapping.
     *
     * The beginning of the mapping is the smallest key entry in the
     * InterpolateableMap.
     *
     * Constant complexity.
     */
    virtual void jumpToBegin() override
    {
        valueIt.jumpToBegin();
        position.setTime(valueIt.getPosition());
    }

    /**
     * @brief Returns true if the iterator has a next value
     * inside its range a call to "next()" can jump to.
     *
     * Constant complexity.
     */
    virtual bool hasNext() const override
    {
        return valueIt.hasNext();
    }

    /**
     * @brief Changes the value of the function at the current
     * position.
     *
     * This method has constant complexity.
     */
    virtual void setValue(argument_value_cref_t value) override
    {
        valueIt.setValue(value);
    }
};

/**
 * @brief Implements the Mapping-interface with an InterpolateableMap from
 * simtime_t to double between which values can be interpolated to represent
 * a Mapping with only time as domain.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
template<template<typename> class Interpolator>
class TimeMapping : public Mapping
{
  protected:
    /** @brief The templated InterpolateableMap the underlying Mapping uses std::map as storage type.*/
    typedef InterpolateableMap<Interpolator<std::map<simtime_t, argument_value_t> > >
        interpolator_map_type;
    typedef typename interpolator_map_type::interpolator_type interpolator_type;
    typedef typename interpolator_map_type::mapped_type mapped_type;
    typedef typename interpolator_map_type::mapped_cref_type mapped_cref_type;
    typedef typename interpolator_map_type::iterator_intpl iterator;
    typedef typename interpolator_map_type::const_iterator_intpl const_iterator;

    /** @brief Stores the key-entries defining the function.*/
    interpolator_map_type entries;

  public:

    /**
     * @brief Initializes the Mapping with the passed Interpolation method.
     */
    TimeMapping() :
        Mapping(), entries() {}
    TimeMapping(const TimeMapping<Interpolator>& o) :
        Mapping(o), entries(o.entries) {}

    /**
     * @brief Initializes the Mapping with the passed Interpolation method.
     */
    TimeMapping(mapped_cref_type outOfRangeVal) :
        Mapping(), entries(outOfRangeVal) {}

    virtual ~TimeMapping() {}
    /**
     * @brief returns a deep copy of this mapping instance.
     */
    virtual Mapping *clone() const override { return new TimeMapping<Interpolator>(*this); }

    /**
     * @brief Returns the value of this Function at the position specified
     * by the passed Argument.
     *
     * This method has logarithmic complexity.
     */
    virtual argument_value_t getValue(const Argument& pos) const override
    {
        return *entries.getIntplValue(pos.getTime());
    }

    /**
     * @brief Changes the value of the function at the specified
     * position.
     *
     * This method has logarithmic complexity.
     */
    virtual void setValue(const Argument& pos, argument_value_cref_t value) override
    {
        entries[pos.getTime()] = value;
    }

    /**
     * @brief Returns a pointer of a new Iterator which is able to iterate
     * over the function and can change the value the iterator points to.
     *
     * Note: The caller of this method has to delete the returned Iterator
     * pointer if it isn't used anymore.
     */
    virtual MappingIterator *createIterator() override
    {
        return new TimeMappingIterator<Interpolator>(entries.beginIntpl());
    }

    /**
     * @brief Returns a pointer of a new Iterator which is able to iterate
     * over the function and can change the value the iterator points to.
     *
     * Note: The caller of this method has to delete the returned Iterator
     * pointer if it isn't used anymore.
     */
    virtual MappingIterator *createIterator(const Argument& pos) override
    {
        return new TimeMappingIterator<Interpolator>(entries.findIntpl(pos.getTime()));
    }
};

/**
 * @brief Provides an implementation of the MappingIterator-
 * Interface which is able to iterate over FrequencyMappings.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
template<template<typename> class Interpolator>
class FrequencyMappingIterator : public MappingIterator
{
  protected:
    /** @brief The templated InterpolateableMap the underlying Mapping uses std::map as storage type.*/
    typedef InterpolateableMap<Interpolator<std::map<argument_value_t, argument_value_t> > >
        interpolator_map_type;
    typedef typename interpolator_map_type::interpolator_type interpolator_type;
    typedef typename interpolator_map_type::mapped_type mapped_type;
    typedef typename interpolator_map_type::iterator_intpl iterator;
    typedef typename interpolator_map_type::const_iterator_intpl const_iterator;

    /** @brief Stores the current position iterator inside the Mapping.*/
    iterator valueIt;

    /** @brief Stores the current position of the iterator.*/
    Argument position;

    /** @brief Stores the next position a call of "next()" would jump to.*/
    Argument nextPosition;

    /**
     * @brief Stores if this mapping represents a step function.
     *
     * Assures that the steps are considered when iterating the mapping
     * by adding a second key-entry as short as possible before every
     * key entry set by the user. The additional key-entry defines the
     * value the mapping has just before the key entry the user added.
     */
    bool isStepMapping;

    bool atPreStep;

  protected:
    void updateNextPos()
    {
        argument_value_t value = valueIt.getNextPosition();
        if (isStepMapping && !atPreStep) {
            value = nexttoward(value, 0);
        }
        nextPosition.setArgValue(Dimension::frequency, value);
    }

  public:

    /**
     * @brief Initializes the Iterator to use the passed InterpolateableMapIterator.
     */
    FrequencyMappingIterator(const iterator& it)
        : MappingIterator()
        , valueIt(it)
        , position()
        , nextPosition()
        , isStepMapping(it.getInterpolator().isStepping())
        , atPreStep(false)
    {
        interpolator_type UsedInterpolator;

        isStepMapping = UsedInterpolator.isStepping();
        position.setArgValue(Dimension::frequency, valueIt.getPosition());
        updateNextPos();
    }

    FrequencyMappingIterator(const FrequencyMappingIterator<Interpolator>& o)
        : MappingIterator(o)
        , valueIt(o.valueIt)
        , position(o.position)
        , nextPosition(o.nextPosition)
        , isStepMapping(o.isStepMapping)
        , atPreStep(o.atPreStep)
    {}

    virtual ~FrequencyMappingIterator() {}

    /**
     * @brief Lets the iterator point to the passed position.
     *
     * The passed new position can be at arbitrary places.
     *
     * This method has logarithmic complexity.
     */
    void jumpTo(const Argument& pos) override
    {
        atPreStep = false;
        valueIt.jumpTo(pos.getArgValue(Dimension::frequency));
        position.setArgValue(Dimension::frequency, pos.getArgValue(Dimension::frequency));
        nextPosition.setArgValue(Dimension::frequency, valueIt.getNextPosition());
        updateNextPos();
    }

    /**
     * @brief Iterates to the specified position. This method
     * should be used if the new position is near the current position.
     *
     * The passed position should compared bigger than the current position.
     *
     * This method has linear complexity over the number of key-entries
     * between the current position and the passed position. So if the
     * passed position is near the current position the complexity is
     * nearly constant.
     */
    void iterateTo(const Argument& pos) override
    {
        atPreStep = false;
        valueIt.iterateTo(pos.getArgValue(Dimension::frequency));
        position.setArgValue(Dimension::frequency, pos.getArgValue(Dimension::frequency));
        nextPosition.setArgValue(Dimension::frequency, valueIt.getNextPosition());
        updateNextPos();
    }

    /**
     * @brief Iterates to the next position of the function.
     *
     * The next position is the next bigger key entry of the
     * InterpoalteableMap.
     *
     * This method has constant complexity.
     */
    virtual void next() override
    {
        if (isStepMapping && !atPreStep) {
            valueIt.iterateTo(nextPosition.getArgValue(Dimension::frequency));
            atPreStep = true;
        }
        else {
            valueIt.next();
            atPreStep = false;
        }
        position.setArgValue(Dimension::frequency, valueIt.getPosition());
        updateNextPos();
    }

    /**
     * @brief Returns true if the current position of the iterator
     * is in range of the function.
     *
     * This method should be used as end-condition when iterating
     * over the function with the "next()" method.
     *
     * THis method has constant complexity.
     */
    virtual bool inRange() const override
    {
        return valueIt.inRange();
    }

    /**
     * @brief Returns the current position of the iterator.
     *
     * This method has constant complexity.
     */
    virtual const Argument& getPosition() const override
    {
        return position;
    }

    /**
     * @brief Returns the next position a call to "next()" would jump to.
     *
     * This method has constant complexity.
     */
    virtual const Argument& getNextPosition() const override
    {
        return nextPosition;
    }

    /**
     * @brief Returns the value of the function at the current
     * position.
     *
     * This method has constant complexity.
     */
    virtual mapped_type getValue() const override
    {
        return *valueIt.getValue();
    }

    /**
     * @brief Lets the iterator point to the begin of the mapping.
     *
     * The beginning of the mapping is the smallest key entry in the
     * InterpolateableMap.
     *
     * Constant complexity.
     */
    virtual void jumpToBegin() override
    {
        valueIt.jumpToBegin();
        position.setArgValue(Dimension::frequency, valueIt.getPosition());
    }

    /**
     * @brief Returns true if the iterator has a next value
     * inside its range a call to "next()" can jump to.
     *
     * Constant complexity.
     */
    virtual bool hasNext() const override
    {
        return valueIt.hasNext();
    }

    /**
     * @brief Changes the value of the function at the current
     * position.
     *
     * This method has constant complexity.
     */
    virtual void setValue(argument_value_cref_t value) override
    {
        valueIt.setValue(value);
    }
};

/**
 * @brief Implements the Mapping-interface with an InterpolateableMap from
 * double to double between which values can be interpolated to represent
 * a Mapping with only frequency as domain.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
template<template<typename> class Interpolator>
class FrequencyMapping : public Mapping
{
  protected:
    /** @brief The templated InterpolateableMap the underlying Mapping uses std::map as storage type.*/
    typedef InterpolateableMap<Interpolator<std::map<argument_value_t, argument_value_t> > >
        interpolator_map_type;
    typedef typename interpolator_map_type::interpolator_type interpolator_type;
    typedef typename interpolator_map_type::mapped_type mapped_type;
    typedef typename interpolator_map_type::mapped_cref_type mapped_cref_type;
    typedef typename interpolator_map_type::iterator_intpl iterator;
    typedef typename interpolator_map_type::const_iterator_intpl const_iterator;

    /** @brief Stores the key-entries defining the function.*/
    interpolator_map_type entries;

  public:

    /**
     * @brief Initializes the Mapping with the passed Interpolation method.
     */
    FrequencyMapping() :
        Mapping(DimensionSet::freqDomain), entries() {}
    FrequencyMapping(const FrequencyMapping<Interpolator>& o) :
        Mapping(o), entries(o.entries) {}

    /**
     * @brief Initializes the Mapping with the passed Interpolation method.
     */
    FrequencyMapping(mapped_cref_type outOfRangeVal) :
        Mapping(DimensionSet::freqDomain), entries(outOfRangeVal) {}

    virtual ~FrequencyMapping() {}
    /**
     * @brief returns a deep copy of this mapping instance.
     */
    virtual Mapping *clone() const override { return new FrequencyMapping<Interpolator>(*this); }

    /**
     * @brief Returns the value of this Function at the position specified
     * by the passed Argument.
     *
     * This method has logarithmic complexity.
     */
    virtual argument_value_t getValue(const Argument& pos) const override
    {
        return *entries.getIntplValue(pos.getArgValue(Dimension::frequency));
    }

    /**
     * @brief Changes the value of the function at the specified
     * position.
     *
     * This method has logarithmic complexity.
     */
    virtual void setValue(const Argument& pos, argument_value_cref_t value) override
    {
        entries[pos.getArgValue(Dimension::frequency)] = value;
    }

    /**
     * @brief Returns a pointer of a new Iterator which is able to iterate
     * over the function and can change the value the iterator points to.
     *
     * Note: The caller of this method has to delete the returned Iterator
     * pointer if it isn't used anymore.
     */
    virtual MappingIterator *createIterator() override
    {
        return new FrequencyMappingIterator<Interpolator>(entries.beginIntpl());
    }

    /**
     * @brief Returns a pointer of a new Iterator which is able to iterate
     * over the function and can change the value the iterator points to.
     *
     * Note: The caller of this method has to delete the returned Iterator
     * pointer if it isn't used anymore.
     */
    virtual MappingIterator *createIterator(const Argument& pos) override
    {
        return new FrequencyMappingIterator<Interpolator>(entries.findIntpl(pos.getArgValue(Dimension::frequency)));
    }
};

/**
 * @brief Helper-class for the MultiDimMapping which provides an Iterator
 * which linear interpolates between two other Mapping iterators. Or in
 * other words, it provides an Iterator for an linear interpolated Mapping.
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
class INET_API LinearIntplMappingIterator : public MappingIterator
{
  protected:
    /** @brief Iterator for the left Mapping to interpolate.*/
    ConstMappingIterator *leftIt;
    /** @brief Iterator for the right Mapping to interpolate.*/
    ConstMappingIterator *rightIt;

    /** @brief The factor defining how strong the left and the right Mapping
     * affect the interpolation.*/
    argument_value_t factor;

  private:
    /** @brief Copy constructor is not allowed.
     */
    LinearIntplMappingIterator(const LinearIntplMappingIterator&);
    /** @brief Assignment operator is not allowed.
     */
    LinearIntplMappingIterator& operator=(const LinearIntplMappingIterator&);

  public:
    /**
     * @brief Initializes the Iterator with the passed Iterators of the mappings to
     * Interpolate and the their interpolation-factor.
     */
    LinearIntplMappingIterator(ConstMappingIterator *leftIt, ConstMappingIterator *rightIt, argument_value_cref_t f);

    /**
     * @brief Deletes the left and the right mapping iterator.
     */
    virtual ~LinearIntplMappingIterator();

    /**
     * @brief An interpolated mapping isn't really iterateable over specific
     * values, it only provides an fast way to get several values from an
     * Interpolated mapping.
     */
    virtual bool hasNext() const override { return false; }
    /**
     * @brief An interpolated mapping isn't really iterateable over specific
     * values, it only provides an fast way to get several values from an
     * Interpolated mapping.
     */
    virtual bool inRange() const override { return false; }

    /**
     * @brief This method isn't supported by an interpolated Mapping.
     */
    virtual void jumpToBegin() override { assert(false); }
    /**
     * @brief This method isn't supported by an interpolated Mapping.
     */
    virtual void next() override { assert(false); }

    /**
     * @brief This method isn't supported by an interpolated Mapping.
     */
    virtual void setValue(argument_value_cref_t) override { assert(false); }

    /**
     * @brief Lets the iterator point to the passed position.
     *
     * This method has logarithmic complexity over both of the
     * underlying Mappings used to interpolate.
     */
    virtual void jumpTo(const Argument& pos) override
    {
        leftIt->jumpTo(pos);
        rightIt->jumpTo(pos);
    }

    /**
     * @brief Increases the iterator to the passed position. This
     * position should be near the current position of the iterator.
     *
     * The passed position has to be compared bigger than the current position.
     *
     * This method has linear complexity over the number of key entries between
     * the current and the passed position in both underlying mappings used to
     * interpolate. So if the passed position is near the current position this
     * method has nearly constant complexity.
     */
    virtual void iterateTo(const Argument& pos) override
    {
        leftIt->iterateTo(pos);
        rightIt->iterateTo(pos);
    }

    /**
     * @brief Returns the value of the Interpolated mapping at the current
     * position of the iterator.
     *
     * This method has constant complexity.
     */
    virtual argument_value_t getValue() const override
    {
        argument_value_cref_t v0 = leftIt->getValue();
        argument_value_cref_t v1 = rightIt->getValue();
        return v0 + (v1 - v0) * factor;
    }

    /**
     * @brief Returns the current position of the iterator.
     *
     * Constant complexity.
     */
    virtual const Argument& getPosition() const override
    {
        return leftIt->getPosition();
    }

    /**
     * @brief This method isn't supported by an interpolated mapping.
     */
    virtual const Argument& getNextPosition() const override { assert(false); return *((Argument *)nullptr); }
};

/**
 * @brief Helper class which represents a linear interpolation between
 * two other mappings.
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
class INET_API LinearIntplMapping : public Mapping
{
  protected:
    /** @brief The left mapping to interpolate.*/
    const ConstMapping *left;
    /** @brief The right mapping to interpolate*/
    const ConstMapping *right;

    /** @brief The interpolation factor determining the linear interpolation
     * between left and right mapping.*/
    argument_value_t factor;

  public:
    LinearIntplMapping(const LinearIntplMapping& o)
        : Mapping(o), left(o.left), right(o.right), factor(o.factor) {}

    LinearIntplMapping& operator=(const LinearIntplMapping& copy)
    {
        LinearIntplMapping tmp(copy);    // All resource all allocation happens here.
                                         // If this fails the copy will throw an exception
                                         // and 'this' object is unaffected by the exception.
        swap(tmp);
        return *this;
    }

    // swap is usually trivial to implement
    // and you should easily be able to provide the no-throw guarantee.
    void swap(LinearIntplMapping& s)
    {
        Mapping::swap(s);    // swap the base class members
        /* Swap all D members */
        std::swap(left, s.left);
        std::swap(right, s.right);
        std::swap(factor, s.factor);
    }

  public:

    /**
     * @brief Initializes the LinearIntplMapping with the passed left and right
     * Mapping to interpolate by the passed interpolation value.
     */
    LinearIntplMapping(const ConstMapping *const left = nullptr, const ConstMapping *const right = nullptr, argument_value_cref_t f = Argument::MappedZero) :
        Mapping(), left(left), right(right), factor(f) {}

    virtual ~LinearIntplMapping() {}

    /**
     * @brief Interpolated mappings are not supposed to be cloned!
     */
    virtual Mapping *clone() const override { assert(false); return 0; }

    /**
     * @brief Returns the linear interpolated value of this Mapping.
     *
     * The value is calculated by the following formula:
     *
     * v = left + (right - left) * intplFactor
     */
    virtual argument_value_t getValue(const Argument& pos) const override
    {
        assert(left);
        assert(right);

        argument_value_cref_t v0 = left->getValue(pos);
        argument_value_cref_t v1 = right->getValue(pos);
        //return v0 + (v1 - v0) * factor;
        return v0 * (Argument::MappedOne - factor) + v1 * factor;
    }

    /**
     * @brief An interpolated mapping doesn't have a valid "first"-entry,
     * so this method is not supported.
     */
    virtual MappingIterator *createIterator() override
    {
        assert(false);
        return 0;
    }

    /**
     * @brief Creates an iterator for this mapping starting at
     * the passed position.
     *
     * Note: The returned Iterator does only support a subset of the
     * normal MappingIterator-methods. See LinearIntplMappingIterator for
     * details.
     */
    virtual MappingIterator *createIterator(const Argument& pos) override
    {
        assert(left);
        assert(right);

        return new LinearIntplMappingIterator(left->createConstIterator(pos), right->createConstIterator(pos), factor);
    }

    /**
     * @brief This method is not supported!
     */
    virtual void setValue(const Argument&, argument_value_cref_t) override { assert(false); }
};

/**
 * @brief Helper class (-specialization) for multiDimMapping which is used by an
 * InterpolateableMap as return value of the "getValue()" - method.
 *
 * Provides either an pointer to an actual SubMapping of the MultiDimMapping or
 * a Pointer to an temporary InterpolatedMapping between two Sub-mappings of the
 * MultiDimMapping.
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
template<>
class Interpolated<Mapping *>
{
  protected:
    typedef Mapping *value_type;
    typedef const value_type& value_cref_type;
    typedef value_type& value_ref_type;
    typedef value_type *value_ptr_type;
    typedef Interpolated<value_type> _Self;

    /** @brief Holds the temporary InterpolatedMapping if necessary.*/
    LinearIntplMapping mapping;

    /** @brief A pointer to the Mapping this class represents.*/
    value_type value;

    /** @brief Stores if we use the temporary IntplMapping or a external pointer.*/
    bool isPointer;

  public:
    /** @brief Stores if the underlying Mapping is interpolated or not.*/
    bool isInterpolated;

  public:
    /**
     * @brief Initializes this Interpolated instance to represent the passed
     * Interpolated Mapping. Copies the passed Mapping to its internal member.
     * Sets "isInterpolated" to true.
     */
    Interpolated(const LinearIntplMapping& m) :
        mapping(m), value(), isPointer(false), isInterpolated(true)
    {
        value = &mapping;
    }

    /**
     * @brief Initializes this Interpolated instance to represent the Mapping
     * the passed pointer points to and with the passed isIntpl value.
     *
     * The passed pointer has to be valid as long as this instance exists.
     */
    Interpolated(value_type v, bool isIntpl = true) :
        mapping(), value(v), isPointer(true), isInterpolated(isIntpl) {}

    /**
     * @brief Copy-constructor which assures that the internal storage is used correctly.
     */
    Interpolated(const _Self& o) :
        mapping(), value(o.value), isPointer(o.isPointer), isInterpolated(o.isInterpolated)
    {
        if (!isPointer) {
            mapping = o.mapping;
            value = &mapping;
        }
    }

    ~Interpolated() {}

    /**
     * @brief Assignment operator which assures that the internal storage is copied
     * correctly.
     */
    const _Self& operator=(const _Self& o)
    {
        isInterpolated = o.isInterpolated;
        isPointer = o.isPointer;
        value = o.value;
        if (!isPointer) {
            mapping = o.mapping;
            value = &mapping;
        }
        return *this;
    }

    /**
     * @brief Dereferences this Interpolated to the represented value (works like
     * dereferencing an std::iterator).
     */
    value_ref_type operator*()
    {
        return value;
    }

    /**
     * @brief Dereferences this Interpolated to the represented value (works like
     * dereferencing an std::iterator).
     */
    value_ptr_type operator->()
    {
        return &value;
    }

    /**
     * @brief Two Interpolated<Mapping*> are compared equal if the pointer to
     * the represented Mapping is the same as well as the "isInterpolated"
     * value.
     */
    bool operator==(const _Self& other) const
    {
        return value == other.value && isInterpolated == other.isInterpolated;
    }

    /**
     * @brief Two Interpolated<Mapping*> are compared non equal if the pointer to
     * the represented Mapping differs or the "isInterpolated"
     * value.
     */
    bool operator!=(const _Self& other) const
    {
        return value != other.value || isInterpolated != other.isInterpolated;
    }
};

/**
 * @brief Specialization of the Linear-template which provides LinearInterpolation
 * for pointer two Mappings. Used by MultiDimMapping.
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
template<>
class Linear<std::map<Argument::mapped_type, Mapping *> > : public InterpolatorBase<std::map<Argument::mapped_type, Mapping *> >
{
  protected:
    typedef InterpolatorBase<std::map<Argument::mapped_type, Mapping *> > base_class_type;
    typedef Linear<std::map<Argument::mapped_type, Mapping *> > _Self;

  public:
    typedef base_class_type::storage_type storage_type;
    typedef base_class_type::container_type container_type;
    typedef base_class_type::key_type key_type;
    typedef base_class_type::key_cref_type key_cref_type;
    typedef base_class_type::mapped_type mapped_type;
    typedef base_class_type::mapped_cref_type mapped_cref_type;
    typedef base_class_type::pair_type pair_type;
    typedef base_class_type::iterator iterator;
    typedef base_class_type::const_iterator const_iterator;
    typedef base_class_type::comparator_type comparator_type;
    typedef base_class_type::interpolated interpolated;

  public:
    Linear() :
        base_class_type() {}

    Linear(mapped_cref_type oorv) :
        base_class_type(oorv) {}

    Linear(const _Self& o) :
        base_class_type(o) {}

    virtual ~Linear() {}

    /**
     * @brief Functor operator of this class which linear interpolates the value
     * at the passed position using the values between the passed Iterators.
     *
     * The upperBound-iterator has to point two the entry next bigger as the
     * passed position to interpolate.
     *
     * The returned instance of interpolated represents the result. Which can be
     * either an actual entry of the interpolated map (if the position two
     * interpolate was exactly that. Or it can be an interpolated value, if the
     * passed position was between two entries of the map.
     * This state can be retrieved with the "isInterpolated"-Member of the returned
     * "interpolated".
     */
    virtual
    interpolated operator()(const const_iterator& first,
            const const_iterator& last,
            key_cref_type pos,
            const_iterator upperBound) const override
    {
        if (first == last) {
            return base_class_type::outOfRangeVal;
        }
        if (upperBound == first) {
            return asInterpolated(upperBound->second, true);
        }

        const_iterator right = upperBound;
        const_iterator left = --upperBound;

        if (left->first == pos)
            return asInterpolated(left->second, false, false);

        if (right == last) {
            return asInterpolated(left->second, true);
        }

        return interpolated(LinearIntplMapping(left->second, right->second, linearInterpolationFactor(pos, left->first, right->first)));
    }

  protected:
    /**
     * @brief calculates the linear interpolation factor used for the created
     * LinearIntplMappings.
     */
    static ConstMapping::argument_value_t linearInterpolationFactor(key_cref_type t, key_cref_type t0, key_cref_type t1)
    {
        assert((t0 <= t && t <= t1) || (t0 >= t && t >= t1));
        if (t0 == t1) {
            return 0;
        }
        return cast_it<ConstMapping::argument_value_t>((t - t0) / (t1 - t0));
    }
};

/**
 * @brief Represents a constant mathematical mapping (f(x) = c)
 *
 * Returns the same value for every point in any dimension.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
class INET_API ConstantSimpleConstMapping : public SimpleConstMapping
{
  protected:
    argument_value_t value;

  public:
    ConstantSimpleConstMapping(const DimensionSet& dims, argument_value_cref_t val) :
        SimpleConstMapping(dims), value(val) {}

    ConstantSimpleConstMapping(const DimensionSet& dims,
            const Argument& key,
            argument_value_cref_t val) :
        SimpleConstMapping(dims, key), value(val) {}

    ConstantSimpleConstMapping(const ConstantSimpleConstMapping& o) :
        SimpleConstMapping(o), value(o.value) {}

    virtual ~ConstantSimpleConstMapping() {}

    virtual argument_value_t getValue(const Argument&) const override
    {
        return value;
    }

    /**
     * @brief Returns the value of this constant mapping.
     */
    argument_value_cref_t getValue() const
    {
        return value;
    }

    /**
     * @brief Sets the value of this constant mapping.
     */
    void setValue(argument_value_cref_t val) { value = val; }

    ConstMapping *constClone() const override
    {
        return new ConstantSimpleConstMapping(dimensions, value);
    }
};

/**
 * @brief Wraps an ConstMappingIterator into a MappingIterator
 * interface.
 *
 * Assumes that "setValue()" of the MappingIterator interface will
 * never be called.
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
class INET_API ConstMappingIteratorWrapper : public MappingIterator
{
  protected:
    ConstMappingIterator *const iterator;

  private:
    /** @brief Copy constructor is not allowed.
     */
    ConstMappingIteratorWrapper(const ConstMappingIteratorWrapper&);
    /** @brief Assignment operator is not allowed.
     */
    ConstMappingIteratorWrapper& operator=(const ConstMappingIteratorWrapper&);

  public:
    ConstMappingIteratorWrapper(ConstMappingIterator *it) :
        MappingIterator(), iterator(it) {}

    virtual ~ConstMappingIteratorWrapper()
    {
        if (iterator)
            delete iterator;
    }

    virtual void setValue(argument_value_cref_t) override { assert(false); }

    virtual const Argument& getNextPosition() const override { return iterator->getNextPosition(); }

    virtual void jumpTo(const Argument& pos) override { iterator->jumpTo(pos); }

    virtual void jumpToBegin() override { iterator->jumpToBegin(); }

    virtual void iterateTo(const Argument& pos) override { iterator->iterateTo(pos); }

    virtual void next() override { iterator->next(); }

    virtual bool inRange() const override { return iterator->inRange(); }

    virtual bool hasNext() const override { return iterator->hasNext(); }

    virtual const Argument& getPosition() const override { return iterator->getPosition(); }

    virtual argument_value_t getValue() const override { return iterator->getValue(); }
};

/**
 * @brief Wraps an ConstMapping into a Mapping interface.
 *
 * Assumes that "setValue()" of the Mapping interface will
 * never be called.
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
class INET_API ConstMappingWrapper : public Mapping
{
  protected:
    const ConstMapping *mapping;

  private:
    /** @brief Assignment operator is not allowed.
     */
    ConstMappingWrapper& operator=(const ConstMappingWrapper&);

  public:
    ConstMappingWrapper(const ConstMapping *m) :
        Mapping(m->getDimensionSet()), mapping(m) {}
    ConstMappingWrapper(const ConstMappingWrapper& o) :
        Mapping(o), mapping(o.mapping) {}

    virtual ~ConstMappingWrapper() {}

    virtual void setValue(const Argument&, argument_value_cref_t) override { assert(false); }

    virtual MappingIterator *createIterator() override
    {
        return new ConstMappingIteratorWrapper(mapping->createConstIterator());
    }

    virtual MappingIterator *createIterator(const Argument& pos) override
    {
        return new ConstMappingIteratorWrapper(mapping->createConstIterator(pos));
    }

    virtual argument_value_t getValue(const Argument& pos) const override { return mapping->getValue(pos); }

    virtual ConstMappingIterator *createConstIterator() const override
    {
        return mapping->createConstIterator();
    }

    virtual ConstMappingIterator *createConstIterator(const Argument& pos) const override
    {
        return mapping->createConstIterator(pos);
    }

    virtual ConstMapping *constClone() const override { return mapping->constClone(); }

    virtual Mapping *clone() const override
    {
        return new ConstMappingWrapper(mapping->constClone());
    }
};

template<template<typename> class Interpolator>
class MultiDimMapping;

/**
 * @brief Implementation of the MappingIterator-interface which is able
 * to iterate over every value in a MultiDimMapping.
 *
 * As the MultiDimMapping has a tree-like structure of sub-mappings to
 * represent multiple dimensions, the MultiDimIterator consist of a
 * number of sub-MultiDimIterator to represent the current position
 * inside the sub-mappings. So every sub-mapping-iterator represents
 * one dimension and the and Iterator to next Dimensions.
 * The last iterator is an TimeMappingIterator.
 *
 * Iteration works by sub-iterator-first-iteration. Which means that
 * at first the sub-iterator at the current position is iterated to its
 * end before the position inside the dimension of this iterator is increased.
 * This assures the iteration order demanded by the MappingIterator-interface.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
template<template<typename> class Interpolator>
class MultiDimMappingIterator : public MappingIterator
{
  protected:
    /** @brief The templated InterpolateableMap the underlying Mapping uses std::map as storage type.*/
    typedef InterpolateableMap<Interpolator<std::map<argument_value_t, Mapping *> > >
        interpolator_map_type;

    typedef typename interpolator_map_type::interpolated interpolated;
    typedef typename interpolator_map_type::iterator_intpl iterator;
    typedef typename interpolator_map_type::const_iterator_intpl const_iterator;

    /** @brief The MultiDimmapping to iterate over.*/
    const MultiDimMapping<Interpolator>& mapping;

    /** @brief Iterator storing the current position inside the underlying Mappings
     * sub-mapping map.*/
    iterator valueIt;

    /** @brief The sub-mapping of the sub-mapping map at the current position.*/
    interpolated subMapping;

    /** @brief An iterator for the sub-mapping which points two the current position
     * in the next dimensions.*/
    MappingIterator *subIterator;

    /** @brief The current position in every Dimension of this Iterator.*/
    Argument position;

    /** @brief The position a call to "next()" would jump to.*/
    Argument nextPosition;

  protected:

    /**
     * @brief Helper method which updates the sub-iterator for the passed
     * position.
     *
     * Called when the position of of the iterator inside the
     * dimension this Iterator represents has changed.
     */
    void updateSubIterator(const Argument& pos)
    {
        interpolated subM = valueIt.getValue();
        if (subM != subMapping) {
            if (subIterator)
                delete subIterator;

            subMapping = subM;
            if (*subMapping)
                subIterator = (*subMapping)->createIterator(pos);
            else
                subIterator = 0;
        }
        else {
            if (subIterator)
                subIterator->jumpTo(pos);
        }
    }

    /**
     * @brief Helper method which updates the sub-iterator and sets the position
     * of the sub-iterator to its beginning.
     *
     * Called when the position of of the iterator inside the
     * dimension this Iterator represents has changed.
     */
    void updateSubIterator()
    {
        interpolated subM = valueIt.getValue();
        if (subM != subMapping) {
            if (subIterator)
                delete subIterator;

            subMapping = subM;
            if (*subMapping) {
                if (subMapping.isInterpolated)
                    subIterator = (*subMapping)->createIterator(position);
                else
                    subIterator = (*subMapping)->createIterator();
            }
            else
                subIterator = 0;
        }
        else {
            if (subIterator)
                subIterator->jumpToBegin();
        }
    }

    /**
     * @brief Helper method which updates the nextPosition member.
     *
     * Called when the current position has changed.
     */
    void updateNextPosition()
    {
        bool intp = subMapping.isInterpolated;

        bool noSubIt = false;
        bool hasNoNext = false;
        if (!intp) {
            noSubIt = !subIterator;
            if (!noSubIt)
                hasNoNext = !subIterator->hasNext();
        }
        if (intp || noSubIt || hasNoNext) {
            if (valueIt.hasNext()) {
                ConstMappingIterator *tmp = (*valueIt.getNextValue())->createConstIterator();
                nextPosition.setArgValues(tmp->getPosition());
                delete tmp;
            }
            else {
                nextPosition = position;
            }
            nextPosition.setArgValue(mapping.myDimension, valueIt.getNextPosition());
        }
        else {
            nextPosition.setArgValues(subIterator->getNextPosition());
        }
    }

  private:
    /** @brief Assignment operator is not allowed.
     */
    MultiDimMappingIterator& operator=(const MultiDimMappingIterator&);

  public:
    /**
     * @brief Initializes the Iterator for the passed MultiDimMapping and sets
     * its position two the first entry of the passed MultiDimMapping.
     */
    MultiDimMappingIterator(MultiDimMapping<Interpolator>& pMapping) :
        MappingIterator(), mapping(pMapping),
        valueIt(pMapping.entries.beginIntpl()),
        subMapping(0), subIterator(nullptr),
        position(), nextPosition()
    {
        subMapping = valueIt.getValue();
        if (!subMapping.isInterpolated && *subMapping) {
            subIterator = (*subMapping)->createIterator();
            position = subIterator->getPosition();
            position.setArgValue(mapping.myDimension, valueIt.getPosition());
        }
        else {
            position = Argument(mapping.dimensions);
        }
        nextPosition = position;

        updateNextPosition();
    }

    /**
     * @brief Intializes the Iterator for the passed MultiDimMapping and sets
     * its position two the passed position.
     */
    MultiDimMappingIterator(MultiDimMapping<Interpolator>& pMapping, const Argument& pos) :
        MappingIterator(), mapping(pMapping),
        valueIt(pMapping.entries.beginIntpl()    /*pMapping.entries.findIntpl(pos.getArgValue(pMapping.myDimension))*/),    //ATTENTION: pMapping.entries.findIntpl(...) results in GCC-Crash at -O2
        subMapping(0), subIterator(nullptr),
        position(pos), nextPosition()
    {
        // valueIt was not initialized with pMapping.entries.findIntpl(...), so we need the jumpTo-call
        valueIt.jumpTo(position.getArgValue(mapping.myDimension));
        subMapping = valueIt.getValue();
        if (*subMapping) {
            subIterator = (*subMapping)->createIterator(position);
        }
        nextPosition = position;

        updateNextPosition();
    }

    MultiDimMappingIterator(const MultiDimMappingIterator& o) :
        MappingIterator(o), mapping(o.mapping),
        valueIt(o.valueIt),
        subMapping(o.subMapping), subIterator(nullptr),
        position(o.position), nextPosition(o.nextPosition)
    {
        // valueIt was not initialized with pMapping.entries.findIntpl(...), so we need the jumpTo-call
        if (*subMapping) {
            subIterator = (*subMapping)->createIterator(position);
        }
    }

    /**
     * @brief Frees the memory allocated for the sub mappings.
     */
    virtual ~MultiDimMappingIterator()
    {
        if (subIterator)
            delete subIterator;
    }

    /**
     * @brief Lets the iterator point to the passed position.
     *
     * The passed new position can be at arbitrary places.
     *
     * Has logarithmic complexity in number of dimensions and number of
     * entries inside each dimension.
     */
    void jumpTo(const Argument& pos) override
    {
        argument_value_cref_t argVal = pos.getArgValue(mapping.myDimension);

        if (argVal != valueIt.getPosition() && pos.hasArgVal(mapping.myDimension)) {
            valueIt.jumpTo(argVal);
            updateSubIterator(pos);
        }
        else {
            if (subIterator)
                subIterator->jumpTo(pos);
        }

        position.setArgValues(pos);
        nextPosition.setArgValues(position);
        updateNextPosition();
    }

    /**
     * @brief Iterates to the specified position. This method
     * should be used if the new position is near the current position.
     *
     * The new position has to be compared bigger than the current position
     *
     * Has linear complexity over the number of entries between the current
     * position and the passed position. This leads to nearly constant
     * complexity for position close together.
     */
    void iterateTo(const Argument& pos) override
    {
        argument_value_cref_t argVal = pos.getArgValue(mapping.myDimension);

        if (argVal != valueIt.getPosition() && pos.hasArgVal(mapping.myDimension)) {
            valueIt.iterateTo(argVal);
            updateSubIterator(pos);
        }
        else {
            if (subIterator)
                subIterator->iterateTo(pos);
        }

        position.setArgValues(pos);
        updateNextPosition();
    }

    /**
     * @brief Iterates to the next position of the function.
     *
     * The next position depends on the implementation of the
     * Function.
     * Calling this method will always work, but if their is no next
     * entry to iterate to inside the underlying Mapping the actual
     * position next jumps will be valid but without meaning.
     * Therefore "hasNext()" should be called before calling this method.
     *
     * Has constant complexity.
     */
    virtual void next() override
    {
        if (!subMapping.isInterpolated && subIterator && subIterator->hasNext()) {
            subIterator->next();
        }
        else {
            valueIt.next();
            updateSubIterator();
        }

        if (subIterator)
            position.setArgValues(subIterator->getPosition());

        position.setArgValue(mapping.myDimension, valueIt.getPosition());

        updateNextPosition();
    }

    /**
     * @brief Returns true if the current position of the iterator
     * is in range of the function.
     *
     * This method should be used as end-condition when iterating
     * over the function with the "next()" method.
     *
     * Constant complexity.
     */
    virtual bool inRange() const override
    {
        return valueIt.inRange() && (subMapping.isInterpolated || (subIterator && subIterator->inRange()));
    }

    /**
     * @brief Returns the current position of the iterator.
     *
     * Constant complexity.
     */
    virtual const Argument& getPosition() const override
    {
        return position;
    }

    /**
     * @brief returns the next position a call to "next()" would jump to.
     *
     * Constant complexity.
     */
    virtual const Argument& getNextPosition() const override
    {
        return nextPosition;
    }

    /**
     * @brief Returns the value of the underlying mapping at the current
     * position.
     *
     * Has constant complexity.
     */
    virtual argument_value_t getValue() const override
    {
        if (subIterator)
            return subIterator->getValue();
        else
            return Argument::MappedZero;
    }

    /**
     * @brief Lets the iterator point to the begin of the function.
     *
     * The beginning of the function depends is the position of the first
     * entry in the underlying Mapping.
     *
     * Constant complexity.
     */
    virtual void jumpToBegin() override
    {
        valueIt.jumpToBegin();
        updateSubIterator();
        if (subIterator)
            position.setArgValues(subIterator->getPosition());

        position.setArgValue(mapping.myDimension, valueIt.getPosition());
        updateNextPosition();
    }

    /**
     * @brief Returns true if the iterator has a valid next value a call to "next()"
     * could jump to.
     *
     * Constant complexity.
     */
    virtual bool hasNext() const override
    {
        return valueIt.hasNext() || (subIterator && subIterator->hasNext() && valueIt.inRange());
    }

    /**
     * @brief Changes the value of the function at the current
     * position.
     *
     * Constant complexity.
     */
    virtual void setValue(argument_value_cref_t value) override
    {
        if (subMapping.isInterpolated) {
            valueIt.setValue(mapping.createSubSignal());
            updateSubIterator(position);
        }
        subIterator->setValue(value);
    }
};

/**
 * @brief Implementation of the Mapping-interface which is able to represent
 * arbitrary dimensional instances of Mappings by using a tree-like structure
 * of sub-mappings, each representing the values for one of the dimensions.
 *
 * This class internally uses a map of Mappings two represent one dimension.
 * Where every Mapping in the map represents a sub-mapping for the values in
 * the next dimension at that position in the this dimension. These sub-mappings
 * can either be in turn MultiDimMappings with further sub-mappings or they can
 * be TimedMappings if their dimension is the time. The TimedMappings therefore
 * represent the leafs of the tree-like structure.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
template<template<typename> class Interpolator>
class MultiDimMapping : public Mapping
{
  protected:
    /** @brief The templated InterpolateableMap the underlying Mapping uses std::map as storage type.*/
    typedef InterpolateableMap<Interpolator<std::map<argument_value_t, Mapping *> > >
        interpolator_map_type;
    typedef typename interpolator_map_type::interpolator_type interpolator_type;
    typedef typename interpolator_map_type::mapped_type mapped_type;
    typedef typename interpolator_map_type::mapped_cref_type mapped_cref_type;
    typedef typename interpolator_map_type::iterator_intpl iterator;
    typedef typename interpolator_map_type::const_iterator_intpl const_iterator;

    /**
     * @brief Returned by the Interpolator if the mapping is accessed outside
     * its range (before or after the last key entry in a dimension).
     */
    ConstantSimpleConstMapping *outOfRangeMapping;

    /**
     * @brief Wraps the out of range mapping which is an instance of ConstMapping
     * inside an instance of Mapping which setValue method is asserted to never be called.
     */
    ConstMappingWrapper *wrappedOORMapping;

    /** @brief Stores the sub-mappings for the dimension this instance represents.*/
    interpolator_map_type entries;

    /** @brief The dimension this instance represents.*/
    Dimension myDimension;

    bool isMaster;

    friend class MultiDimMappingIterator<Interpolator>;

  protected:
    /**
     * @brief Initializes the Mapping with the passed DimensionSet as domain and
     * the passed dimension as the dimension this instance should represent.
     *
     * Also takes the interpolation method to use, but not yet implemented.
     *
     * This constructor is only used internally to create the sub-mappings.
     */
    MultiDimMapping(const DimensionSet& myDims, Dimension myDim) :
        Mapping(myDims),
        outOfRangeMapping(0),
        wrappedOORMapping(0),
        entries(),
        myDimension(myDim),
        isMaster(false) {}

    /**
     * @brief Initializes the Mapping with the passed DimensionSet as domain and
     * the passed dimension as the dimension this instance should represent.
     *
     * Also takes the interpolation method to use, but not yet implemented.
     *
     * This constructor is only used internally to create the sub-mappings.
     */
    MultiDimMapping(const DimensionSet& myDims, Dimension myDim,
            ConstantSimpleConstMapping *oorm,
            ConstMappingWrapper *wrappedoorm) :
        Mapping(myDims),
        outOfRangeMapping(oorm),
        wrappedOORMapping(wrappedoorm),
        entries(wrappedOORMapping),
        myDimension(myDim),
        isMaster(false) {}

    /**
     * @brief Intern copy-constructor which assures that the sub-mappings are deep
     * copied instead of only their pointers.
     */
    MultiDimMapping(const MultiDimMapping<Interpolator>& o,
            ConstantSimpleConstMapping *oorm,
            ConstMappingWrapper *wrappedoorm) :
        Mapping(o),
        outOfRangeMapping(oorm),
        wrappedOORMapping(wrappedoorm),
        entries(o.entries),
        myDimension(o.myDimension),
        isMaster(false)
    {
        entries.setOutOfRangeVal(wrappedOORMapping);

        copySubMappings();
    }

    /**
     * @brief Internal helper method which creates a new sub-mapping for this
     * MultiDimMapping instance.
     */
    mapped_type createSubSignal() const
    {
        const Dimension& nextDim = *(--dimensions.find(myDimension));
        if (wrappedOORMapping == 0) {
            if (nextDim == Dimension::time)
                return new TimeMapping<Interpolator>();
            else
                return new MultiDimMapping<Interpolator>(dimensions, nextDim);
        }
        else {
            if (nextDim == Dimension::time)
                return new TimeMapping<Interpolator>(outOfRangeMapping->getValue());
            else
                return new MultiDimMapping<Interpolator>(dimensions, nextDim, outOfRangeMapping, wrappedOORMapping);
        }
    }

    void copySubMappings()
    {
        const auto itEnd = entries.end();
        Dimension nextDim = *(--dimensions.find(myDimension));

        if (nextDim == Dimension::time) {
            for (auto it = entries.begin(); it != itEnd; ++it) {
                it->second = new TimeMapping<Interpolator>(*(static_cast<TimeMapping<Interpolator> *>(it->second)));
            }
        }
        else {
            for (auto it = entries.begin(); it != itEnd; ++it) {
                if (outOfRangeMapping == 0) {
                    it->second = new MultiDimMapping<Interpolator>(*(static_cast<MultiDimMapping<Interpolator> *>(it->second)));
                }
                else {
                    it->second = new MultiDimMapping<Interpolator>(*(static_cast<MultiDimMapping<Interpolator> *>(it->second)), outOfRangeMapping, wrappedOORMapping);
                }
            }
        }
    }

  public:
    /**
     * @brief Initializes the Mapping with the passed DimensionSet as domain.
     *
     * Also takes the interpolation-method but is not used yet.
     */
    MultiDimMapping(const DimensionSet& myDims) :
        Mapping(myDims),
        outOfRangeMapping(0),
        wrappedOORMapping(0),
        entries(),
        myDimension(),
        isMaster(true)
    {
        myDimension = *(dimensions.rbegin());
    }

    /**
     * @brief Initializes the Mapping with the passed DimensionSet as domain.
     *
     * Also takes the interpolation-method but is not used yet.
     */
    MultiDimMapping(const DimensionSet& myDims, argument_value_cref_t oorv) :
        Mapping(myDims),
        outOfRangeMapping(new ConstantSimpleConstMapping(myDims, oorv)),
        wrappedOORMapping(new ConstMappingWrapper(outOfRangeMapping)),
        entries(wrappedOORMapping),
        myDimension(),
        isMaster(true)
    {
        myDimension = *(dimensions.rbegin());
    }

    /**
     * @brief Copy-constructor which assures that the sub-mappings are deep
     * copied instead of only their the pointers.
     */
    MultiDimMapping(const MultiDimMapping<Interpolator>& o) :
        Mapping(o),
        outOfRangeMapping(o.outOfRangeMapping),
        wrappedOORMapping(o.wrappedOORMapping),
        entries(o.entries),
        myDimension(o.myDimension),
        isMaster(true)
    {
        if (outOfRangeMapping != 0) {
            outOfRangeMapping = new ConstantSimpleConstMapping(dimensions, o.outOfRangeMapping->getValue());
            wrappedOORMapping = new ConstMappingWrapper(outOfRangeMapping);
            entries.setOutOfRangeVal(wrappedOORMapping);
        }
        copySubMappings();
    }

    /**
     * @brief Copy operator which assures that the sub-mappings are deep
     * copied instead of only their the pointers.
     */
    const MultiDimMapping& operator=(const MultiDimMapping<Interpolator>& o)
    {
        const typename interpolator_map_type::const_iterator itEnd = entries.end();
        for (typename interpolator_map_type::const_iterator it = entries.begin(); it != itEnd; ++it) {
            if (it->second)
                delete it->second;
        }

        dimensions = o.dimensions;
        entries = o.entries;
        myDimension = o.myDimension;
        outOfRangeMapping = o.outOfRangeMapping;
        wrappedOORMapping = o.wrappedOORMapping;
        isMaster = true;

        if (outOfRangeMapping != 0) {
            outOfRangeMapping = new ConstantSimpleConstMapping(dimensions, o.outOfRangeMapping->getValue());
            wrappedOORMapping = new ConstMappingWrapper(outOfRangeMapping);
            entries.setOutOfRangeVal(wrappedOORMapping);
        }

        copySubMappings();

        return *this;
    }

    /**
     * @brief returns a deep copy of this mapping instance.
     */
    virtual Mapping *clone() const override { return new MultiDimMapping<Interpolator>(*this); }

    /**
     * @brief Frees the memory for the sub mappings.
     */
    virtual ~MultiDimMapping()
    {
        const typename interpolator_map_type::const_iterator itEnd = entries.end();
        for (typename interpolator_map_type::const_iterator it = entries.begin(); it != itEnd; ++it) {
            if (it->second)
                delete it->second;
        }

        if (isMaster) {
            if (outOfRangeMapping)
                delete outOfRangeMapping;
            if (wrappedOORMapping)
                delete wrappedOORMapping;
        }
    }

    /**
     * @brief Returns the value of this Mapping at position specified
     * by the passed Argument.
     *
     * Has logarithmic complexity over the number of dimensions and the number of
     * entries per dimension.
     */
    virtual argument_value_t getValue(const Argument& pos) const override
    {
        assert(pos.hasArgVal(myDimension));
        argument_value_cref_t argVal = pos.getArgValue(myDimension);
        typename interpolator_map_type::interpolated subM = entries.getIntplValue(argVal);

        if (!(*subM))
            return Argument::MappedZero;

        return (*subM)->getValue(pos);
    }

    /**
     * @brief Changes the value of the Mapping at the specified
     * position.
     *
     * Has logarithmic complexity over the number of dimensions and the number of
     * entries per dimension.
     */
    virtual void setValue(const Argument& pos, argument_value_cref_t value) override
    {
        argument_value_cref_t argVal = pos.getArgValue(myDimension);
        auto posIt = entries.lower_bound(argVal);

        if (posIt == entries.end() || (entries.key_comp()(argVal, posIt->first))) {
            posIt = entries.insert(posIt, std::make_pair(argVal, createSubSignal()));
        }
        posIt->second->setValue(pos, value);
    }

    /**
     * @brief Returns a pointer of a new Iterator which is able to iterate
     * over the Mapping and can change the value the iterator points to.
     *
     * The caller of this method has to delete the Iterator if not needed
     * anymore.
     */
    virtual MappingIterator *createIterator() override
    {
        return new MultiDimMappingIterator<Interpolator>(*this);
    }

    /**
     * @brief Returns a pointer of a new Iterator which is able to iterate
     * over the Mapping and can change the value the iterator points to.
     *
     * The caller of this method has to delete the Iterator if not needed
     * anymore.
     */
    virtual MappingIterator *createIterator(const Argument& pos) override
    {
        return new MultiDimMappingIterator<Interpolator>(*this, pos);
    }

    /**
     * @brief Returns the dimension this instance represents.
     */
    Dimension getDimension() { return myDimension; }
};

/**
 * @brief MappingIterator implementation for FilledUpMappings.
 *
 * Assures that although FilledUpMapping is an Mapping instance the
 * "setValue()"-method may never be called.
 *
 * @sa FilledUpMapping
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
class INET_API FilledUpMappingIterator : public MultiDimMappingIterator<Linear>
{
  public:
    FilledUpMappingIterator(FilledUpMapping& mapping);

    FilledUpMappingIterator(FilledUpMapping& mapping, const Argument& pos);

    virtual void setValue(argument_value_cref_t) override
    {
        assert(false);
    }
};

/**
 * @brief Takes a source ConstMapping with a domain A and a set of KeyEntries
 * for a domain B and creates a clone of the source mapping with the domain B
 * and the KeyEntries passed.
 *
 * This class is used by "applyElementWiseOperator()"-method to be able
 * to handle cases where the second mappings domain is a real subset of
 * the first mappings domain (meaning the first mappings domain has the same
 * dimensions as the seconds domain and at least one further dimension).
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
class INET_API FilledUpMapping : public MultiDimMapping<Linear>
{
//--------members----------

  public:
    typedef std::set<argument_value_t> KeySet;
    typedef std::map<Dimension, KeySet> KeyMap;

  protected:
    Mapping *fillRef;
    const KeyMap& keys;

  private:
    /** @brief Copy constructor is not allowed.
     */
    FilledUpMapping(const FilledUpMapping&);
    /** @brief Assignment operator is not allowed.
     */
    FilledUpMapping& operator=(const FilledUpMapping&);

//--------methods----------

  protected:
    void fillRefIfNecessary()
    {
        const KeyMap::const_iterator itEnd = keys.end();
        KeyMap::const_iterator it = keys.find(myDimension);

        if (it == itEnd)
            return;

        fillRef = createSubSignal();

        const KeySet::const_iterator keyItEnd = it->second.end();
        for (KeySet::const_iterator keyIt = it->second.begin(); keyIt != keyItEnd; ++keyIt) {
            entries.insert(entries.end(), std::make_pair(*keyIt, fillRef));
        }
    }

    FilledUpMapping(const DimensionSet& myDims, Dimension myDim, const KeyMap& rkeys) :
        MultiDimMapping<Linear>(myDims, myDim), fillRef(0), keys(rkeys)
    {
        fillRefIfNecessary();
    }

    Mapping *createSubSignal() const
    {
        const Dimension& nextDim = *(--dimensions.find(myDimension));
        if (nextDim == Dimension::time)
            return MultiDimMapping<Linear>::createSubSignal();
        else
            return new FilledUpMapping(dimensions, nextDim, keys);
    }

  public:
    FilledUpMapping(const ConstMapping *source, const DimensionSet& dims, const KeyMap& rkeys) :
        MultiDimMapping<Linear>(dims), fillRef(0), keys(rkeys)
    {
        ConstMappingIterator *it = source->createConstIterator();

        if (it->inRange()) {
            fillRefIfNecessary();

            while (it->inRange()) {
                appendValue(it->getPosition(), it->getValue());

                if (!it->hasNext())
                    break;
                it->next();
            }
        }
        delete it;
    }

    virtual ~FilledUpMapping()
    {
        if (fillRef != 0) {
            delete fillRef;
            entries.clear();
        }
    }

    virtual void appendValue(const Argument& pos, argument_value_cref_t value) override
    {
        if (fillRef != 0) {
            fillRef->appendValue(pos, value);
            return;
        }
        argument_value_cref_t argVal = pos.getArgValue(myDimension);
        auto posIt = entries.lower_bound(argVal);

        if (posIt == entries.end() || (entries.key_comp()(argVal, posIt->first))) {
            posIt = entries.insert(posIt, std::make_pair(argVal, createSubSignal()));
        }
        posIt->second->appendValue(pos, value);
    }

    virtual MappingIterator *createIterator() override
    {
        return new FilledUpMappingIterator(*this);
    }

    virtual MappingIterator *createIterator(const Argument& pos) override
    {
        return new FilledUpMappingIterator(*this, pos);
    }
};

/**
 * @brief Provides several utility methods for Mappings.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
class INET_API MappingUtils
{
  public:
    typedef std::list<const ConstMapping *> MappingBuffer;

    /** @brief The default value for findMin() functions if it does not find a minimum element.
     *
     * It will be initialized with the infinity value.
     */
    const static Argument::mapped_type cMinNotFound;
    /** @brief The default value for findMax() functions if it does not find a maximum element.
     *
     * It will be initialized with the negative infinity value.
     */
    const static Argument::mapped_type cMaxNotFound;

  private:
    static const ConstMapping *createCompatibleMapping(const ConstMapping& src, const ConstMapping& dst);

    static bool iterateToNext(ConstMappingIterator *it1, ConstMappingIterator *it2);

  public:

    /**
     * @brief Returns an appropriate changeable Mapping with the specified domain
     * and the specified interpolation method.
     *
     * Note: The interpolation method is always linear, at the moment.
     */
    static Mapping *createMapping(const DimensionSet& domain = DimensionSet(Dimension::time),
            Mapping::InterpolationMethod intpl = Mapping::LINEAR);

    /**
     * @brief Returns an appropriate changeable Mapping with the specified domain
     * and the specified interpolation method.
     *
     * Note: The interpolation method is always linear, at the moment.
     */
    static Mapping *createMapping(Mapping::argument_value_cref_t outOfRangeValue,
            const DimensionSet& domain = DimensionSet(Dimension::time),
            Mapping::InterpolationMethod intpl = Mapping::LINEAR);

    template<class Operator>
    static Mapping *applyElementWiseOperator(const ConstMapping& f1, const ConstMapping& f2,
            const Argument& intvlStart,
            const Argument& intvlEnd,
            Operator op)
    {
        return 0;
    }

    template<class Operator>
    static Mapping *applyElementWiseOperator(const ConstMapping& f1, const ConstMapping& f2, Operator op,
            Mapping::argument_value_cref_t outOfRangeVal = Argument::MappedZero,
            bool contOutOfRange = true)
    {
        using std::operator<<;

        const ConstMapping *const f2Comp = createCompatibleMapping(f2, f1);
        const ConstMapping *const f1Comp = createCompatibleMapping(f1, f2);

        const DimensionSet& domain = f1Comp->getDimensionSet();
        Mapping *const result = (contOutOfRange) ? MappingUtils::createMapping(domain) : MappingUtils::createMapping(outOfRangeVal, domain);

        ConstMappingIterator *const itF1 = f1Comp->createConstIterator();
        ConstMappingIterator *const itF2 = f2Comp->createConstIterator();
        const bool bF1InRange = itF1->inRange();
        const bool bF2InRange = itF2->inRange();

        if (!bF1InRange && !bF2InRange) {
            delete itF1;
            delete itF2;
            return result;
        }

        MappingIterator *itRes = 0;

        if (bF1InRange && (!bF2InRange || itF1->getPosition() < itF2->getPosition())) {
            itF2->jumpTo(itF1->getPosition());
        }
        else {
            itF1->jumpTo(itF2->getPosition());
        }

        itRes = result->createIterator(itF1->getPosition());

        while (itF1->inRange() || itF2->inRange()) {
//            assert(itF1->getPosition().isSamePosition(itF2->getPosition()));

            Mapping::argument_value_cref_t prod = op(itF1->getValue(), itF2->getValue());
            //result->setValue(itF1->getPosition(), prod);
            itRes->setValue(prod);

            if (!iterateToNext(itF1, itF2))
                break;

            itRes->iterateTo(itF1->getPosition());
        }

        delete itF1;
        delete itF2;
        delete itRes;

        if (&f2 != f2Comp)
            delete (f2Comp);
        if (&f1 != f1Comp)
            delete (f1Comp);

        return result;
    }

    /**
     * @brief Multiplies the passed functions element-wise with each other
     * and returns the result in a new Function.
     *
     * The domain (DimensionSet) of the result is defined by the domain
     * of the first operand.
     * The domain of the second Mapping has to be a subset of the domain of
     * the first mapping.
     */
    static Mapping *multiply(const ConstMapping& f1, const ConstMapping& f2);
    static Mapping *add(const ConstMapping& f1, const ConstMapping& f2);
    static Mapping *subtract(const ConstMapping& f1, const ConstMapping& f2);
    static Mapping *divide(const ConstMapping& f1, const ConstMapping& f2);

    static Mapping *multiply(const ConstMapping& f1, const ConstMapping& f2, Mapping::argument_value_cref_t outOfRangeVal);
    static Mapping *add(const ConstMapping& f1, const ConstMapping& f2, Mapping::argument_value_cref_t outOfRangeVal);
    static Mapping *subtract(const ConstMapping& f1, const ConstMapping& f2, Mapping::argument_value_cref_t outOfRangeVal);
    static Mapping *divide(const ConstMapping& f1, const ConstMapping& f2, Mapping::argument_value_cref_t outOfRangeVal);

    /**
     * @brief Iterates over the passed mapping and returns value at the key entry
     * with the highest value.
     *
     * @param m            The map where the maximum value shall be searched.
     * @param cRetNotFound The value which shall be returned if no maximum was found (default MappingUtils::cMaxNotFound).
     * @return The value at the key entry with the highest value or <tt>cRetNotFound</tt> if map is empty.
     */
    static Argument::mapped_type findMax(const ConstMapping& m, Argument::mapped_type_cref cRetNotFound = cMaxNotFound);

    /**
     * @brief Iterates over the passed mapping and returns the value at the key
     * entry with the highest value in the range defined by the passed min and
     * max parameter.
     *
     * The area defined by the min and max parameter is the number of key entries
     * which position in each dimension is bigger or equal than the value of the min
     * parameter in that dimension and smaller or equal than max parameter in
     * that dimension.
     *
     * @param m            The map where the maximum value shall be searched.
     * @param min          The beginning of search range.
     * @param max          The end of search range.
     * @param cRetNotFound The value which shall be returned if no maximum was found (default MappingUtils::cMaxNotFound).
     * @return The value at the key entry with the highest value or <tt>cRetNotFound</tt> if map is empty or no element in range [min,max].
     */
    static Argument::mapped_type findMax(const ConstMapping& m, const Argument& min, const Argument& max, Argument::mapped_type_cref cRetNotFound = cMaxNotFound);

    /**
     * @brief Iterates over the passed mapping and returns value at the key entry
     * with the smallest value.
     *
     * @param m            The map where the minimum value shall be searched.
     * @param cRetNotFound The value which shall be returned if no minimum was found (default MappingUtils::cMinNotFound).
     * @return The value at the key entry with the lowest value or <tt>cRetNotFound</tt> if map is empty.
     */
    static Argument::mapped_type findMin(const ConstMapping& m, Argument::mapped_type_cref cRetNotFound = cMinNotFound);

    /**
     * @brief Iterates over the passed mapping and returns the value at the key
     * entry with the smallest value in the range defined by the passed min and
     * max parameter.
     *
     * The area defined by the min and max parameter is the number of key entries
     * which position in each dimension is bigger or equal than the value of the min
     * parameter in that dimension and smaller or equal than max parameter in
     * that dimension.
     *
     * @param m            The map where the minimum value shall be searched.
     * @param min          The beginning of search range.
     * @param max          The end of search range.
     * @param cRetNotFound The value which shall be returned if no minimum was found (default MappingUtils::cMinNotFound).
     * @return The value at the key entry with the highest value or <tt>cRetNotFound</tt> if map is empty or no element in range [min,max].
     */
    static Argument::mapped_type findMin(const ConstMapping& m, const Argument& min, const Argument& max, Argument::mapped_type_cref cRetNotFound = cMinNotFound);

    /*
       static Mapping* multiply(ConstMapping& f1, ConstMapping& f2, const Argument& from, const Argument& to);
       static Mapping* add(ConstMapping& f1, ConstMapping& f2, const Argument& from, const Argument& to);
       static Mapping* subtract(ConstMapping& f1, ConstMapping& f2, const Argument& from, const Argument& to);
       static Mapping* divide(ConstMapping& f1, ConstMapping& f2, const Argument& from, const Argument& to);
     */

    /**
     * @brief Adds a discontinuity in time-dimension, i.e. its representation, to a passed mapping.
     *
     * This is done by setting a regular entry and a limit-entry. The limit-entry shall be
     * very close to the regular entry (on its left or right).
     *
     * The implementation works simply by adding the limit-value as a separate entry at the
     * position of the limit-time. This means that this methods adds a total of two entries
     * to the passed mapping.
     *
     * Note: One should use the methods 'pre' or 'post' provided by MappingUtils to calculate
     * the limit-time for the discontinuity.
     *
     * @param m The mapping the discontinuity will be added to.
     * @param pos The position of the regular entry.
     * @param value The value of the regular entry.
     * @param limitTime The time-point of the limit-entry.
     * @param limitValue The value of the limit-entry.
     *
     */
    static void addDiscontinuity(Mapping *m,
            const Argument& pos, Argument::mapped_type_cref value,
            simtime_t_cref limitTime, Argument::mapped_type_cref limitValue);

    /**
     * @brief returns the closest value of simtime before passed value
     */
    static simtime_t pre(simtime_t_cref t);

    /**
     * @brief returns the closest value of simtime after passed values
     */
    static simtime_t post(simtime_t_cref t);

    /**
     * @brief returns the incremented position point (used in RSAMConstMappingIterator::setNextPosition).
     */
    static simtime_t incNextPosition(simtime_t_cref t);
};

/**
 * @brief Deletes its ConstMapping when this iterator is deleted.
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
class INET_API ConcatConstMappingIterator : public FilteredConstMappingIterator
{
//--------members----------

  protected:
    ConstMapping *baseMapping;

  private:
    /** @brief Copy constructor is not allowed.
     */
    ConcatConstMappingIterator(const ConcatConstMappingIterator&);
    /** @brief Assignment operator is not allowed.
     */
    ConcatConstMappingIterator& operator=(const ConcatConstMappingIterator&);

  public:
    ConcatConstMappingIterator(ConstMapping *baseMapping) :
        FilteredConstMappingIterator(baseMapping->createConstIterator()),
        baseMapping(baseMapping) {}

    ConcatConstMappingIterator(ConstMapping *baseMapping, const Argument& pos) :
        FilteredConstMappingIterator(baseMapping->createConstIterator(pos)),
        baseMapping(baseMapping) {}

    virtual ~ConcatConstMappingIterator()
    {
        if (baseMapping)
            delete baseMapping;
    }
};

/**
 * @brief Defines it values by concatenating one or more
 * Mappings to a reference Mapping.
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
template<class Operator>
class ConcatConstMapping : public ConstMapping
{
  protected:
    typedef std::pair<Dimension, Argument::const_iterator> DimIteratorPair;
    typedef std::list<ConstMapping *> MappingSet;

    MappingSet mappings;
    ConstMapping *refMapping;

    bool continueOutOfRange;
    Argument::mapped_type oorValue;
    Operator op;

  public:
    ConcatConstMapping(const ConcatConstMapping& o)
        : ConstMapping(o)
        , mappings(o.mappings)
        , refMapping(nullptr)
        , continueOutOfRange(o.continueOutOfRange)
        , oorValue(o.oorValue)
        , op()
    {}

    /**
     *  @brief  %ConcatConstMapping assignment operator.
     *  @param  copy  A %ConcatConstMapping of identical element and allocator types.
     *
     *  All the elements of @a copy are copied.
     */
    ConcatConstMapping& operator=(const ConcatConstMapping& copy)
    {
        ConcatConstMapping tmp(copy);    // All resource all allocation happens here.
                                         // If this fails the copy will throw an exception
                                         // and 'this' object is unaffected by the exception.
        swap(tmp);
        return *this;
    }

    /**
     *  @brief  Swaps data with another %ConcatConstMapping.
     *  @param  s  A %ConcatConstMapping of the same element and allocator types.
     *
     *  This exchanges the elements between two ConcatConstMapping's in constant time.
     *  Note that the global std::swap() function is specialized such that
     *  std::swap(s1,s2) will feed to this function.
     */
    void swap(ConcatConstMapping& s)
    {
        ConstMapping::swap(s);    // swap the base class members
        /* Swap all D members */
        std::swap(mappings, s.mappings);
        std::swap(refMapping, s.refMapping);
        std::swap(continueOutOfRange, s.continueOutOfRange);
        std::swap(oorValue, s.oorValue);
        std::swap(op, s.op);
    }

  public:
    /**
     * @brief Initializes with the passed reference Mapping, the operator
     * and the Mappings defined by the passed iterator.
     */
    template<class Iterator>
    ConcatConstMapping(ConstMapping *refMapping,
            Iterator first, Iterator last,
            bool continueOutOfRange = true,
            Argument::mapped_type_cref oorValue = Argument::MappedZero,
            Operator op = Operator()) :
        ConstMapping(refMapping->getDimensionSet()),
        mappings(),
        refMapping(refMapping),
        continueOutOfRange(continueOutOfRange),
        oorValue(oorValue),
        op(op)
    {
        while (first != last) {
            mappings.push_back(*first);
            ++first;
        }
    }

    /**
     * @brief Initializes with the passed reference Mapping, the operator
     * and another Mapping to concatenate.
     */
    ConcatConstMapping(ConstMapping *refMapping, ConstMapping *other,
            bool continueOutOfRange = true,
            Argument::mapped_type_cref oorValue = Argument::MappedZero,
            Operator op = Operator()) :
        ConstMapping(refMapping->getDimensionSet()),
        mappings(),
        refMapping(refMapping),
        continueOutOfRange(continueOutOfRange),
        oorValue(oorValue),
        op(op)
    {
        mappings.push_back(other);
    }

    virtual ~ConcatConstMapping() { delete refMapping; }

    /**
     * @brief Adds another Mapping to the list of Mappings to
     * concatenate.
     */
    void addMapping(ConstMapping *m)
    {
        mappings.push_back(m);
    }

    virtual Argument::mapped_type getValue(const Argument& pos) const override
    {
        const MappingSet::const_iterator itEnd = mappings.end();
        Argument::mapped_type res = refMapping->getValue(pos);

        for (MappingSet::const_iterator it = mappings.begin(); it != itEnd; ++it) {
            res = op(res, (*it)->getValue(pos));
        }

        return res;
    }

    /**
     * @brief Returns the concatenated Mapping.
     */
    Mapping *createConcatenatedMapping() const
    {
        assert(!mappings.empty());

        MappingSet::const_iterator it = mappings.begin();
        const MappingSet::const_iterator itEnd = mappings.end();

        Mapping *result = MappingUtils::applyElementWiseOperator(*refMapping, **it, op,
                    oorValue, continueOutOfRange);

        while (++it != itEnd) {
            Mapping *buf = result;
            result = MappingUtils::applyElementWiseOperator(*buf, **it, op,
                        oorValue, continueOutOfRange);
            delete buf;
        }

        return result;
    }

    virtual ConstMappingIterator *createConstIterator() const override
    {
        if (mappings.empty()) {
            return refMapping->createConstIterator();
        }
        return new ConcatConstMappingIterator(createConcatenatedMapping());
    }

    virtual ConstMappingIterator *createConstIterator(const Argument& pos) const override
    {
        if (mappings.empty()) {
            return refMapping->createConstIterator(pos);
        }
        return new ConcatConstMappingIterator(createConcatenatedMapping(), pos);
    }

    virtual ConstMapping *constClone() const override
    {
        return new ConcatConstMapping(*this);
    }

    /**
     * @brief Returns the pointer to the reference mapping.
     */
    ConstMapping *getRefMapping()
    {
        return refMapping;
    }
};

/**
 * @brief Common base for a Const- and NonConst-Iterator for a DelayedMapping.
 *
 * @sa BaseDelayedMapping
 * @ingroup mappingDetails
 * @author Karl Wessel
 */
template<class Base, class Iterator>
class BaseDelayedIterator : public Base
{
  protected:
    simtime_t delay;

    Argument position;
    Argument nextPosition;

  protected:
    Argument undelayPosition(const Argument& pos) const
    {
        Argument res(pos);
        res.setTime(res.getTime() - delay);
        return res;
    }

    Argument delayPosition(const Argument& pos) const
    {
        Argument res(pos);
        res.setTime(res.getTime() + delay);
        return res;
    }

    void updatePosition()
    {
        nextPosition = delayPosition(this->origIterator->getNextPosition());
        position = delayPosition(this->origIterator->getPosition());
    }

  public:
    BaseDelayedIterator(Iterator *it, simtime_t_cref delay) :
        Base(it), delay(delay), position(), nextPosition()
    {
        updatePosition();
    }

    virtual ~BaseDelayedIterator() {}

    virtual const Argument& getNextPosition() const { return nextPosition; }

    virtual void jumpTo(const Argument& pos)
    {
        this->origIterator->jumpTo(undelayPosition(pos));
        updatePosition();
    }

    virtual void jumpToBegin()
    {
        this->origIterator->jumpToBegin();
        updatePosition();
    }

    virtual void iterateTo(const Argument& pos)
    {
        this->origIterator->iterateTo(undelayPosition(pos));
        updatePosition();
    }

    virtual void next()
    {
        this->origIterator->next();
        updatePosition();
    }

    virtual const Argument& getPosition() const
    {
        return position;
    }
};

/**
 * @brief ConstIterator for a ConstDelayedMapping
 *
 * @sa ConstDelayedMapping
 * @ingroup mappingDetails
 * @author Karl Wess
 */
typedef BaseDelayedIterator<FilteredConstMappingIterator, ConstMappingIterator> ConstDelayedMappingIterator;

/**
 * @brief Iterator for a DelayedMapping.
 *
 * @sa DelayedMapping
 * @ingroup mappingDetails
 * @author Karl Wessel
 */
typedef BaseDelayedIterator<FilteredMappingIterator, MappingIterator> DelayedMappingIterator;

/**
 * @brief Common base for Const- and NonConst-DelayedMapping.
 *
 * @sa DelayedMapping, ConstDelayedMapping
 * @ingroup mappingDetails
 * @author Karl Wessel
 */
template<class Base>
class BaseDelayedMapping : public Base
{
  protected:
    Base *mapping;
    simtime_t delay;

  protected:
    Argument delayPosition(const Argument& pos) const
    {
        Argument res(pos);
        res.setTime(res.getTime() - delay);
        return res;
    }

  public:
    BaseDelayedMapping(const BaseDelayedMapping& o) :
        Base(o), mapping(o.mapping), delay(o.delay) {}

    /**
     *  @brief  %BaseDelayedMapping assignment operator.
     *  @param  copy  A %BaseDelayedMapping of identical element and allocator types.
     *
     *  All the elements of @a copy are copied.
     */
    BaseDelayedMapping& operator=(const BaseDelayedMapping& copy)
    {
        BaseDelayedMapping tmp(copy);    // All resource all allocation happens here.
                                         // If this fails the copy will throw an exception
                                         // and 'this' object is unaffected by the exception.
        swap(tmp);
        return *this;
    }

    /**
     *  @brief  Swaps data with another %BaseDelayedMapping.
     *  @param  s  A %BaseDelayedMapping of the same element and allocator types.
     *
     *  This exchanges the elements between two BaseDelayedMapping's in constant time.
     *  Note that the global std::swap() function is specialized such that
     *  std::swap(s1,s2) will feed to this function.
     */
    void swap(BaseDelayedMapping& s)
    {
        Base::swap(s);    // swap the base class members
        /* Swap all D members */
        std::swap(mapping, s.mapping);
        std::swap(delay, s.delay);
    }

  public:
    BaseDelayedMapping(Base *mapping, simtime_t_cref delay) :
        Base(mapping->getDimensionSet()), mapping(mapping), delay(delay) {}

    virtual ~BaseDelayedMapping() {}

    virtual typename Base::argument_value_t getValue(const Argument& pos) const
    {
        return mapping->getValue(delayPosition(pos));
    }

    virtual ConstMappingIterator *createConstIterator() const
    {
        return new ConstDelayedMappingIterator(mapping->createConstIterator(), delay);
    }

    virtual ConstMappingIterator *createConstIterator(const Argument& pos) const
    {
        return new ConstDelayedMappingIterator(mapping->createConstIterator(delayPosition(pos)), delay);
    }

    /**
     * @brief Returns the delay used by this mapping.
     */
    virtual simtime_t_cref getDelay() const
    {
        return delay;
    }

    /**
     * @brief Changes the delay to the passed value.
     */
    virtual void delayMapping(simtime_t_cref d)
    {
        delay = d;
    }
};

/**
 * @brief Moves another ConstMapping in its time dimension.
 *
 * See propagation delay effect of the signal for an example
 * how to use this mapping.
 *
 * @ingroup mappingDetails
 * @author Karl Wessel
 */
class INET_API ConstDelayedMapping : public BaseDelayedMapping<const ConstMapping>
{
  public:
    ConstDelayedMapping(const ConstMapping *mapping, simtime_t_cref delay) :
        BaseDelayedMapping<const ConstMapping>(mapping, delay) {}

    virtual ~ConstDelayedMapping() {}

    virtual ConstMapping *constClone() const override
    {
        return new ConstDelayedMapping(mapping->constClone(), delay);
    }
};

/**
 * @brief Moves another Mapping in its time dimension.
 *
 * See propagation delay effect of the signal for an example
 * how to use this mapping.
 *
 * @ingroup mappingDetails
 * @author Karl Wessel
 */
class INET_API DelayedMapping : public BaseDelayedMapping<Mapping>
{
  public:
    DelayedMapping(Mapping *mapping, simtime_t_cref delay) :
        BaseDelayedMapping<Mapping>(mapping, delay) {}

    virtual ~DelayedMapping() {}

    virtual void setValue(const Argument& pos, Argument::mapped_type_cref value) override
    {
        mapping->setValue(delayPosition(pos), value);
    }

    virtual Mapping *clone() const override
    {
        return new DelayedMapping(mapping->clone(), delay);
    }

    virtual MappingIterator *createIterator() override
    {
        return new DelayedMappingIterator(mapping->createIterator(), delay);
    }

    virtual MappingIterator *createIterator(const Argument& pos) override
    {
        return new DelayedMappingIterator(mapping->createIterator(delayPosition(pos)), delay);
    }
};

INET_API Mapping *operator*(const ConstMapping& f1, const ConstMapping& f2);
INET_API Mapping *operator/(const ConstMapping& f1, const ConstMapping& f2);
INET_API Mapping *operator+(const ConstMapping& f1, const ConstMapping& f2);
INET_API Mapping *operator-(const ConstMapping& f1, const ConstMapping& f2);

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MAPPINGUTILS_H

