/*
    Copyright 2017 Felspar Co Ltd. http://www.kirit.com/f5
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <f5/threading/eventfd.hpp>

#include <deque>
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
            /// The number of items in the deque that we know we can pop
            /// because the number has already been given to us by the
            /// signal.
            uint64_t poppable;
        public:
            /// The type of item that is put in the channel
            using value_type = T;

            /// Construct for the specified IO service
            channel(boost::asio::io_service &ios)
            : signal{ios}, poppable{} {
            }

            /// Produce an item to be consued
            void produce(T&&t) {
                std::unique_lock<std::mutex> lock{exclusive};
                items.push_back(std::move(t));
                lock.unlock();
                signal.produced();
            }

            /// Consume an item
            T consume(boost::asio::yield_context &yield) {
                while ( not poppable ) {
                    poppable += signal.consume(yield);
                }
                std::lock_guard<std::mutex> lock{exclusive};
                auto ret = std::move(items.front());
                items.pop_front();
                --poppable;
                return ret;
            }
        };


    }


}

