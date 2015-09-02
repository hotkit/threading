/*
    Copyright 2015, Felspar Co Ltd. http://www.kirit.com/f5
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/core>

#include <boost/asio/spawn.hpp>

#include <system_error>


namespace f5 {


    inline namespace threading {


        namespace eventfd {


            /// Allows for a limit to be placed on work through a
            /// Boost ASIO reactor.
            class limiter {
                /// The IO service
                boost::asio::io_service &service;
                /// The yield context that we can use
                boost::asio::yield_context &yield;
                /// The file descriptor
                boost::asio::posix::stream_descriptor fd;
                /// The limit before we block waiting for some of the work
                /// to complete.
                const uint64_t m_limit;
                /// The amount of outstanding work
                uint64_t m_outstanding;

                /// Wait until at least one job has completed. Returns
                /// the number of jobs that have completed.
                uint64_t wait(boost::asio::yield_context &yield) {
                    uint64_t count = 0;
                    boost::asio::streambuf buffer;
                    boost::asio::async_read(fd, buffer,
                        boost::asio::transfer_exactly(sizeof(count)), yield);
                    buffer.sgetn(reinterpret_cast<char *>(&count), sizeof(count));
                    assert(count <= m_outstanding);
                    m_outstanding -= count;
                    return count;
                }

                /// Get the file descriptor, or throw an exception
                static auto get_eventfd() {
                    auto fd = ::eventfd(0, 0);
                    if ( fd < 0 ) {
                        std::error_code error(errno, std::system_category());
                        throw fostlib::exceptions::null(
                            "Bad eventfd file descriptor", error.message().c_str());
                    }
                    return fd;
                }
            public:
                /// Construct with a given limit
                limiter(
                    boost::asio::io_service &ios, boost::asio::yield_context &y, uint64_t limit
                ) : service(ios), yield(y),
                    fd(ios, get_eventfd()),
                    m_limit(limit),
                    m_outstanding{}
                {}
                /// The destructor ensures that there is no outstanding work
                /// before it completes
                ~limiter() {
                    while ( m_outstanding )
                        wait(yield);
                }

                /// Return the IO service
                boost::asio::io_service &get_io_service() {
                    return service;
                }

                /// The number of outstanding jobs
                uint64_t limit() const {
                    return m_limit;
                }
                /// The current number of outstanding jobs
                uint64_t outstanding() const {
                    return m_outstanding;
                }

                /// A proxy for an outstanding job
                class job {
                    /// TODO: This implementation uses the shared_ptr
                    /// for reference counting. This is really a bit of a
                    /// waste as the job (probably) doesn't need to be
                    /// thread safe.
                    friend class limiter;
                    /// Set to true when the job has been signalled
                    bool completed;
                    /// The limiter that owns this job
                    limiter &limit;
                    /// Construct a new job
                    job(limiter &l)
                    : completed(false), limit(l) {
                    }
                public:
                    /// Make non-copyable
                    job(const job &) = delete;
                    job &operator = (const job &) = delete;

                    /// Move constructor
                    job(job &&j)
                    : completed(j.completed), limit(j.limit) {
                        j.completed = true; // Never call done on the one moved from
                    }
                    /// Signal the job as completed, if not already done so
                    ~job() {
                        done([](auto, auto){});
                    }

                    /// Signal that the job is completed, if not already done so
                    template<typename E>
                    void done(E efn) {
                        if ( !completed ) {
                            completed = true;
                            uint64_t count = 1;
                            boost::asio::async_write(limit.fd,
                                boost::asio::buffer(&count, sizeof(count)),
                                [efn, &count](auto error, auto bytes) {
                                    if ( error || bytes != sizeof(count) )
                                        efn(error, bytes);
                                });
                        }
                    }
                };
                friend class job;

                /// Add another outstanding job and return it
                std::shared_ptr<job> operator ++ () {
                    while ( m_outstanding > m_limit )
                        wait(yield);
                    ++m_outstanding;
                    return std::make_shared<job>(job(*this));
                }
            };


        }


    }


}

