/*
    Copyright 2015, Felspar Co Ltd. http://www.kirit.com/f5
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <boost/circular_buffer.hpp>

#include <mutex>


namespace f5 {


    inline namespace threading {


        /// Thread safe circular buffer. It has a fixed number of slots.
        template<typename V>
        class tsring {
            std::mutex mutex;
            boost::circular_buffer<V> ring;
        public:
            /// Construct a ring with the specified number of slots available
            tsring(std::size_t s)
            : ring(s) {
            }

            /// Emplace an item on to the end of the buffer. If the buffer is full
            /// then the first item is overwritten.
            ///
            /// Returns the number of free slots in the buffer
            template<typename F>
            std::size_t push_back(F fn) {
                std::unique_lock<std::mutex> lock(mutex);
                ring.push_back(fn());
                return ring.capacity() - ring.size();
            }
            /// Emplace an item on to the back of the buffer. If the buffer is full
            /// the predicate is run. It can return `true` to indicate that the new
            /// should be placed anyway.
            ///
            /// Returns the number of free slots in the buffer
            template<typename F, typename P>
            std::size_t push_back(F fn, P pred) {
                std::unique_lock<std::mutex> lock(mutex);
                if ( !ring.full() || pred(ring.back()) ) {
                    ring.push_back(fn());
                }
                return ring.capacity() - ring.size();
            }

            /// Pop the beginning of the buffer and return the value that was there. If the buffer
            /// is empty then return the value passed in
            template<typename D>
            D pop_front(D d) {
                std::unique_lock<std::mutex> lock(mutex);
                if ( ring.empty() ) {
                    return std::move(d);
                } else {
                    auto v = std::move(ring[0]);
                    ring.pop_front();
                    return std::move(v);
                }
            }
        };


    }


}

