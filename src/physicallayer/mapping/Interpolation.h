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

#ifndef __INET_INTERPOLATION_H
#define __INET_INTERPOLATION_H

#include <limits>
#include <map>
#include <algorithm>
#include <assert.h>

namespace inet {

namespace physicallayer {

/**
 * @brief Represents an interpolated value of any type.
 *
 * This class is used for performance, transparency and memory
 * reasons. Since the actual value can be of arbitrary
 * type and therefore arbitrary complexity (for example a whole
 * Mapping instance) or could even be a pointer to an existing
 * or a new object. This class hides the actual value from the
 * user and just provides it with the value and takes care
 * of any memory deallocation if necessary.
 *
 * The actual value can be gotten by dereferencing the instance of
 * this class (like with iterators).
 *
 * Returned by InterpolateableMaps "getValue()"
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
template<class V>
class Interpolated
{
  protected:
    typedef V value_type;
    typedef const value_type& value_cref_type;
    typedef value_type& value_ref_type;
    typedef value_type *value_ptr_type;

    /** @brief A value this class represents.*/
    value_type value;

  public:
    /** @brief Stores if the underlying value is interpolated or not.*/
    const bool isInterpolated;

  public:
    Interpolated(value_cref_type v, bool isIntpl = true) :
        value(v), isInterpolated(isIntpl) {}

    /**
     * @brief Copy-constructor which assures that the internal storage is used correctly.
     */
    Interpolated(const Interpolated<value_type>& o) :
        value(o.value), isInterpolated(o.isInterpolated) {}

    value_ref_type operator*()
    {
        return value;
    }

    value_ptr_type operator->()
    {
        return &value;
    }

    /**
     * @brief Two Interpolated<V> are compared equal if the value is the same as well as the "isInterpolated" flag.
     */
    bool operator==(const Interpolated<value_type>& other) const
    {
        return value == other.value && isInterpolated == other.isInterpolated;
    }

    /**
     * @brief Two Interpolated<V> are compared non equal if the value differs or the "isInterpolated" flag.
     */
    bool operator!=(const Interpolated<value_type>& other) const
    {
        return value != other.value || isInterpolated != other.isInterpolated;
    }

  private:
    Interpolated() :
        value(), isInterpolated(true) {}
};

/**
 * @brief Compares a the first value of a pair to a value.
 *
 * @ingroup mappingDetails
 */
template<class Pair, class Key>
class PairLess
{
  public:
    bool operator()(const Pair& p, const Key& v) const
    {
        return p.first < v;
    }

    bool operator()(const Key& v, const Pair& p) const
    {
        return v < p.first;
    }

    bool operator()(const Pair& left, const Pair& right) const
    {
        return left.first < right.first;
    }
};

template<class _ContainerType>
class InterpolatorBase
{
  public:
    typedef _ContainerType storage_type;
    typedef storage_type container_type;
    typedef typename storage_type::key_type key_type;
    typedef const key_type& key_cref_type;
    typedef typename storage_type::mapped_type mapped_type;
    typedef const mapped_type& mapped_cref_type;
    typedef typename storage_type::value_type pair_type;
    typedef typename storage_type::iterator iterator;
    typedef typename storage_type::const_iterator const_iterator;
    typedef PairLess<pair_type, key_type> comparator_type;
    typedef Interpolated<mapped_type> interpolated;

  public:
    /** @brief Comparator for STL functions. */
    comparator_type comp;

  protected:
    bool continueOutOfRange;
    interpolated outOfRangeVal;

  public:
    InterpolatorBase() :
        comp(), continueOutOfRange(true), outOfRangeVal(mapped_type()) {}

    InterpolatorBase(mapped_cref_type oorv) :
        comp(), continueOutOfRange(false), outOfRangeVal(oorv) {}

    virtual ~InterpolatorBase() {}

    /** @{
     *  @brief Getter and Setter method for out of range value.
     */
    void setOutOfRangeVal(mapped_cref_type oorv)
    {
        continueOutOfRange = false;
        outOfRangeVal = interpolated(oorv);
    }

    mapped_cref_type getOutOfRangeVal(void) const
    {
        return *(&outOfRangeVal);
    }

    /** @} */

    bool continueAtOutOfRange(void) const
    {
        return continueOutOfRange;
    }

    /**
     * @brief Functor operator of this class which interpolates the value
     * at the passed position using the values between the passed Iterators.
     *
     * The returned instance of interpolated represents the result. Which can be
     * either an actual entry of the interpolated map (if the position two
     * interpolate was exactly that. Or it can be an interpolated value, if the
     * passed position was between two entries of the map.
     * This state can be retrieved with the "isInterpolated"-Member of the returned
     * "interpolated".
     */
    interpolated operator()(const const_iterator& first,
            const const_iterator& last,
            key_cref_type pos) const
    {
        if (first == last) {
            return outOfRangeVal;
        }

        const_iterator right = std::upper_bound(first, last, pos, comp);

        return operator()(first, last, pos, right);
    }

    /** @brief Represents the interpolator a stepping function. */
    virtual bool isStepping() const { return false; }

    /**
     * @brief Functor operator of this class which interpolates the value
     * at the passed position using the values between the passed Iterators.
     *
     * The upperBound-iterator has to point two the entry next bigger as the
     * passed position to interpolate.
     *
     * The returned instance of interpolated represents the result. Which can be
     * either an actual entry of the interpolated map (if the position to
     * interpolate was exactly that. Or it can be an interpolated value, if the
     * passed position was between two entries of the map.
     * This state can be retrieved with the "isInterpolated"-Member of the returned
     * "interpolated".
     *
     * @return The value of a element between first and last which is nearest to pos and the position is of the element is less or equal to pos.
     */
    virtual
    interpolated operator()(const const_iterator& first,
            const const_iterator& last,
            key_cref_type pos,
            const_iterator upperBound) const = 0;

  protected:
    interpolated asInterpolated(mapped_cref_type rVal, bool bIsOutOfRange, bool bIsInterpolated = true) const
    {
        if (!bIsOutOfRange)
            return interpolated(rVal, bIsInterpolated);

        if (continueOutOfRange)
            return interpolated(rVal);
        else
            return outOfRangeVal;
    }
};
/**
 * @brief Given two iterators defining a range of key-value-pairs this class
 * provides interpolation of values for an arbitrary key by returning the
 * value of the next smaller entry.
 *
 * If there is no smaller entry it returns the next bigger or the
 * "out of range"-value, if set.
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
template<class _ContainerType>
class NextSmaller : public InterpolatorBase<_ContainerType>
{
  protected:
    typedef InterpolatorBase<_ContainerType> base_class_type;

  public:
    typedef typename base_class_type::storage_type storage_type;
    typedef typename base_class_type::container_type container_type;
    typedef typename base_class_type::key_type key_type;
    typedef typename base_class_type::key_cref_type key_cref_type;
    typedef typename base_class_type::mapped_type mapped_type;
    typedef typename base_class_type::mapped_cref_type mapped_cref_type;
    typedef typename base_class_type::pair_type pair_type;
    typedef typename base_class_type::iterator iterator;
    typedef typename base_class_type::const_iterator const_iterator;
    typedef typename base_class_type::comparator_type comparator_type;
    typedef typename base_class_type::interpolated interpolated;

  public:
    NextSmaller() :
        base_class_type() {}

    NextSmaller(mapped_cref_type oorv) :
        base_class_type(oorv) {}

    virtual ~NextSmaller() {}

    /**
     * @brief Functor operator of this class which interpolates the value
     * at the passed position using the values between the passed Iterators.
     *
     * The upperBound-iterator has to point two the entry next bigger as the
     * passed position to interpolate.
     *
     * The returned instance of interpolated represents the result. Which can be
     * either an actual entry of the interpolated map (if the position to
     * interpolate was exactly that. Or it can be an interpolated value, if the
     * passed position was between two entries of the map.
     * This state can be retrieved with the "isInterpolated"-Member of the returned
     * "interpolated".
     *
     * @return The value of a element between first and last which is nearest to pos and the position is of the element is less or equal to pos.
     */
    virtual
    interpolated operator()(const const_iterator& first,
            const const_iterator& last,
            key_cref_type pos,
            const_iterator upperBound) const
    {
        if (first == last) {
            return base_class_type::outOfRangeVal;
        }
        if (upperBound == first) {
            return this->asInterpolated(upperBound->second, true);
        }

        upperBound--;
        return this->asInterpolated(upperBound->second, false, !(upperBound->first == pos));
    }

    /** @brief Represents the interpolator a stepping function. */
    virtual bool isStepping() const { return true; }
};

/**
 * @brief Given two iterators defining a range of key-value-pairs this class
 * provides interpolation of values for an arbitrary key by returning the
 * value of the nearest entry.
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
template<class _ContainerType>
class Nearest : public InterpolatorBase<_ContainerType>
{
  protected:
    typedef InterpolatorBase<_ContainerType> base_class_type;

  public:
    typedef typename base_class_type::storage_type storage_type;
    typedef typename base_class_type::container_type container_type;
    typedef typename base_class_type::key_type key_type;
    typedef typename base_class_type::key_cref_type key_cref_type;
    typedef typename base_class_type::mapped_type mapped_type;
    typedef typename base_class_type::mapped_cref_type mapped_cref_type;
    typedef typename base_class_type::pair_type pair_type;
    typedef typename base_class_type::iterator iterator;
    typedef typename base_class_type::const_iterator const_iterator;
    typedef typename base_class_type::comparator_type comparator_type;
    typedef typename base_class_type::interpolated interpolated;

  public:
    Nearest() :
        base_class_type() {}

    Nearest(mapped_cref_type oorv) :
        base_class_type(oorv) {}

    virtual ~Nearest() {}

    /**
     * @brief Functor operator of this class which interpolates the value
     * at the passed position using the values between the passed Iterators.
     *
     * The upperBound-iterator has to point two the entry next bigger as the
     * passed position to interpolate.
     *
     * The returned instance of interpolated represents the result. Which can be
     * either an actual entry of the interpolated map (if the position to
     * interpolate was exactly that. Or it can be an interpolated value, if the
     * passed position was between two entries of the map.
     * This state can be retrieved with the "isInterpolated"-Member of the returned
     * "interpolated".
     *
     * @return The value of a element between first and last which is nearest to pos.
     */
    virtual
    interpolated operator()(const const_iterator& first,
            const const_iterator& last,
            key_cref_type pos,
            const_iterator upperBound) const
    {
        if (first == last) {
            return base_class_type::outOfRangeVal;
        }
        if (upperBound == first) {
            return this->asInterpolated(upperBound->second, true);
        }

        const_iterator left = upperBound;
        --left;

        if (left->first == pos)
            return this->asInterpolated(left->second, false, false);

        const_iterator right = upperBound;

        if (right == last) {
            return this->asInterpolated(left->second, true);
        }

        return this->asInterpolated(((pos - left->first < right->first - pos) ? left : right)->second, false);
    }
};

template<class TFrom, class TTo>
TTo cast_it(TFrom rValToCast)
{
    return static_cast<TTo>(rValToCast);
}

template<class T>
T cast_it(T rValToCast)
{
    return rValToCast;
}

template<class T>
T cast_it(simtime_t rValToCast)
{
    return cast_it(SIMTIME_DBL(rValToCast));
}

/**
 * @brief Given two iterators defining a range of key-value-pairs this class
 * provides linear interpolation of the value at an arbitrary key-position.
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
template<class _ContainerType>
class Linear : public InterpolatorBase<_ContainerType>
{
  protected:
    typedef InterpolatorBase<_ContainerType> base_class_type;

  public:
    typedef typename base_class_type::storage_type storage_type;
    typedef typename base_class_type::container_type container_type;
    typedef typename base_class_type::key_type key_type;
    typedef typename base_class_type::key_cref_type key_cref_type;
    typedef typename base_class_type::mapped_type mapped_type;
    typedef typename base_class_type::mapped_cref_type mapped_cref_type;
    typedef typename base_class_type::pair_type pair_type;
    typedef typename base_class_type::iterator iterator;
    typedef typename base_class_type::const_iterator const_iterator;
    typedef typename base_class_type::comparator_type comparator_type;
    typedef typename base_class_type::interpolated interpolated;

  public:
    Linear() :
        base_class_type() {}

    Linear(mapped_cref_type oorv) :
        base_class_type(oorv) {}

    virtual ~Linear() {}

    /**
     * @brief Functor operator of this class which linear interpolates the value
     * at the passed position using the values between the passed Iterators.
     *
     * The upperBound-iterator has to point to the entry next bigger as the
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
            const_iterator upperBound) const
    {
        if (first == last) {
            return base_class_type::outOfRangeVal;
        }
        if (upperBound == first) {
            return this->asInterpolated(upperBound->second, true);
        }

        const_iterator right = upperBound;
        const_iterator left = --upperBound;

        if (left->first == pos)
            return this->asInterpolated(left->second, false, false);

        if (right == last) {
            return this->asInterpolated(left->second, true);
        }

        return interpolated(linearInterpolation(pos, left->first, right->first, left->second, right->second));
    }

  protected:
    /**
     * @brief Calculates the linear interpolation factor used for the interpolation.
     */
    static mapped_type linearInterpolationFactor(key_cref_type t, key_cref_type t0, key_cref_type t1)
    {
        assert((t0 <= t && t <= t1) || (t0 >= t && t >= t1));
        if (t0 == t1) {
            return 0;
        }
        return cast_it<mapped_type>((t - t0) / (t1 - t0));
    }

    static mapped_type linearInterpolation(key_cref_type t,
            key_cref_type t0, key_cref_type t1,
            mapped_cref_type v0, mapped_cref_type v1)
    {
        typedef std::numeric_limits<mapped_type> tnumlimits_for_v;

        if (tnumlimits_for_v::has_infinity) {
            // we have possible infinity values, so that we can do some checks
            const mapped_type cInf = tnumlimits_for_v::infinity();
            const bool bV0IsInf = (v0 == cInf) || (tnumlimits_for_v::is_signed ? (v0 == -cInf) : false);

            if (bV0IsInf || (v1 == cInf) || (tnumlimits_for_v::is_signed ? (v1 == -cInf) : false)) {
                if (tnumlimits_for_v::is_signed && (v1 == -v0)) {
                    // v0 == +/-Inf and v1 == -/+Inf
                    if (tnumlimits_for_v::has_quiet_NaN)
                        return tnumlimits_for_v::quiet_NaN();
                    // mhhh!? No quiet_NaN available, so we fall back to old
                    // handling :(
                }
                else {
                    // the result should be infinity
                    return bV0IsInf ? v0 : v1;
                }
            }
        }
        assert((t0 <= t && t <= t1) || (t0 >= t && t >= t1));
        if (t0 == t1) {
            assert(v0 == v1);
            return v0;
        }
        const mapped_type mu = linearInterpolationFactor(t, t0, t1);

        return v0 * (static_cast<mapped_type>(1) - mu) + v1 * mu;
        //return v0 + (((v1 - v0) * (t - t0)) / (t1 - t0));
    }
};

/**
 * @brief Template for an interpolateable const iterator for any container
 * which maps from a key to a value. This doesn't necessarily has to be a
 * map, but also can be a sorted list of pairs.
 *
 * The ConstInterpolateableIterator provides an iterator which as able to
 * iterate in arbitrary steps over a iterateable number of pairs of "Key" and "Value".
 * To determine the Value for a Key which does not exist in within the iterateable
 * number of pairs it Interpolates between the nearby existing pairs.
 * The actual Interpolation is determined by the passed Interpolator-template
 * parameter.
 *
 * An example use would be to be able to iterate over a std::map<double, double>
 * in arbitrary steps (even at positions for which no Key exist inside the map) and
 * be able to return an interpolated Value.
 *
 * NOTE: The ConstInterpolateableIterator will become invalid if the underlying
 *       data structure is changed!
 *
 * Template parameters:
 * Pair         - the type of the pair used as values in the container.
 *                Default is std::map<Key, V>::value_type (which is of type
 *                std::pair<Key, V>.
 *                The Pair type has to provide the two public members "first" and "second".
 * Key          - The type of the "first" member of the Pair type
 * V			- the type of the "second" member of the Pair type
 * Iterator		- the type of the iterator of the container (should be a const iterator).
 *                Default is std::map<Key, V>::const_iterator
 * Interpolator - The Interpolation operator to use, this has to be a class which
 *                overwrites the ()-operator with the following parameters:
 *                Interpolated operator()(const Iterator& first,
 *                                        const Iterator& last,
 *                                        const Key& pos)
 *                Interpolated operator()(const Iterator& first,
 *                                        const Iterator& last,
 *                                        const Key& pos,
 *                                        Iterator upperBound)
 *                See the NextSmaller template for an example of an Interpolator.
 *                Default is NextSmaller<Key, V, Pair, Iterator>.
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
template<typename _Interpolator, typename _IteratorType = typename _Interpolator::const_iterator>
class ConstInterpolateableIterator
{
  public:
    typedef _Interpolator interpolator_type;
    typedef typename interpolator_type::container_type container_type;
    typedef typename interpolator_type::key_type key_type;
    typedef typename interpolator_type::key_cref_type key_cref_type;
    typedef typename interpolator_type::mapped_type mapped_type;
    typedef typename interpolator_type::mapped_cref_type mapped_cref_type;
    typedef typename interpolator_type::pair_type pair_type;
    typedef typename interpolator_type::iterator iterator;
    typedef typename interpolator_type::const_iterator const_iterator;
    typedef typename interpolator_type::comparator_type comparator_type;

    typedef _IteratorType used_iterator;
    /** @brief typedef for the returned Interpolated value of this class.*/
    //typedef Interpolated<V> interpolated;
    typedef typename interpolator_type::interpolated interpolated;

  protected:
    used_iterator first;
    used_iterator last;
    used_iterator right;

    key_type position;
    const interpolator_type& interpolate;

  public:
    /**
     * @brief Initializes the iterator with the passed Iterators
     * as boundaries.
     */
    ConstInterpolateableIterator(const used_iterator& first, const used_iterator& last, const interpolator_type& intpl) :
        first(first), last(last), right(first), position(), interpolate(intpl)
    {
        jumpToBegin();
    }

    virtual ~ConstInterpolateableIterator() {}

    bool operator==(const ConstInterpolateableIterator& other)
    {
        return position == other.position && right == other.right;
    }

    /**
     * @brief Moves the iterator to the passed position. This position
     * can be any value of the Key-type.
     */
    void jumpTo(key_cref_type pos)
    {
        if (pos == position)
            return;

        if (first != last)
            right = std::upper_bound(first, last, pos, interpolate.comp);

        position = pos;
    }

    /**
     * @brief Moves the iterator to the first element.
     */
    void jumpToBegin()
    {
        right = first;
        if (right != last) {
            position = right->first;
            ++right;
        }
        else {
            position = key_type();
        }
    }

    /**
     * @brief forward iterates the iterator to the passed position. This position
     * can be any value of the Key-type.
     *
     * This method assumes that the passed position is near the current position
     * of the iterator. If this is the case this method will be faster than the
     * jumpTo-method.
     */
    void iterateTo(key_cref_type pos)
    {
        if (pos == position)
            return;

        while (right != last && !(pos < right->first))
            ++right;

        position = pos;
    }

    /**
     * @brief Iterates to the next entry in the underlying data structure.
     *
     * If the current position is before the position of the first element of the data
     * structure this method will iterate to the first entry.
     * If the current position is after the position of the last element of the data
     * structure this method will increase the current position with the ++ operator.
     */
    void next()
    {
        if (hasNext()) {
            position = right->first;
            ++right;
        }
        else
            position += 1;
    }

    key_type getNextPosition()
    {
        if (hasNext())
            return right->first;
        else
            return position + 1;
    }

    /**
     * @brief Returns true if the current position of the iterator is between the
     * position of the first and the last entry of the data structure.
     */
    bool inRange() const
    {
        if (first == last)
            return false;

        const_iterator tail = last;
        return !(position < first->first) && !((--tail)->first < position);
    }

    /**
     * @brief Returns true if the a call of "next()" would increase to the position
     * of an a valid entry of the data structure. This means if the current position
     * is smaller than position of the last entry.
     */
    bool hasNext() const
    {
        return right != last;
    }

    /**
     * @brief Returns the interpolated value at the current position of the
     * Iterator.
     *
     * See definition of Interpolated on details on the return type.
     */
    interpolated getValue() const
    {
        return interpolate(first, last, position, right);
    }

    interpolated getNextValue() const
    {
        if (right == last) {
            return interpolate(first, last, position + 1, right);
        }
        else {
            const_iterator tmp = right;
            return interpolate(first, last, right->first, ++tmp);
        }
    }

    /**
     * @brief Returns the current position of the iterator.
     */
    key_cref_type getPosition() const
    {
        return position;
    }

    const interpolator_type& getInterpolator() const
    {
        return interpolate;
    }
};

/**
 * @brief Provides an interpolateable iterator for any Container which maps
 * from keys to values which is able to change the underlying Container.
 *
 * The underlying Container has to provide the following things:
 * - a Member "value_type" which defines the type of the Key-Value-pairs
 * - a Member "iterator" which defines the type of the iterator
 * - an "insert"-method with the following Syntax:
 *      Iterator insert(Iterator pos, Container::value_type newEntry)
 *   which returns an iterator pointing to the newly inserted element
 *
 * See ConstInterpolateableIterator for more details.
 *
 * @author Karl Wessel
 * @ingroup mappingDetails
 */
template<typename TInterpolator>
class InterpolateableIterator : public ConstInterpolateableIterator<TInterpolator, typename TInterpolator::iterator>
{
  protected:
    typedef ConstInterpolateableIterator<TInterpolator, typename TInterpolator::iterator>
        base_class_type;
    typedef typename base_class_type::container_type container_type;
    typedef typename container_type::const_iterator const_iterator;
    typedef typename container_type::iterator iterator;
    typedef typename base_class_type::interpolator_type interpolator_type;
    typedef typename base_class_type::key_type key_type;
    typedef typename base_class_type::key_cref_type key_cref_type;
    typedef typename base_class_type::mapped_type mapped_type;
    typedef typename base_class_type::mapped_cref_type mapped_cref_type;
    typedef typename container_type::value_type pair_type;
    typedef typename base_class_type::comparator_type comparator_type;

    /** @brief typedef for the returned Interpolated value of this class.*/
    //typedef Interpolated<V> interpolated;
    typedef typename interpolator_type::interpolated interpolated;

    container_type& cont;

  public:
    InterpolateableIterator(container_type& cont, const interpolator_type& intpl) :
        base_class_type(cont.begin(), cont.end(), intpl), cont(cont) {}

    virtual ~InterpolateableIterator() {}
    /**
     * @brief: Changes (and adds if necessary) the value for the entry at the
     * current position of the iterator to the passed value
     */
    void setValue(mapped_cref_type value)
    {
        //container is empty or position is smaller first entry
        if (this->right == this->first) {
            //insert new entry before first entry and store new entry as new first
            this->first = cont.insert(this->first, std::make_pair(this->position, value));
        }
        else {
            iterator left = this->right;
            --left;
            if (left->first == this->position) {
                left->second = value;
            }
            else {
                cont.insert(this->right, std::make_pair(this->position, value));
            }
        }
    }
};

/**
 * @brief Represents a std::map which is able to interpolate.
 *
 * Returns interpolated values if accessed at position without keys.
 *
 * Used to represent Mappings
 *
 * @author Karl Wessel
 * @sa Mapping
 * @ingroup mappingDetails
 */
template<class TInterpolator, class TContainer = typename TInterpolator::container_type>
class InterpolateableMap : public TContainer
{
  public:
    typedef TContainer container_type;
    typedef TInterpolator interpolator_type;
    typedef container_type base_class_type;
    typedef typename base_class_type::key_type key_type;
    typedef const key_type& key_cref_type;
    typedef typename base_class_type::mapped_type mapped_type;
    typedef typename base_class_type::value_type pair_type;
    typedef const mapped_type& mapped_cref_type;

    /** @brief typedef for the returned Interpolated value of this class.*/
    //typedef Interpolated<V> interpolated;
    typedef typename interpolator_type::interpolated interpolated;
    typedef typename base_class_type::const_iterator const_iterator;
    typedef typename base_class_type::iterator iterator;

    typedef InterpolateableIterator<TInterpolator> iterator_intpl;
    typedef ConstInterpolateableIterator<TInterpolator> const_iterator_intpl;

  protected:

    interpolator_type interpolate;

  public:

    InterpolateableMap() :
        interpolate() {}

    InterpolateableMap(mapped_cref_type oorv) :
        interpolate(oorv) {}

    void setOutOfRangeVal(mapped_cref_type oorv)
    {
        interpolate.setOutOfRangeVal(oorv);
    }

    interpolated getIntplValue(key_cref_type pos) const
    {
        return interpolate(this->begin(), this->end(), pos, this->upper_bound(pos));
    }

    const_iterator_intpl findIntpl(key_cref_type pos) const
    {
        const_iterator_intpl it(this->begin(), this->end(), interpolate);

        it.jumpTo(pos);

        return it;
    }

    const_iterator_intpl beginIntpl() const
    {
        const_iterator_intpl it(this->begin(), this->end(), interpolate);

        return it;
    }

    iterator_intpl findIntpl(key_cref_type pos) __attribute__((noinline))
    {
        iterator_intpl it(*this, interpolate);

        it.jumpTo(pos);

        return it;
    }

    iterator_intpl beginIntpl()
    {
        iterator_intpl it(*this, interpolate);

        return it;
    }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_INTERPOLATION_H

