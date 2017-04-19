/*
    Copyright 2017 Felspar Co Ltd. http://www.kirit.com/f5
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <f5/threading/eventfd.hpp>

#include <deque>
#include <experimental/optional>
#include <mutex>


namespace f5 {


    namespace boost_asio {


        /// A producer/consumer channel compatible with Boost.ASIO and
        /// coroutines. In this channel design the producers may always
        /// enque items into the channel without having to wait for the
        /// consumer to make room.
        template<typename T>
        class channel {
            /// Mutex that controls access to the queue items
            std::mutex exclusive;
            /// The current channel content
            std::deque<T> items;
            /// Communication between producer and consumer about how
            /// many items are in the channel
            threading::eventfd::unlimited signal;
            /// Return and pop the head of the deque. There must already
            /// be a lock covering the deque
            T pop_head() {
                auto ret = std::move(items.front());
                items.pop_front();
                return ret;
            }
        public:
            /// The type of item that is put in the channel
            using value_type = T;

            /// Construct for the specified IO service
            channel(boost::asio::io_service &ios)
            : signal{ios} {
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
            T consume(boost::asio::yield_context &yield) {
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
        };


    }


}

