/*
    Copyright 2015, Felspar Co Ltd. http://www.kirit.com/f5
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <mutex>
#include <vector>


namespace f5 {


    inline namespace threading {


        /// Thread safe associative array (map) implemented on a std::vector
        template<typename K, typename V>
        class tsmap {
            std::mutex mutex;
            std::vector<std::pair<K, V>> map;
        public:
            /// Adds a value at the key if there isn't one there already.
            /// Returns a reference to the item
            template<typename... Args>
            V &emplace_if_not_found(const K &k, Args&&... args) {
                std::unique_lock<std::mutex> lock(mutex);
                auto bound = std::lower_bound(map.begin(), map.end(), k,
                    [](const auto &l, auto r) {
                        return l.first < r;
                    });
                if ( bound != map.end() && bound->first == k ) {
                    return bound->second;
                }
                return map.emplace(bound, std::piecewise_construct,
                    std::forward_as_tuple(k),
                    std::forward_as_tuple(args...))->second;
            }
            /// Adds a value at the key if there isn't one there already.
            /// Returns a reference to the newly constructed item
            template<typename F>
            V &add_if_not_found(const K &k, F lambda) {
                std::unique_lock<std::mutex> lock(mutex);
                auto bound = std::lower_bound(map.begin(), map.end(), k,
                    [](const auto &l, auto r) {
                        return l.first < r;
                    });
                if ( bound != map.end() && bound->first == k ) {
                    return bound->second;
                }
                return map.emplace(bound, std::piecewise_construct,
                    std::forward_as_tuple(k),
                    std::forward_as_tuple(lambda()))->second;
            }
        };


    }


}

