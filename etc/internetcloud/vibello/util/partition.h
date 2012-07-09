/* 
 * File:   partition.h
 * Author: archmage
 *
 * Created on January 13, 2011, 12:44 PM
 */

#ifndef VIBELLO_UTIL_PARTITION_H
#define	VIBELLO_UTIL_PARTITION_H

#include <vector>
#include <queue>
#include <iostream>

namespace vibello {
namespace util {

// Used for sorting workpackages in priority queue so that smallest is atop
template<class wp_t>
struct wp_comparator : public std::binary_function<wp_t, wp_t, bool>
{
    bool operator() (const wp_t& x,
                     const wp_t&y) const
    {
        return x.get_effort()>y.get_effort();
    }
};

// Workpackage, contains elems and a effort estimate
template<typename elem_t, typename effort_t, typename estimator_t>
struct wp : std::vector<elem_t>
{
    inline wp() : effort(effort_t()) { }
    effort_t effort;

    inline void push(const elem_t&e)
    {
        push_back(e);
        effort+=estimator_t()(e);
    }

    inline effort_t get_effort() const
    {
        return effort;
    }
};

// Comparator for sorting elems by effort, biggest front
template<typename estimator_t>
struct effort_comparator
: public std::binary_function<typename estimator_t::argument_type, typename estimator_t::argument_type, bool>
{

    bool operator() (typename estimator_t::argument_type const& x,
                     typename estimator_t::argument_type const& y) const
    {
        return estimator_t()(x)>estimator_t()(y);
    }
};

/**
 * Partition elems in range into equal work packages according to effort estimator.
 * The second range will hold the end iterators of resulting work packages.
 * Its length determines the number of work packages.
 */
template <class RandomAccessIterator, class RandomAccessIterator2, typename estimator_t>
inline void partition(RandomAccessIterator elems_begin, RandomAccessIterator elems_end,
                      RandomAccessIterator2 partitions_begin, RandomAccessIterator2 partitions_end)
{
    typedef typename RandomAccessIterator::value_type elem_t;
    typedef typename estimator_t::result_type effort_t;
    typedef wp<elem_t, effort_t, estimator_t> wp_t;
    std::priority_queue<wp_t, std::vector<wp_t>, wp_comparator<wp_t> > wps;

    // Sort elements, so big ones are at the beginning
    sort(elems_begin, elems_end, effort_comparator<estimator_t>());

    // Create work packages
    for (RandomAccessIterator2 i=partitions_begin; i!=partitions_end; ++i)
        wps.push(wp_t());

    // Distribute elements among work packages
    for (RandomAccessIterator i=elems_begin; i!=elems_end; ++i)
    {
        //std::cout<<estimator_t()(*i)<<" ";
        wp_t wp=wps.top(); // TODO: optimize
        wps.pop();
        wp.push(*i);
        wps.push(wp);
    }

    // Copy work packages back into elem range, store end iterators
    RandomAccessIterator out=elems_begin;
    for (RandomAccessIterator2 i=partitions_begin; i!=partitions_end; ++i)
    {
        const wp_t&wp(wps.top());
        std::cout<<"wp effort:"<<wp.get_effort()<<std::endl;
        *i=out=copy(wp.begin(), wp.end(), out);
        wps.pop();
    }
}

} // namespace util
} // namespace vibello

#endif	/* VIBELLO_UTIL_PARTITION_H */
