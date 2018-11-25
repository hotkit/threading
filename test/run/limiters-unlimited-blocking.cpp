#include <f5/threading/limiters.hpp>
#include <f5/threading/sync.hpp>
#include <cassert>
#include <iostream>
#include <thread>


int main() {
    boost::asio::io_service ios;
    f5::threading::fd::unlimited ul{ios};
    std::atomic<std::size_t> consumed{};
    std::atomic<bool> continued{false};

    /**
        This coroutine will be paused in the middle as there won't be any
        production for it to consume.
     */
    f5::threading::sync s1, s2;
    boost::asio::spawn(ios, [&](auto yield) {
        s1.done();
        consumed += ul.consume(yield);
        continued = true;
        s2.done();
    });
    /**
        We have to run the executor in another thread because the call
        to `run` won't exit until the coroutine has run to completion.
     */
    std::thread t{[&]() { ios.run(); }};

    /// Don't continue past here until we know the coroutine has started
    s1.wait();
    if (continued) {
        std::cout << "Continued flag set too early" << std::endl;
        return 2;
    }
    /// Produce something for the coroutine to consume
    ul.produced();
    /// Make sure the coroutine has exited
    s2.wait();
    /// Make sure that `ios.run()` has returned
    t.join();

    if (consumed != 1u) {
        std::cout << "Produced 1, consumed " << consumed << std::endl;
        return 3;
    }

    return 0;
}
