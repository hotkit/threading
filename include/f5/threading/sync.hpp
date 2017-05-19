/*
    Copyright 2017, Felspar Co Ltd. http://www.kirit.com/f5
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <boost/coroutine/exceptions.hpp>

#include <future>


namespace f5 {


    inline namespace threading {


        /// Sync between two threads. The controlling thread creates
        /// an instance of sync and then waits on it. The other thread is
        /// given a reference and signals by calling done when its work
        /// is complete. Effectively just a thin wrapper around a future.
        class sync {
            std::promise<void> blocker;
        public:
            void wait() {
                auto ready = blocker.get_future();
                ready.get();
            }

            void done() {
                blocker.set_value();
            }

            template<typename F>
            auto operator () (F op) {
                return [=](auto &&...s) {
                    try {
                        op(std::forward<decltype(s)>(s)...);
                        blocker.set_value();
                    } catch ( boost::coroutines::detail::forced_unwind&  ) {
                        blocker.set_value();
                        throw;
                    } catch ( ... ) {
                        blocker.set_exception(std::current_exception());
                    }
                };
            }
        };


    }


}

