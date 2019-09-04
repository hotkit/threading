/**
    Copyright 2017-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <f5/threading/queue.hpp>

#include <boost/circular_buffer.hpp>


namespace f5 {


    namespace boost_asio {


        /// Combine a circular buffer and an eventfd limiter for mulitple
        /// producers and consumers in a capacity limited manner. For a
        /// similar construct which accepts unlimited items see queue.
        template<typename V>
        class channel {
            using queue_job = std::pair<std::unique_ptr<fd::limiter::job>, V>;
            using queue_type =
                    queue<queue_job, boost::circular_buffer<queue_job>>;
            queue_type buffer;
            fd::limiter throttle;

          public:
            /// Construct a new channel with the specified capacity
            channel(boost::asio::io_service &ios, uint64_t limit)
            : buffer(ios, typename queue_type::store_type(limit)),
              throttle(ios, limit) {}

            /// Return the IO service
            boost::asio::io_service &get_io_service() {
                return throttle.get_io_service();
            }
            /// Return the capacity of the channel
            std::size_t size() const { return throttle.limit(); }

            /// Add a new item to the buffer. The coroutine yields until
            /// there is space for the item. Returns the remaining capacity
            template<typename Y>
            void produce(V v, Y yield) {
                auto job = throttle.next_job(yield);
                buffer.produce(std::make_pair(std::move(job), std::move(v)));
            }

            /// Yield until a value is available to consume. The space in
            /// the buffer is freed up straight away.
            template<typename Y>
            V consume(Y yield) {
                return buffer.consume(yield).second;
            }

            /// Yield until all of the work that has been produced has been
            /// consumed.
            template<typename Y>
            void wait_for_all_outstanding(Y yield) {
                throttle.wait_for_all_outstanding(yield);
            }

            /// Close the channel. Does not wait for work to complete
            void close() {
                throttle.close();
                buffer.close();
            }
        };


    }


}
