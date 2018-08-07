#include <f5/threading/limiters.hpp>
#include <f5/threading/sync.hpp>
#include <cassert>
#include <iostream>
#include <thread>


int main() {
    boost::asio::io_service ios;
    f5::threading::fd::unlimited ul{ios};
    std::atomic<std::size_t> consumed{};
    f5::threading::sync s1, s2;
    std::atomic<bool> continued{false};

    boost::asio::spawn(ios, [&](auto yield) {
        s1.done();
        consumed += ul.consume(yield);
        continued = true;
        s2.done();
    });
    std::thread t{[&]() {
        ios.run();
    }};

    s1.wait();
    if ( continued ) {
        std::cout << "Continued flag set too early" << std::endl;
        return 2;
    }
    ul.produced();
    s2.wait();
    t.join();

    if ( consumed != 1u ) {
        std::cout << "Produced 1, consumed " << consumed << std::endl;
        return 3;
    }

    return 0;
}

