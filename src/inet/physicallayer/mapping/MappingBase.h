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

#ifndef __INET_MAPPINGBASE_H
#define __INET_MAPPINGBASE_H

#include <omnetpp.h>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <cassert>
#include <sstream>
#include <iomanip>

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/mapping/Interpolation.h"

namespace inet {

namespace physicallayer {

/**
 * @brief Specifies a dimension for mappings (like time, frequency, etc.)
 *
 * The dimension is represented external by a string
 * (like "time") and internally by an unique ID.
 *
 * Note: Since the ID for a Dimensions is set the first time an instance
 * of this dimensions is created and the id is used to provide a
 * defined ordering of the Dimensions it DOES matter which
 * dimensions are instantiated the first time.
 * Only the time dimension will always have zero as unique id.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
class INET_API Dimension
{
  protected:
    /** @brief The unique name of the dimension this instance represents.*/
    const char *name;

    /** @brief The unique id of the dimension this instance represents.*/
    int id;

  public:
    /** @brief Shortcut to the time Dimension, same as 'Dimension("time")',
     * but spares the parsing of a string.*/
    static const Dimension time;

    /** @brief Shortcut to the frequency Dimension, same as 'Dimension("frequency")',
     * but spares the parsing of a string.*/
    static const Dimension frequency;

  public:
    Dimension() :
        id(0)
    {}

    /**
     * @brief Creates a new dimension instance representing the
     * dimension with the passed name.
     */
    Dimension(const char *name, int id);

    /**
     * @brief Returns true if the ids of the two dimensions are equal.
     */
    bool operator==(const Dimension& other) const { return id == other.id; }

    /**
     * @brief Returns true if the id of the other dimension is
     * greater then the id of this dimension.
     *
     * This is needed to be able to use Dimension as a key in std::map.
     */
    bool operator<(const Dimension& other) const { return id < other.id; }

    /** @brief Sorting operator by dimension ID.*/
    bool operator<=(const Dimension& other) const { return id <= other.id; }
    /** @brief Sorting operator by dimension ID.*/
    bool operator>(const Dimension& other) const { return id > other.id; }
    /** @brief Sorting operator by dimension ID.*/
    bool operator>=(const Dimension& other) const { return id >= other.id; }
    /** @brief Sorting operator by dimension ID.*/
    bool operator!=(const Dimension& other) const { return id != other.id; }

    /**
     * @brief Returns the name of this dimension.
     */
    const char *getName() const
    {
        return name;
    }

    /**
     * @brief Returns the unique id of the dimension this instance
     * represents.
     *
     * The id is used to uniquely identify dimensions as well as to
     * provide a sorting of dimensions.
     * Note: The "time"-dimension will always have the ID zero.
     */
    int getID() const { return id; }

    /** @brief Output operator for a dimension.*/
    friend std::ostream& operator<<(std::ostream& out, const Dimension& d)
    {
        return out << d.getName() << "(" << d.id << ")";
    }
};

/**
 * @brief Represents a set of dimensions which is used to define over which
 * dimensions a mapping is defined (the domain of the mapping).
 *
 * This class actually public extends from std::set<Dimension>. So any method
 * provided by std::set can be also used.
 *
 * The dimensions are stored ordered by their ids.
 *
 * Note: Unlike Arguments and Mappings, a DimensionSet does not contain "time"
 * as dimension per default. You'll have to add it like any other dimension.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
class INET_API DimensionSet    /*:public std::set<Dimension>*/
{
  protected:
    typedef std::set<Dimension> _Base;

    _Base Storage;

  public:
    typedef _Base::value_type value_type;
    typedef _Base::iterator iterator;
    typedef _Base::reverse_iterator reverse_iterator;
    typedef _Base::reverse_iterator const_reverse_iterator;
    typedef _Base::const_iterator const_iterator;
    typedef _Base::size_type size_type;
    typedef _Base::const_reference const_reference;

    /** @brief Shortcut to a DimensionSet which only contains time. */
    static const DimensionSet timeDomain;

    /** @brief Shortcut to a DimensionSet which only contains frequency. */
    static const DimensionSet freqDomain;

    /** @brief Shortcut to a DimensionSet which contains time and frequency. */
    static const DimensionSet timeFreqDomain;

  public:
    /**
     * @brief Default constructor creates an empty DimensionSet
     */
    DimensionSet() : Storage() {}

    /**
     * @brief Copy constructor.
     */
    DimensionSet(const DimensionSet& o) : Storage(o.Storage)
    {}

    /**
     *  @brief  %DimensionSet assignment operator.
     *  @param  copy  A %DimensionSet of identical element and allocator types.
     *
     *  All the elements of @a copy are copied.
     */
    DimensionSet& operator=(DimensionSet const& copy)
    {
        DimensionSet tmp(copy);    // All resource all allocation happens here.
                                   // If this fails the copy will throw an exception
                                   // and 'this' object is unaffected by the exception.
        swap(tmp);
        return *this;
    }

    /**
     *  @brief  Swaps data with another %DimensionSet.
     *  @param  s  A %DimensionSet of the same element and allocator types.
     *
     *  This exchanges the elements between two DimensionSet's in constant time.
     *  Note that the global std::swap() function is specialized such that
     *  std::swap(s1,s2) will feed to this function.
     */
    void swap(DimensionSet& s)
    {
        // X::swap(s); // swap the base class members
        /* Swap all D members */
        std::swap(Storage, s.Storage);
    }

    /**
     * @brief Creates a new DimensionSet with the passed Dimension as
     * initial Dimension
     */
    DimensionSet(const DimensionSet::value_type& d) : Storage()
    {
        Storage.insert(d);
    }

    /**
     * @brief Creates a new DimensionSet with the passed Dimensions as
     * initial Dimensions (convenience method)
     */
    DimensionSet(const DimensionSet::value_type& d1, const DimensionSet::value_type& d2) : Storage()
    {
        Storage.insert(d1);
        Storage.insert(d2);
    }

    /**
     * @brief Creates a new DimensionSet with the passed Dimensions as
     * initial Dimensions (convenience method)
     */
    DimensionSet(const DimensionSet::value_type& d1, const DimensionSet::value_type& d2, const DimensionSet::value_type& d3) : Storage()
    {
        Storage.insert(d1);
        Storage.insert(d2);
        Storage.insert(d3);
    }

    /**
     * @brief Returns true if the passed DimensionSet is a subset
     * of this DimensionSet.
     *
     * A DimensionSet is a subset of this DimensionSet if every
     * Dimension of the passed DimensionSet is defined in this
     * DimensionSet.
     */
    bool isSubSet(const DimensionSet& other) const
    {
        if (Storage.size() < other.Storage.size())
            return false;

        return std::includes(Storage.begin(), Storage.end(), other.Storage.begin(), other.Storage.end());
    }

    /**
     * @brief Returns true if the passed DimensionSet is a real subset
     * of this DimensionSet.
     *
     * A DimensionSet is a real subset of this DimensionSet if every
     * Dimension of the passed DimensionSet is defined in this
     * DimensionSet and there is at least one Dimensions in this set
     * which isn't in the other set.
     */
    bool isRealSubSet(const DimensionSet& other) const
    {
        if (size() <= other.size())
            return false;

        return std::includes(begin(), end(), other.begin(), other.end());
    }

    /**
     * @brief Adds the passed dimension to the DimensionSet.
     */
    void addDimension(const DimensionSet::value_type& d)
    {
        Storage.insert(d);
    }

    /**
     * @brief Returns true if the passed Dimension is inside this DimensionSet.
     */
    bool hasDimension(const DimensionSet::value_type& d) const
    {
        return Storage.count(d) > 0;
    }

    /**
     * @brief Returns true if the dimensions of both sets are equal.
     */
    bool operator==(const DimensionSet& o) const
    {
        if (size() != o.size())
            return false;

        return std::equal(begin(), end(), o.begin());
    }

    iterator begin()
    {
        return Storage.begin();
    }

    iterator end()
    {
        return Storage.end();
    }

    const_iterator begin() const
    {
        return Storage.begin();
    }

    const_iterator end() const
    {
        return Storage.end();
    }

    reverse_iterator rbegin()
    {
        return Storage.rbegin();
    }

    reverse_iterator rend()
    {
        return Storage.rend();
    }

    const_reverse_iterator rbegin() const
    {
        return Storage.rbegin();
    }

    const_reverse_iterator rend() const
    {
        return Storage.rend();
    }

    const_iterator find(const value_type& __x) const
    {
        return Storage.find(__x);
    }

    ///  Returns the size of the %DimensionSet.
    size_type size() const
    {
        return Storage.size();
    }

    /**
     *  @brief A template function that attempts to insert a range
     *  of elements.
     *  @param  first  Iterator pointing to the start of the range to be
     *                 inserted.
     *  @param  last  Iterator pointing to the end of the range.
     *
     *  Complexity similar to that of the range constructor.
     */
    template<typename _InputIterator>
    void insert(_InputIterator __first, _InputIterator __last)
    { Storage.insert(__first, __last); }

    /**
     *  @brief Attempts to insert an element into the %DimensionSet.
     *  @param  position  An iterator that serves as a hint as to where the
     *                    element should be inserted.
     *  @param  x  Element to be inserted.
     *  @return  An iterator that points to the element with key of @a x (may
     *           or may not be the element passed in).
     *
     *  This function is not concerned about whether the insertion took place,
     *  and thus does not return a boolean like the single-argument insert()
     *  does.  Note that the first parameter is only a hint and can
     *  potentially improve the performance of the insertion process.  A bad
     *  hint would cause no gains in efficiency.
     *
     *  For more on @a hinting, see:
     *  http://gcc.gnu.org/onlinedocs/libstdc++/manual/bk01pt07ch17.html
     *
     *  Insertion requires logarithmic time (if the hint is not taken).
     */
    iterator insert(const_iterator __position, const value_type& __x)
    {
        return Storage.insert(__position, __x);
    }

    ///  Returns true if the %DimensionSet is empty.
    bool empty() const
    {
        return Storage.empty();
    }
};

/**
 * @brief Defines an argument for a mapping.
 *
 * Defines values for a specified set of dimensions, but at
 * least for the time dimension.
 *
 * Note: Currently an Argument can be maximal defined over ten Dimensions
 * plus the time dimension!
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
class INET_API Argument
{
  public:
    typedef DimensionSet::value_type key_type;
    typedef double mapped_type;
    typedef mapped_type mapped_type_cref;

    /** @brief Zero value of a Argument value. */
    const static mapped_type MappedZero;
    /** @brief One value of a Argument value. */
    const static mapped_type MappedOne;

  protected:
    typedef std::map<key_type, mapped_type> container_type;
    typedef container_type::value_type value_type;
    /** @brief Stores the time dimension in Omnet's time type */
    simtime_t time;

    /** @brief Maps the dimensions of this Argument to their values. */
    container_type values;

  public:
    /** @brief Iterator type for this set.*/
    typedef container_type::iterator iterator;
    /** @brief Const-iterator type for this set.*/
    typedef container_type::const_iterator const_iterator;

  protected:
    /**
     * @brief Inserts the passed value for the passed Dimension into
     * this Argument.
     *
     * The parameter "pos" defines the position inside the Dimension<->Value-pair
     * array to start searching for the dimension to set.
     *
     * If the "ignoreUnknown"-parameter is set to true the new value is only
     * set if the Dimension was defined in this Argument before (means, no
     * new DImensions are added to the Argument).
     *
     * The method returns the position inside the array the value was inserted.
     */
    inline iterator insertValue(iterator pos, const Argument::value_type& valPair, iterator& itEnd, bool ignoreUnknown = false);

    template<typename IteratorType>
    class key_iterator : public IteratorType
    {
      public:
        typedef typename IteratorType::value_type::first_type& reference;
        typedef typename IteratorType::value_type::first_type *pointer;

        key_iterator(const IteratorType& other) : IteratorType(other) {};

        const reference operator*() const
        {
            return IteratorType::operator*().first;
        }

        pointer operator->() const
        {
            return &IteratorType::operator->()->first;
        }
    };

  public:
    /**
     * @brief Initialize this argument with the passed value for
     * the time dimension.
     */
    Argument(simtime_t_cref timeVal = SIMTIME_ZERO);

    /**
     * @brief Initializes the Argument with the dimensions of the
     * passed DimensionSet set to zero, and the passed value for the
     * time (or zero, if omitted).
     */
    Argument(const DimensionSet& dims, simtime_t_cref timeVal = SIMTIME_ZERO);

    /**
     * @brief Returns the time value of this argument.
     */
    simtime_t_cref getTime() const;

    /**
     * @brief Changes the time value of this argument.
     */
    void setTime(simtime_t_cref time);

    /**
     * @brief Returns true if this Argument has a value for the
     * passed Dimension.
     */
    bool hasArgVal(const Argument::key_type& dim) const;

    /**
     * @brief Returns the value for the specified dimension.
     *
     * Note: Don't use this function to get the time value!
     * Use "getTime()" instead.
     *
     * Returns zero if no value with the specified dimension
     * is set for this argument.
     */
    mapped_type_cref getArgValue(const Argument::key_type& dim) const;

    /**
     * @brief Changes the value for the specified dimension.
     *
     * Note: Don't use this function to change the time value!
     * Use "setTime()" instead.
     *
     * If the argument doesn't already contain a value for the
     * specified dimension the new dimension is added.
     */
    void setArgValue(const Argument::key_type& dim, Argument::mapped_type_cref value);

    /**
     * @brief Update the values of this Argument with the values
     * of the passed Argument.
     *
     * Only the dimensions from the passed Argument are updated or
     * added.
     *
     * If the ignoreUnknown parameter is set to true, only the Dimensions
     * already inside the Argument are updated.
     */
    void setArgValues(const Argument& o, bool ignoreUnknown = false);

    /**
     * @brief Returns true if the passed Argument points to
     * the same position.
     *
     * The functions returns true if every Dimension in the passed
     * Argument exists in this Argument and their values are
     * the same. The difference to the == operator is that the
     * dimensions of the passed Argument can be a subset of the
     * dimensions of this Argument.
     */
    bool isSamePosition(const Argument& other) const;

    /**
     * @brief Two Arguments are compared equal if they have
     * the same dimensions and the same values.
     */
    bool operator==(const Argument& o) const;

    /**
     * @brief Two Arguments are compared close if they have
     * the same dimensions and their values don't differ more
     * then a specific epsilon.
     */
    bool isClose(const Argument& o, Argument::mapped_type_cref epsilon = Argument::mapped_type(0.000001)) const;

    /**
     * @brief Returns true if this Argument is smaller then the
     * passed Argument. The dimensions of the Arguments have to
     * be the same.
     *
     * An Argument is compared smaller than another Argument if
     * the value of the Dimension with the highest id is compared
     * smaller.
     * If the value of the highest Dimension is compared bigger the
     * Argument isn't compared smaller (method returns false).
     * If the values of the Dimension with the highest Dimension are
     * equal, the next smaller Dimension is compared.
     */
    bool operator<(const Argument& o) const;

    /**
     * @brief Compares this Argument with the passed Argument in the
     * dimensions of the passed DimensionsSet. (Every other Dimension
     * is asserted equal).
     *
     * @return < 0 - passed Argument is bigger
     *         = 0 - Arguments are equal
     *         > 0 - passed Argument is smaller
     *
     * See "operator<" for definition of smaller, equal and bigger.
     */
    int compare(const Argument& o, const DimensionSet *const dims = NULL) const;

    /**
     * @brief Returns the dimensions this argument is defined over
     *
     * Note: this method has linear complexity over the number of dimensions,
     * since the DimensionSet has to be created from the values and their
     * dimensions inside this Argument.
     */
    DimensionSet getDimensions() const
    {
        typedef key_iterator<const_iterator> key_const_iterator;

        DimensionSet res(Dimension::time);

        res.insert(key_const_iterator(values.begin()), key_const_iterator(values.end()));

        return res;
    }

    /**
     * @brief Output operator for Arguments.
     *
     * Produces output of form "(x1, x2, x3, <...>)".
     */
    friend std::ostream& operator<<(std::ostream& out, const Argument& d)
    {
        using std::operator<<;

        out << "(" << d.time;

        for (const_iterator it = d.begin(); it != d.end(); ++it) {
            out << ", " << it->second << "@" << it->first;
        }
        return out << ")";
    }

    /**
     * @brief Fast implementation of the copy-operator then the default
     * implementation.
     */
    Argument& operator=(const Argument& o);

    /**
     * @brief Returns an iterator to the first argument value in this Argument.
     */
    iterator begin() { return values.begin(); }
    /**
     * @brief Returns an iterator to the first argument value in this Argument.
     */
    const_iterator begin() const { return values.begin(); }

    /**
     * @brief Returns an iterator to the value behind the last argument value.
     */
    iterator end() { return values.end(); }
    /**
     * @brief Returns an iterator to the value behind the last argument value.
     */
    const_iterator end() const { return values.end(); }

    /**
     * @brief Returns an iterator to the Argument value for the passed Dimension.
     *
     * Returns end() if there is no Argument for that dimension.
     */
    iterator find(const Argument::key_type& dim);
    /**
     * @brief Returns an iterator to the Argument value for the passed Dimension.
     *
     * Returns end() if there is no Argument for that dimension.
     */
    const_iterator find(const Argument::key_type& dim) const;

    /**
     * @brief Returns an iterator to the first Argument value which dimension
     * compares greater or equal to the passed Dimension.
     */
    iterator lower_bound(const Argument::key_type& dim);
    /**
     * @brief Returns an iterator to the first Argument value which dimension
     * compares greater or equal to the passed Dimension.
     */
    const_iterator lower_bound(const Argument::key_type& dim) const;
};

/**
 * @brief This exception is thrown by the MappingIterators when "next()" or "nextPosition()" is called
 * although "hasNext()" would return false (means there is no next position).
 *
 * Although this exception isn't thrown by every implementation of the "next()"-method it is always
 * a bad idea to call "next()" although there isn't any next position.
 * You should check "hasNext()" before calling "next()" or "nextPosition()".
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
class NoNextIteratorException
{
};

/**
 * @brief Defines an const iterator for a ConstMapping which is able
 * to iterate over the Mapping.
 *
 * Iterators provide a fast way to iterate over every "entry" or important
 * position of a Mapping. They also provide constant complexity for accessing
 * the Mapping at the position of the Iterator.
 *
 * Every implementation of an mapping-Iterator provides the same ordering
 * when iterating over a Mapping. This is that, if the iterator is increased,
 * the position of the entry it points to has to be also "increased". Which
 * means that the new position has to be compared bigger than the previous
 * position (see class Argument for comparison of positions).
 *
 * "Const" means that you can not change the values of the underlying Mapping
 * with this iterator.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
class INET_API ConstMappingIterator
{
  public:
    typedef Argument::mapped_type argument_value_t;
    typedef Argument::mapped_type_cref argument_value_cref_t;

  public:
    ConstMappingIterator() {}
    virtual ~ConstMappingIterator() {}

    /**
     * @brief Copy constructor.
     */
    ConstMappingIterator(const ConstMappingIterator&) {}

    /**
     *  @brief  %ConstMappingIterator assignment operator.
     *  @param  copy  A %ConstMappingIterator of identical element and allocator types.
     *
     *  All the elements of @a copy are copied.
     */
    ConstMappingIterator& operator=(const ConstMappingIterator&) { return *this; }

    /**
     *  @brief  Swaps data with another %ConstMappingIterator.
     *  @param  s  A %ConstMappingIterator of the same element and allocator types.
     *
     *  This exchanges the elements between two ConstMappingIterator's in constant time.
     *  Note that the global std::swap() function is specialized such that
     *  std::swap(s1,s2) will feed to this function.
     */
    void swap(ConstMappingIterator&) {}

    /**
     * @brief Returns the position the next call to "next()" of this
     * Iterator would iterate to.
     */
    virtual const Argument& getNextPosition() const = 0;

    /**
     * @brief Lets the iterator point to the passed position.
     *
     * The passed new position can be at arbitrary places.
     */
    virtual void jumpTo(const Argument& pos) = 0;

    /**
     * @brief Lets the iterator point to the begin of the mapping.
     *
     * The beginning of the mapping depends on the implementation. With an
     * implementation based on a number of key-entries, this could be the
     * key entry with the smallest position (see class Argument for ordering
     * of positions).
     */
    virtual void jumpToBegin() = 0;

    /**
     * @brief Iterates to the specified position. This method
     * should be used if the new position is near the current position.
     * Furthermore the new position has to be compared bigger than
     * the old position.
     */
    virtual void iterateTo(const Argument& pos) = 0;

    /**
     * @brief Iterates to the next position of the Mapping.
     *
     * The next position depends on the implementation of the
     * Mapping. With an implementation based on a number of key-entries
     * this probably would be the next bigger key-entry.
     */
    virtual void next() = 0;

    /**
     * @brief Returns true if the current position of the iterator
     * is in range of the function.
     *
     * This method should be used as end-condition when iterating
     * over the function with the "next()" method.
     */
    virtual bool inRange() const = 0;

    /**
     * @brief Returns true if the iterator has a next value it
     * can iterate to on a call to "next()".
     */
    virtual bool hasNext() const = 0;

    /**
     * @brief Returns the current position of the iterator.
     */
    virtual const Argument& getPosition() const = 0;

    /**
     * @brief Returns the value of the Mapping at the current
     * position.
     *
     * The complexity of this method should be constant for every
     * implementation.
     */
    virtual argument_value_t getValue() const = 0;
};

class Mapping;

namespace mixim {

namespace math {

template<typename T, bool B = std::numeric_limits<T>::has_infinity>
struct mW2dBm
{
};

template<typename T>
struct mW2dBm<T, false>
{
    inline T operator()(const T& mW)
    {
        return 10 * log10(mW);
    }
};

template<typename T>
struct mW2dBm<T, true>
{
    inline T operator()(const T& mW)
    {
        typedef std::numeric_limits<T> tnumlimits_for_v;
        // we have possible infinity values, so that we can do some checks
        const T cInf = tnumlimits_for_v::infinity();
        const bool bIsInf = (mW == cInf) || (tnumlimits_for_v::is_signed ? (mW == -cInf) : false);

        if (bIsInf) {
            return mW;
        }
        return 10 * log10(mW);
    }
};
};
};

/**
 * @brief Represents a not changeable mapping (mathematical function)
 * from domain with at least the time to  a Argument::mapped_type value.
 *
 * This class is an interface which describes a mapping (math.)
 * from a arbitrary dimensional domain (represented by a DimensionSet)
 * to a Argument::mapped_type value.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
class INET_API ConstMapping
{
  public:
    typedef Argument::mapped_type argument_value_t;
    typedef Argument::mapped_type_cref argument_value_cref_t;

  protected:
    /** @brief The dimensions of this mappings domain.*/
    DimensionSet dimensions;

  public:
    /**
     *  @brief  %ConstMapping assignment operator.
     *  @param  copy  A %ConstMapping of identical element and allocator types.
     *
     *  All the elements of @a copy are copied.
     */
    ConstMapping& operator=(const ConstMapping& copy)
    {
        dimensions = copy.dimensions;
        return *this;
    }

    /**
     *  @brief  Swaps data with another %ConstMapping.
     *  @param  s  A %ConstMapping of the same element and allocator types.
     *
     *  This exchanges the elements between two DimensionSet's in constant time.
     *  Note that the global std::swap() function is specialized such that
     *  std::swap(s1,s2) will feed to this function.
     */
    void swap(ConstMapping& s)
    {
        //::swap(s); // swap the base class members
        /* Swap all D members */
        dimensions.swap(s.dimensions);
    }

  private:
    template<class T>
    std::string toString(T v, unsigned int length) const
    {
        std::stringstream osToStr(std::stringstream::out);

        using std::operator<<;

        osToStr << std::fixed;
        osToStr.precision(16);
        osToStr.width(length);
        osToStr << std::right << v;

        return osToStr.str();
    }

    std::string toString(simtime_t_cref v, unsigned int length) const
    {
        return toString(SIMTIME_DBL(v), length);
    }

  public:

    /**
     * @brief Initializes the ConstMapping with a the time dimension as domain.
     */
    ConstMapping() :
        dimensions(Dimension::time) {}
    ConstMapping(const ConstMapping& o) :
        dimensions(o.dimensions) {}

    /**
     * @brief Initializes the ConstMapping with the passed DimensionSet as
     * Domain.
     *
     * The passed DimensionSet has to contain the time dimension!
     */
    ConstMapping(const DimensionSet& dimSet) :
        dimensions(dimSet)
    {
//        assert(dimSet.hasDimension(Dimension::time));
    }

    virtual ~ConstMapping() {}

    /**
     * @brief Returns the value of this Mapping at the position specified
     * by the passed Argument.
     *
     * The complexity of this method depends on the actual implementation.
     */
    virtual argument_value_t getValue(const Argument& pos) const = 0;

    /**
     * @brief Returns a pointer of a new Iterator which is able to iterate
     * over this Mapping.
     *
     * See class ConstIterator for details.
     */
    virtual ConstMappingIterator *createConstIterator() const = 0;

    /**
     * @brief Returns a pointer of a new Iterator which is able to iterate
     * over the function. The iterator starts at the passed position.
     *
     * See class ConstIterator for details.
     */
    virtual ConstMappingIterator *createConstIterator(const Argument& pos) const = 0;

    /**
     * @brief returns a deep copy of this mapping instance.
     */
    virtual ConstMapping *constClone() const = 0;

    /**
     * @brief Returns the value of this Mapping at the position specified
     * by the passed Argument.
     */
    argument_value_t operator[](const Argument& pos) const
    {
        return getValue(pos);
    }

    /**
     * @brief Returns this Mappings domain as DimensionSet.
     */
    const DimensionSet& getDimensionSet() const
    {
        return dimensions;
    }

    /**
     * @brief Prints the Mapping to an output stream.
     *
     * @param out           The output stream to print.
     * @param lTimeScale    The scaling factor for the time header values (default 1000).
     * @param lLeftColScale The scaling factor for the left column values (default 1, unscaled).
     * @param sTableHead    The header string for the left column (default: "o\\ms").
     * @param pOnlyDim      Pointer to a specific dimension which should be used as the left column (should not be the Dimension::time).
     */
    template<class stream>
    stream& print(stream& out,
            argument_value_cref_t lTimeScale = argument_value_t(1),
            argument_value_cref_t lLeftColScale = Argument::MappedOne,
            const std::string& sTableHead = std::string("o\\ms"),
            const Dimension *const pOnlyDim = NULL) const
    {
        const ConstMapping& m = *this;
        DimensionSet::value_type otherDim;
        const DimensionSet& dims = m.getDimensionSet();
        bool bOnlyDimFound = false;

        using std::operator<<;

        std::stringstream osDimHead(std::stringstream::out);
        bool bTimeIsIn = false;
        const DimensionSet::iterator dimsEnd = dims.end();
        for (DimensionSet::iterator it = dims.begin(); it != dimsEnd; ++it) {
            if (*it != Dimension::time) {
                if (pOnlyDim && *it == *pOnlyDim) {
                    otherDim = *it;
                    bOnlyDimFound = pOnlyDim != NULL;
                }
                else if (!pOnlyDim) {
                    otherDim = *it;
                }
                if (!osDimHead.str().empty())
                    osDimHead << ", ";
                osDimHead << *it;
            }
            else
                bTimeIsIn = true;
        }
        if (bTimeIsIn || !osDimHead.str().empty()) {
            out << "Mapping domain: ";
            if (bTimeIsIn) {
                out << "time";
                if (!osDimHead.str().empty())
                    out << ", ";
            }
            out << osDimHead.str() << endl;
        }

        ConstMappingIterator *it = m.createConstIterator();

        if (!it->inRange()) {
            out << "Mapping is empty." << endl;
            return out;
        }

        Argument min = it->getPosition();
        Argument max = it->getPosition();

        typedef std::set<simtime_t> t_time_container_type;
        typedef std::set<argument_value_t> t_value_container_type;

        t_time_container_type timePositions;
        t_value_container_type otherPositions;
        std::size_t iMaxHeaderItemLen = m.toString(it->getPosition().getTime() * lTimeScale, 2).length();
        std::size_t iMaxLeftItemLen = 5 > sTableHead.length() ? 5 : sTableHead.length();
        const bool bIs2Dim = dims.size() == 2;

        if (bIs2Dim && bOnlyDimFound)
            bOnlyDimFound = false; // we have only the time and the requested dimension (fallback to normal case)
        timePositions.insert(it->getPosition().getTime());
        if (bIs2Dim || bOnlyDimFound) {
            Argument::const_iterator posValIt;
            if (bOnlyDimFound && (posValIt = it->getPosition().find(otherDim)) != it->getPosition().end()) {
                otherPositions.insert(it->getPosition().getArgValue(otherDim));
                iMaxLeftItemLen = std::max(iMaxLeftItemLen, m.toString(posValIt->second * lLeftColScale, 2).length());
            }
            else if (!bOnlyDimFound) {
                otherPositions.insert(it->getPosition().begin()->second);
                iMaxLeftItemLen = std::max(iMaxLeftItemLen, m.toString(it->getPosition().begin()->second * lLeftColScale, 2).length());
            }
        }

        while (it->hasNext()) {
            it->next();
            const Argument& pos = it->getPosition();
            const Argument::const_iterator posValItEnd = pos.end();
            Argument::const_iterator posValIt;

            min.setTime(std::min(min.getTime(), pos.getTime()));
            max.setTime(std::max(max.getTime(), pos.getTime()));

            timePositions.insert(pos.getTime());

            iMaxHeaderItemLen = std::max(iMaxHeaderItemLen, m.toString(pos.getTime() * lTimeScale, 2).length());

            if (bOnlyDimFound && (posValIt = pos.find(otherDim)) != posValItEnd) {
                iMaxLeftItemLen = std::max(iMaxLeftItemLen, m.toString(posValIt->second * lLeftColScale, 2).length());
            }
            for (Argument::const_iterator itA = pos.begin(); itA != posValItEnd; ++itA) {
                if (bIs2Dim || bOnlyDimFound) {
                    if (!bOnlyDimFound || itA->first == otherDim) {
                        otherPositions.insert(itA->second);
                        iMaxLeftItemLen = std::max(iMaxLeftItemLen, m.toString(itA->second * lLeftColScale, 2).length());
                    }
                }
                min.setArgValue(itA->first, std::min(min.getArgValue(itA->first), itA->second));
                max.setArgValue(itA->first, std::max(max.getArgValue(itA->first), itA->second));
            }
        }
        delete it;

        if (!bIs2Dim && !bOnlyDimFound) {
            if (!bOnlyDimFound && pOnlyDim != NULL) {
                out << "map contains no " << pOnlyDim->getName() << " dimension!" << endl;
                return out;
            }
            else
                out << "domain - min=" << min << " max=" << max << endl;
        }
        if (bOnlyDimFound && otherPositions.empty()) {
            out << "Defines no own key entries for " << pOnlyDim->getName() << " dimension! That does NOT mean it doesn't define any attenuation." << endl;
            return out;
        }

        t_time_container_type::const_iterator tIt;
        const t_time_container_type::const_iterator tItEnd = timePositions.end();
        std::stringstream osBorder(std::stringstream::out);
        std::stringstream osHeader(std::stringstream::out);
        mixim::math::mW2dBm<argument_value_t> fctor2dBm;

        for (tIt = timePositions.begin(); tIt != tItEnd; ++tIt) {
            osHeader << m.toString(*tIt * lTimeScale, iMaxHeaderItemLen) << osHeader.fill();
        }
        osBorder.fill('-');
        osBorder << std::setw(iMaxLeftItemLen) << "" << osBorder.fill() << "+" << osBorder.fill() << std::setw(osHeader.str().length()) << "";

        out << osBorder.str() << std::endl;
        out << std::setw(iMaxLeftItemLen) << std::left << sTableHead << out.fill() << "|" << out.fill();
        out << osHeader.str() << std::endl;
        out << osBorder.str() << std::endl;

        it = m.createConstIterator();

        if (dims.size() == 1) {
            out << std::setw(iMaxLeftItemLen) << "value" << out.fill() << "|" << out.fill();
            while (it->inRange()) {
                out << m.toString(fctor2dBm(it->getValue()), iMaxHeaderItemLen) << out.fill();
                if (!it->hasNext()) {
                    break;
                }
                it->next();
            }
        }
        else {
            t_value_container_type::const_iterator fIt = otherPositions.begin();

            tIt = timePositions.begin();
            out << m.toString((*fIt) * lLeftColScale, iMaxLeftItemLen) << out.fill() << "|" << out.fill();
            while (it->inRange()) {
                if (*fIt != it->getPosition().getArgValue(otherDim)) {
                    ++fIt;
                    out << std::endl << m.toString((*fIt) * lLeftColScale, iMaxLeftItemLen) << out.fill() << "|" << out.fill();
                    tIt = timePositions.begin();
                    assert(*fIt == it->getPosition().getArgValue(otherDim));
                }

                while (tIt != tItEnd && *tIt < it->getPosition().getTime()) {
                    // blank item because the header time does not match
                    ++tIt;
                    out << std::setw(iMaxHeaderItemLen + 1) << "";
                }
                if (tIt != tItEnd) {
                    // jump to next header item
                    ++tIt;
                }

                out << m.toString(fctor2dBm(it->getValue()), iMaxHeaderItemLen) << out.fill();

                if (!it->hasNext()) {
                    break;
                }
                it->next();
            }
        }
        out << std::endl << osBorder.str() << std::endl;
        return out;
    }

    friend std::ostream& operator<<(std::ostream& out, const ConstMapping& rMapToPrint)
    {
        return rMapToPrint.print(out);
    }
};

/**
 * @brief Defines an iterator for a Mapping which is able
 * to iterate over the Mapping.
 *
 * See class ConstMapping for details on Mapping iterators.
 *
 * Implementations of this class are able to change the underlying
 * Mapping at the current position of the iterator with constant
 * complexity.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
class INET_API MappingIterator : public ConstMappingIterator
{
  public:
    MappingIterator() : ConstMappingIterator() {}

    virtual ~MappingIterator() {}
    /**
     * @brief Changes the value of the Mapping at the current
     * position.
     *
     * Implementations of this method should provide constant
     * complexity.
     */
    virtual void setValue(argument_value_cref_t value) = 0;
};

/**
 * @brief Represents a changeable mapping (mathematical function)
 * from at least time to Argument::mapped_type.
 *
 * This class extends the ConstMapping interface with write access.
 *
 * See ConstMapping for details.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
class INET_API Mapping : public ConstMapping
{
  public:
    /** @brief Types of interpolation methods for mappings.*/
    enum InterpolationMethod {
        /** @brief interpolates with next lower entry*/
        STEPS,
        /** @brief interpolates with nearest entry*/
        NEAREST,
        /** @brief interpolates linear with next lower and next upper entry
                   constant before the first and after the last entry*/
        LINEAR
    };

  public:
    /**
     *  @brief  %Mapping assignment operator.
     *  @param  copy  A %Mapping of identical element and allocator types.
     *
     *  All the elements of @a copy are copied.
     */
    Mapping& operator=(const Mapping& copy)
    {
        ConstMapping::operator=(copy);
        return *this;
    }

    /**
     *  @brief  Swaps data with another %Mapping.
     *  @param  s  A %Mapping of the same element and allocator types.
     *
     *  This exchanges the elements between two DimensionSet's in constant time.
     *  Note that the global std::swap() function is specialized such that
     *  std::swap(s1,s2) will feed to this function.
     */
    void swap(Mapping& s)
    {
        ConstMapping::swap(s);    // swap the base class members
        /* Swap all D members */
    }

  public:

    /**
     * @brief Initializes the Mapping with the passed DimensionSet as domain.
     *
     * The passed DimensionSet has to contain the time dimension!
     */
    Mapping(const DimensionSet& dims) :
        ConstMapping(dims) {}

    /**
     * @brief Initializes the Mapping with the time dimension as domain.
     */
    Mapping() :
        ConstMapping() {}

    Mapping(const Mapping& o) :
        ConstMapping(o) {}

    virtual ~Mapping() {}

    /**
     * @brief Changes the value of the Mapping at the specified
     * position.
     *
     * The complexity of this method depends on the implementation.
     */
    virtual void setValue(const Argument& pos, argument_value_cref_t value) = 0;

    /**
     * @brief Appends the passed value at the passed position to the mapping.
     * This method assumes that the passed position is after the last key entry
     * of the mapping.
     *
     * Depending on the implementation of the underlying method this method
     * could be faster or at least as fast as the "setValue()"-method.
     *
     * Implementations of the Mapping interface can override this method
     * if appending values to the end of the mapping could be implemented
     * faster than the "setValue()" method. Otherwise this method just
     * calls the "setValue()"-method implementation.
     */
    virtual void appendValue(const Argument& pos, argument_value_cref_t value)
    {
        setValue(pos, value);
    }

    /**
     * @brief Returns a pointer of a new Iterator which is able to iterate
     * over the Mapping and can change it.
     */
    virtual MappingIterator *createIterator() = 0;

    /**
     * @brief Returns a pointer of a new Iterator which is able to iterate
     * over the function and can change it.
     *
     * The iterator starts at the passed position.
     */
    virtual MappingIterator *createIterator(const Argument& pos) = 0;

    /**
     * @brief Returns an ConstMappingIterator by use of the respective implementation
     * of the "createIterator()"-method.
     *
     * Override this method if your ConstIterator differs from the normal iterator.
     */
    virtual ConstMappingIterator *createConstIterator() const
    {
        return dynamic_cast<ConstMappingIterator *>(const_cast<Mapping *>(this)->createIterator());
    }

    /**
     * @brief Returns an ConstMappingIterator by use of the respective implementation
     * of the "createIterator()"-method.
     *
     * Override this method if your ConstIterator differs from the normal iterator.
     */
    virtual ConstMappingIterator *createConstIterator(const Argument& pos) const
    {
        return dynamic_cast<ConstMappingIterator *>(const_cast<Mapping *>(this)->createIterator(pos));
    }

    /**
     * @brief Returns a deep copy of this Mapping.
     */
    virtual Mapping *clone() const = 0;

    /**
     * @brief Returns a deep const copy of this mapping by using
     * the according "clone()"-implementation.
     */
    virtual ConstMapping *constClone() const { return clone(); }
};

//###################################################################################
//#                     default Mapping implementations                              #
//###################################################################################

/**
 * @brief A fully working ConstIterator-implementation usable with almost every
 * ConstMapping.
 *
 * Although this ConstIterator would work with almost any ConstMapping it should
 * only be used for ConstMappings whose "getValue()"-method has constant complexity.
 * This is because the iterator just calls the "getValue()"-method of the
 * underlying ConstMapping on every call of its own "getValue()"-method.
 *
 * The underlying ConstMapping has to provide a set of key-entries (Arguments) to the
 * iterator to tell it the positions it should iterate over.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
class INET_API SimpleConstMappingIterator : public ConstMappingIterator
{
  protected:
    /** @brief Type for a set of Arguments defining key entries.*/
    typedef std::set<Argument> KeyEntrySet;
    /** @brief Type of a key entries item.*/
    typedef KeyEntrySet::value_type KeyEntryType;

    /** @brief The underlying ConstMapping to iterate over. */
    const ConstMapping *const mapping;

    /** @brief The dimensions of the underlying ConstMapping.*/
    const DimensionSet& dimensions;

    /** @brief The current position of the iterator.*/
    KeyEntryType position;

    /** @brief A pointer to a set of Arguments defining the positions to
     * iterate over.
     */
    const KeyEntrySet *keyEntries;

    /** @brief An iterator over the key entry set which defines the next bigger
     * entry of the current position.*/
    KeyEntrySet::const_iterator nextEntry;

  private:
    /** @brief Copy constructor is not allowed.
     */
    SimpleConstMappingIterator(const SimpleConstMappingIterator&);
    /** @brief Assignment operator is not allowed.
     */
    SimpleConstMappingIterator& operator=(const SimpleConstMappingIterator&);

  public:
    /**
     * @brief Initializes the ConstIterator for the passed ConstMapping,
     * with the passed key entries to iterate over and the passed position
     * as start.
     *
     * Note: The pointer to the key entries has to be valid as long as the
     * iterator exists.
     */
    SimpleConstMappingIterator(const ConstMapping *mapping,
            const SimpleConstMappingIterator::KeyEntrySet *keyEntries,
            const Argument& start);

    /**
     * @brief Initializes the ConstIterator for the passed ConstMapping,
     * with the passed key entries to iterate over.
     *
     * Note: The pointer to the key entries has to be valid as long as the
     * iterator exists.
     */
    SimpleConstMappingIterator(const ConstMapping *mapping,
            const SimpleConstMappingIterator::KeyEntrySet *keyEntries);

    /**
     * @brief Returns the next position a call to "next()" would iterate to.
     *
     * This method has constant complexity.
     *
     * Throws an NoNextIteratorException if there is no next point of interest.
     */
    virtual const Argument& getNextPosition() const
    {
        if (nextEntry == keyEntries->end())
            throw NoNextIteratorException();
        return *nextEntry;
    }

    /**
     * @brief Lets the iterator point to the passed position.
     *
     * This method has logarithmic complexity (over the number of key entries).
     */
    virtual void jumpTo(const Argument& pos)
    {
        position.setArgValues(pos, true);
        nextEntry = keyEntries->upper_bound(position);
    }

    /**
     * @brief Lets the iterator point to the first "position of interest"
     * of the underlying mapping.
     *
     * This method has constant complexity.
     */
    virtual void jumpToBegin()
    {
        nextEntry = keyEntries->begin();

        if (nextEntry == keyEntries->end()) {
            position = Argument(dimensions);
        }
        else {
            position = *nextEntry;
            ++nextEntry;
        }
    }

    /**
     * @brief Increases the position of the iterator to the passed
     * position.
     *
     * The passed position has to be compared greater than the previous
     * position.
     *
     * This method has constant complexity.
     */
    virtual void iterateTo(const Argument& pos)
    {
        position.setArgValues(pos, true);
        const KeyEntrySet::const_iterator keyEntriesEnd = keyEntries->end();
        while (nextEntry != keyEntriesEnd && !(position < *nextEntry))
            ++nextEntry;
    }

    /**
     * @brief Iterates to the next "point of interest" of the Mapping.
     *
     * Throws an NoNextIteratorException if there is no next point of interest.
     *
     * This method has constant complexity.
     */
    virtual void next()
    {
        if (nextEntry == keyEntries->end())
            throw NoNextIteratorException();

        position = *nextEntry;
        ++nextEntry;
    }

    /**
     * @brief Returns true if the current position of the iterator is equal or bigger
     * than the first point of interest and lower or equal than the last point of
     * interest.
     *
     * Has constant complexity.
     */
    virtual bool inRange() const
    {
        return !(keyEntries->empty()
                 || (*(keyEntries->rbegin()) < position)
                 || (position < *(keyEntries->begin())));
    }

    /**
     * @brief Returns true if there is a next position a call to "next()" can iterate to.
     *
     * Has constant complexity.
     */
    virtual bool hasNext() const { return nextEntry != keyEntries->end(); }

    /**
     * @brief Returns the current position of the iterator.
     *
     * Constant complexity.
     */
    virtual const Argument& getPosition() const { return position; }

    /**
     * @brief Returns the value of the underlying mapping at the current
     * position of the iterator.
     *
     * This method has the same complexity as the "getValue()" method of the
     * underlying mapping.
     */
    virtual argument_value_t getValue() const { return mapping->getValue(position); }
};

/**
 * @brief Abstract subclass of ConstMapping which can be used as base for
 * any ConstMapping implementation with read access of constant complexity.
 *
 * Any subclass only has to implement the "getValue()" and the "clone()"-method.
 *
 * The subclass has to define the "points of interest" an iterator should
 * iterate over.
 * This should be done either at construction time or later by calling the
 * "initializeArguments()"-method.
 *
 * The SimpleConstMapping class provides Iterator creation by using the
 * SimpleConstMappingIterator which assumes that the underlying ConstMappings
 * getValue()-method is fast enough to be called on every iteration step
 * (which means constant complexity).
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
class INET_API SimpleConstMapping : public ConstMapping
{
  protected:
    /** @brief Type for a set of Arguments defining key entries.*/
    typedef std::set<Argument> KeyEntrySet;

    /** @brief A set of Arguments defining the "points of interest" an iterator
     * should iterate over.*/
    KeyEntrySet keyEntries;

  protected:

    /**
     * @brief Utility method to fill add range of key entries in the time dimension
     * to the key entry set.
     */
    void createKeyEntries(const Argument& from, const Argument& to, const Argument& step, Argument& pos);

    /**
     * @brief Utility method to fill add range of key entries in the passed dimension
     * (and recursively its sub dimensions) to the key entry set.
     */
    void createKeyEntries(const Argument& from, const Argument& to, const Argument& step,
            DimensionSet::const_iterator curDim, Argument& pos);

  public:
    /**
     * @brief Returns a pointer of a new Iterator which is able to iterate
     * over the Mapping.
     *
     * This method asserts that the mapping had been fully initialized.
     */
    virtual ConstMappingIterator *createConstIterator() const
    {
        return new SimpleConstMappingIterator(this, &keyEntries);
    }

    /**
     * @brief Returns a pointer of a new Iterator which is able to iterate
     * over the Mapping. The iterator starts at the passed position.
     *
     * This method asserts that the mapping had been fully initialized.
     */
    virtual ConstMappingIterator *createConstIterator(const Argument& pos) const
    {
        return new SimpleConstMappingIterator(this, &keyEntries, pos);
    }

  public:
    /**
     * @brief Initializes a not yet iterateable SimpleConstmapping with the
     * passed DimensionSet as domain.
     */
    SimpleConstMapping(const DimensionSet& dims) :
        ConstMapping(dims), keyEntries() {}
    SimpleConstMapping(const SimpleConstMapping& o) :
        ConstMapping(o), keyEntries(o.keyEntries) {}

    /**
     *  @brief  %SimpleConstMapping assignment operator.
     *  @param  copy  A %SimpleConstMapping of identical element and allocator types.
     *
     *  All the elements of @a copy are copied.
     */
    SimpleConstMapping& operator=(const SimpleConstMapping& copy)
    {
        ConstMapping::operator=(copy);
        return *this;
    }

    /**
     *  @brief  Swaps data with another %SimpleConstMapping.
     *  @param  s  A %SimpleConstMapping of the same element and allocator types.
     *
     *  This exchanges the elements between two SimpleConstMapping's in constant time.
     *  Note that the global std::swap() function is specialized such that
     *  std::swap(s1,s2) will feed to this function.
     */
    void swap(SimpleConstMapping& s) { ConstMapping::swap(s);    /* swap the base class members */ }

    /**
     * @brief Fully initializes this mapping with the passed position
     * as key entry.
     *
     * A SimpleConstMapping initialized by this constructor is able to return a valid
     * ConstIterator.
     */
    SimpleConstMapping(const DimensionSet& dims,
            const Argument& key) :
        ConstMapping(dims), keyEntries()
    {
        initializeArguments(key);
    }

    /**
     * @brief Fully initializes this mapping with the passed two positions
     * as key entries.
     *
     * A SimpleConstMapping initialized by this constructor is able to return a valid
     * ConstIterator.
     */
    SimpleConstMapping(const DimensionSet& dims,
            const Argument& key1, const Argument& key2) :
        ConstMapping(dims), keyEntries()
    {
        initializeArguments(key1, key2);
    }

    /**
     * @brief Fully initializes this mapping with the key entries defined by
     * the passed min, max and interval values.
     *
     * A SimpleConstMapping initialized by this constructor is able to return a valid
     * ConstIterator.
     */
    SimpleConstMapping(const DimensionSet& dims,
            const Argument& min, const Argument& max, const Argument& interval) :
        ConstMapping(dims), keyEntries()
    {
        initializeArguments(min, max, interval);
    }

    /**
     * @brief Initializes the key entry set with the passed position
     * as entry.
     *
     * Convenience method for simple mappings.
     */
    void initializeArguments(const Argument& key)
    {
        keyEntries.clear();
        keyEntries.insert(key);
    }

    /**
     * @brief Initializes the key entry set with the passed two positions
     * as entries.
     *
     * Convenience method for simple mappings.
     */
    void initializeArguments(const Argument& key1, const Argument& key2)
    {
        keyEntries.clear();
        keyEntries.insert(key1);
        keyEntries.insert(key2);
    }

    /**
     * @brief Initializes the key entry set with the passed min, max and
     * interval-Arguments.
     *
     * After a call to this method this SimpleConstMapping is able to return a valid
     * ConstIterator.
     */
    void initializeArguments(const Argument& min,
            const Argument& max,
            const Argument& interval);

    /**
     * @brief Returns the value of the mapping at the passed position.
     *
     * This method has to be implemented by every subclass and should
     * only have constant complexity.
     */
    virtual argument_value_t getValue(const Argument& pos) const = 0;

    /**
     * @brief creates a clone of this mapping.
     *
     * This method has to be implemented by every subclass.
     */
    ConstMapping *constClone() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MAPPINGBASE_H

