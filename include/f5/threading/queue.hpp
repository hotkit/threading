/**
    Copyright 2017-2018, Felspar Co Ltd. <https://www.kirit.com/f5>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#pragma once


#include <f5/threading/limiters.hpp>

#include <deque>
#include <experimental/optional>
#include <mutex>


namespace f5 {


    namespace boost_asio {


        /// A producer/consumer queue compatible with Boost.ASIO and
        /// coroutines. In this queue design the producers may always
        /// enque items into the queue without having to wait for the
        /// consumer to make room. For a capacity limited version of a
        /// similar concept see channel.
        template<typename T, typename S = std::deque<T>>
        class queue {
            /// Mutex that controls access to the queue items
            std::mutex exclusive;
            /// The current queue content
            S items;
            /// Communication between producer and consumer about how
            /// many items are in the channel
            threading::fd::unlimited signal;
            /// Return and pop the head of the deque. There must already
            /// be a lock covering the deque
            T pop_head() {
                auto ret = std::move(items.front());
                items.pop_front();
                return ret;
            }
        public:
            /// The type of storage used by the queue
            using store_type = S;
            /// The type of item that is put in the queue
            using value_type = T;

            /// Construct for the specified IO service
            queue(boost::asio::io_service &ios, S s = S())
            : items(std::move(s)), signal{ios} {
            }

            /// Produce an item to be consued
            void produce(T t) {
                std::unique_lock<std::mutex> lock{exclusive};
                items.push_back(std::move(t));
                lock.unlock();
                signal.produced();
            }

            /// Consume an item, block the coroutine until one becomes
            /// available.
            T consume(boost::asio::yield_context yield) {
                while ( true ) {
                    auto check_size = [this]() {
                        std::unique_lock<std::mutex> lock{exclusive};
                        return items.size();
                    };
                    while ( not check_size() ) {
                        signal.consume(yield);
                    }
                    std::lock_guard<std::mutex> lock{exclusive};
                    /// We need to recheck for there being items again
                    /// because we released the lock above and another
                    /// thread could have come in and stolen the item.
                    /// If there isn't one we'll loop around again until
                    /// there is.
                    if ( items.size() ) {
                        return pop_head();
                    }
                }
            }
            /// Return a job if one is available
            std::experimental::optional<T> consume() {
                std::unique_lock<std::mutex> lock{exclusive};
                if ( items.size() ) {
                    return pop_head();
                } else {
                    return {};
                }
            }

            /// Close the queue
            void close() {
                signal.close();
            }
        };


    }


}

