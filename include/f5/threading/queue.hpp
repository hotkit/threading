/*
    Copyright 2017, Felspar Co Ltd. http://www.kirit.com/f5
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <f5/threading/channel.hpp>

#include <boost/circular_buffer.hpp>


namespace f5 {


    namespace boost_asio {


        /// Combine a circular buffer and an eventfd limiter for mulitple
        /// producers and consumers in a capacity limited manner. For a
        /// similar construct which accepts unlimited items see channel.
        template<typename V>
        class queue {
            using channel_job = std::pair<std::unique_ptr<eventfd::limiter::job>, V>;
            channel<channel_job, boost::circular_buffer<channel_job>> buffer;
            eventfd::limiter throttle;

        public:
            /// Construct a new queue with the specified capacity
            queue(boost::asio::io_service &ios, uint64_t limit)
            : buffer(limit), throttle(ios, limit) {
            }

            /// Return the IO service
            boost::asio::io_service &get_io_service() {
                return throttle.get_io_service();
            }

            /// Add a new item to the buffer. The coroutine yields until
            /// there is space for the item. Returns the remaining capacity
            void produce(V v, boost::asio::yield_context &yield) {
                auto job = throttle.next_job(yield);
                buffer.push-back(std::make_pair(std::move(job), std::move(v)));
            }

            /// Yield until a value is available to consume. The space in
            /// the buffer is freed up straight away.
            V consume(boost::asio::yield_context &yield) {
                return buffer.consume(yield);
            }

            /// Yield until all of the work that has been produced has been
            /// consumed.
            void wait_for_all_outstanding(boost::asio::yield_context &yield) {
                throttle.wait_for_all_outstanding(yield);
            }
        };


    }


}

