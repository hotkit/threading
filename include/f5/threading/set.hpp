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


        /// Thread safe set implemented on a std::vector
        template<typename V,
            typename P = typename container_default_policy<V>::type>
        class tsset {
            /// Mutex used to control access to the vector
            mutable std::mutex mutex;
            /// Vector which stores the data
            std::vector<V> set;

            /// Traits for controlling aspects of the implementation
            using traits = P;

            /// Return the lower bound for the key
            auto lower_bound(const V &k) const {
                return std::lower_bound(set.begin(), set.end(), k,
                    [](const auto &l, auto r) {
                        return l < r;
                    });
            }
            auto lower_bound(const V &k) {
                return std::lower_bound(set.begin(), set.end(), k,
                    [](const auto &l, auto r) {
                        return l < r;
                    });
            }
        public:
            /// Return an estimate of the size of the set.
            std::size_t size() {
                std::unique_lock<std::mutex> lock(mutex);
                return set.size();
            }

            /// Insert the item if not found. Returns true if the item was
            /// inserted.
            bool insert_if_not_found(const V &v) {
                std::unique_lock<std::mutex> lock(mutex);
                auto bound = lower_bound(v);
                if ( bound == set.end() || *bound != v ) {
                    set.insert(bound, v);
                    return true;
                }
                return false;
            }

            /// Iterate over the content of the set
            template<typename F>
            F for_each(F fn) const {
                std::unique_lock<std::mutex> lock(mutex);
                return std::move(std::for_each(set.begin(), set.end(), fn));
            }

            /// Remove the last item from the set and return it. If the set
            /// is empty then return the argument passed.
            typename traits::found_type pop_back(const V &s = V()) {
                std::unique_lock<std::mutex> lock(mutex);
                if ( set.empty() ) {
                    return traits::found_from_V(s);
                } else {
                    typename traits::found_type b{traits::found_from_V(set.back())};
                    set.pop_back();
                    return b;
                }
            }

            /// Remove the items that match the predicate
            template<typename F>
            std::size_t remove_if(F fn) {
                std::unique_lock<std::mutex> lock(mutex);
                set.erase(std::remove_if(set.begin(), set.end(), fn), set.end());
                return set.size();
            }
        };


    }


}

