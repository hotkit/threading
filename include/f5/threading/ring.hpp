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


        /// Thread safe circular buffer
        template<typename V>
        class tsring {
            std::mutex mutex;
            boost::circular_buffer<V> ring;
        public:
            tsring(std::size_t s)
            : ring(s) {
            }

            template<typename F>
            bool emplace_back(F lambda) {
                return false;
            }
        };


    }


}

