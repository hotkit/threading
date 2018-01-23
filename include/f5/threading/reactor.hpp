/*
    Copyright 2015-2017 Felspar Co Ltd. http://www.kirit.com/f5
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <boost/asio/io_service.hpp>
#include <boost/coroutine/exceptions.hpp>

#include <thread>


namespace f5 {


    namespace boost_asio {


        /// A pool of the requested number of threads for use in servicing
        /// an io_service.
        class reactor_pool final {
            /// The Boost ASIO IO service that is run by this pool
            boost::asio::io_service ios;
            /// The threads in the pool
            std::vector<std::thread> threads;
            /// Work instance used to stop the threads from terminating
            /// until we want them to.
            std::unique_ptr<boost::asio::io_service::work> work;

        public:
            /// Default construct a reactor pool with thread count matching
            /// the hardware and that will terminate threads if exceptions
            /// leak
            reactor_pool()
            : reactor_pool([](){ return false; }) {
            }

            /// Construct a pool with the requested thread count and using
            /// the passed in exception handler. The handler returns true
            /// if it wishes this thread to continue to handle jobs. If it
            /// returns false then the thread exits, but it won't be joined
            /// until the pool itself passes out of scope.
            template<typename F>
            explicit reactor_pool(
                F exception_handler,
                std::size_t thread_count = std::thread::hardware_concurrency()
            ) : work(std::make_unique<boost::asio::io_service::work>(ios)) {
                for ( auto t = 0u; t != thread_count; ++t ) {
                    threads.emplace_back(
                        [this, exception_handler]() {
                            bool again = false;
                            do {
                                try {
                                    again = false;
                                    ios.run();
                                } catch ( boost::coroutines::detail::forced_unwind&  ) {
                                    throw;
                                } catch ( ... ) {
                                    again = exception_handler();
                                }
                            } while ( again );
                        });
                }
            }

            /// Stop all work and join all threads
            void close() {
                if ( work ) {
                    work.reset();
                    ios.stop();
                    for ( auto &t : threads ) t.join();
                }
            }
            ~reactor_pool() {
                close();
            }

            /// Make non-copyable and non assignable
            reactor_pool(const reactor_pool &) = delete;
            reactor_pool &operator = (const reactor_pool &) = delete;

            /// Return the number of threads servicing the pool
            std::size_t size() const {
                return threads.size();
            }

            /// Return the contained io_service instance
            boost::asio::io_service &get_io_service() {
                return ios;
            }
        };


    }


}

