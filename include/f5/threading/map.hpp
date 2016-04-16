/*
    Copyright 2015, Felspar Co Ltd. http://www.kirit.com/f5
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <mutex>
#include <vector>

#include <f5/threading/policy.hpp>


namespace f5 {


    inline namespace threading {


        /// Thread safe associative array (map) implemented on a std::vector
        template<typename K, typename V,
            typename P = typename container_default_policy<V>::type>
        class tsmap {
            /// Mutex used to control access to the vector
            mutable std::mutex mutex;
            /// Vector which stores the data
            std::vector<std::pair<K, V>> map;

            /// Traits for controlling aspects of the implementation
            using traits = P;

            /// Return the lower bound for the key
            template<typename L>
            auto lower_bound(const L &k) const {
                return std::lower_bound(map.begin(), map.end(), k,
                    [](const auto &l, auto r) {
                        return l.first < r;
                    });
            }
            template<typename L>
            auto lower_bound(const L &k) {
                return std::lower_bound(map.begin(), map.end(), k,
                    [](const auto &l, auto r) {
                        return l.first < r;
                    });
            }
        public:
            /// Return an estimate of the size of the map.
            std::size_t size() {
                std::unique_lock<std::mutex> lock(mutex);
                return map.size();
            }

            /// Return a pointer to the value if found. If not found then
            /// return nullptr
            template<typename L>
            typename traits::found_type find(const L &k, const V &s = V()) const {
                std::unique_lock<std::mutex> lock(mutex);
                auto bound = lower_bound(k);
                if ( bound == map.end() || k != bound->first ) {
                    return traits::found_from_V(s);
                } else {
                    return traits::found_from_V(bound->second);
                }
            }

            /// Ensures the item at the requested key is the value given
            template<typename A>
            typename traits::value_return_type insert_or_assign(
                const K &k, const A &a
            ) {
                std::unique_lock<std::mutex> lock(mutex);
                auto bound = lower_bound(k);
                if ( bound != map.end() && bound->first == k ) {
                    // We have a cache hit, so assign
                    return traits::value_from_V(bound->second = a);
                } else {
                    // We have a cache miss so insert
                    map.emplace(bound, std::piecewise_construct,
                        std::forward_as_tuple(k),
                        std::forward_as_tuple(a));
                    return traits::value_from_V(map.back().second);
                }
            }
            /// Adds the item if the key is not found. If the key is found and
            /// the predicate returns true then replaces the value with the
            /// one returned by the lambda
            template<typename C, typename F>
            typename traits::value_return_type insert_or_assign_if(
                const K &k, C predicate, F lambda
            ) {
                std::unique_lock<std::mutex> lock(mutex);
                auto bound = lower_bound(k);
                if ( bound != map.end() && bound->first == k ) {
                    // Cache hit so check the predicate
                    if ( predicate(bound->second) ) {
                        return traits::value_from_V(bound->second = lambda());
                    } else {
                        return traits::value_from_V(bound->second);
                    }
                }
                // Cache miss, so use the lambda to get the value to insert
                return traits::value_from_V(
                    map.emplace(bound, std::piecewise_construct,
                        std::forward_as_tuple(k),
                        std::forward_as_tuple(lambda()))->second);
            }
            /// Adds a value at the key if there isn't one there already.
            /// Returns a reference to the item
            template<typename... Args>
            typename traits::value_return_type emplace_if_not_found(
                const K &k, Args&&... args
            ) {
                std::unique_lock<std::mutex> lock(mutex);
                auto bound = lower_bound(k);
                if ( bound != map.end() && bound->first == k ) {
                    // A cache hit, so return what we have
                    return traits::value_from_V(bound->second);
                }
                // Insert before returning the new value
                return traits::value_from_V(
                    map.emplace(bound, std::piecewise_construct,
                        std::forward_as_tuple(k),
                        std::forward_as_tuple(args...))->second);
            }
            /// Adds a value at the key if there isn't one there already.
            /// Returns a reference to the newly constructed item. If
            /// the item is arleady in the map then the second lambda is
            /// executed.
            template<typename F, typename M>
            typename traits::value_return_type add_if_not_found(
                const K &k, F lambda, M miss
            ) {
                std::unique_lock<std::mutex> lock(mutex);
                auto bound = lower_bound(k);
                if ( bound != map.end() && bound->first == k ) {
                    // Cache hit so don't run the lambda
                    miss(traits::value_from_V(bound->second));
                    return traits::value_from_V(bound->second);
                }
                // Cache miss, so use the lambda to get the value to insert
                return traits::value_from_V(
                    map.emplace(bound, std::piecewise_construct,
                        std::forward_as_tuple(k),
                        std::forward_as_tuple(lambda()))->second);
            }
            /// Adds a value at the key if there isn't one there already.
            template<typename F>
            typename traits::value_return_type add_if_not_found(
                const K &k, F lambda
            ) {
                return add_if_not_found(k, lambda, [](const auto &){});
            }

            /// Iterate over the content of the map
            template<typename F>
            F for_each(F fn) const {
                std::unique_lock<std::mutex> lock(mutex);
                std::for_each(map.begin(), map.end(),
                    [fn](const auto &v) {
                        fn(v.first, v.second);
                    });
                return std::move(fn);
            }

            /// Remove the requested key (if found). Returns true if the
            /// key and its value were removed
            bool remove(const K &k) {
                std::unique_lock<std::mutex> lock(mutex);
                auto bound = lower_bound(k);
                if ( bound == map.end() )
                    return false;
                else {
                    map.erase(bound);
                    return true;
                }
            }

            /// Removes values where the predicate is true. Returns how
            /// many are left.
            template<typename Pr>
            std::size_t remove_if(Pr predicate) {
                std::unique_lock<std::mutex> lock(mutex);
                map.erase(std::remove_if(map.begin(), map.end(),
                    [predicate](const auto &v) {
                        return predicate(v.first, v.second);
                    }), map.end());
                return map.size();
            }
        };


    }


}

