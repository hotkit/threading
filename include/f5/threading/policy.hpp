/*
    Copyright 2015, Felspar Co Ltd. http://www.kirit.com/f5
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


namespace f5 {


    inline namespace threading {


        /// Policy which returns the values held in the container
        template<typename V>
        struct container_default_policy {
            using found_type = V;
            using value_return_type = V;

            static found_type found_from_V(const V &v) {
                return v;
            }
            static value_return_type value_from_V(V &v) {
                return v;
            }
        };

        template<typename V>
        struct pointer_dereference_policy {
            using found_type = decltype(&*V());
            using value_return_type = decltype(*V());

            static found_type found_from_V(const V &v) {
                return &*v;
            }
            static value_return_type value_from_V(V &v) {
                return *v;
            }
        };


    }


}

