#include <f5/threading/limiters.hpp>
#include <cassert>
#include <iostream>


int main() {
    boost::asio::io_service ios;
    f5::threading::fd::unlimited ul{ios};
    for ( std::size_t n{}; n < 100; ++n ) {
        ul.produced(1);
    }

    std::size_t consumed{};
    for ( std::size_t n{}; n < 100; ++n ) {
        boost::asio::spawn(ios, [&](auto yield) {
            consumed += ul.consume(yield);
        });
    }
    ios.run();

    if ( consumed != 100u ) {
        std::cout << "Produced 100, consumed " << consumed << std::endl;
        return 1;
    } else {
        return 0;
    }
}

