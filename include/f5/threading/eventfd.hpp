/*
    Copyright 2015-2016, Felspar Co Ltd. http://www.kirit.com/f5
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/core>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include <system_error>


namespace f5 {


    inline namespace threading {


        namespace eventfd {


            /// Store a Boost ASIO compatible eventfd file descriptor. It is
            /// a drop in replacement for the underlying Boost
            /// stream_descriptor, but ensures that the file descriptor
            /// is properly established.
            class fd {
                boost::asio::posix::stream_descriptor descriptor;

            public:
                /// Get a new file descriptor, or throw an exception
                static auto create(unsigned int initval = 0, int flags = 0) {
                    auto fd = ::eventfd(initval, flags);
                    if ( fd < 0 ) {
                        std::error_code error(errno, std::system_category());
                        throw fostlib::exceptions::null(
                            "Bad eventfd file descriptor", error.message().c_str());
                    }
                    return fd;
                }

                /// Construct an eventfd with a file descriptor in it
                fd(boost::asio::io_service &ios)
                : descriptor(ios, create()) {
                }

                /// Fetch a reference to the stream_descriptor
                operator boost::asio::posix::stream_descriptor &() {
                    return descriptor;
                }

                /// Forward call to embedded descriptor
                template<typename... U>
                auto async_read_some(U&&... u) {
                    return descriptor.async_read_some(std::forward<U>(u)...);
                }

                /// Forward call to embedded descriptor
                template<typename... U>
                auto async_write_some(U&&... u) {
                    return descriptor.async_write_some(std::forward<U>(u)...);
                }

                /// Read the current value from the file descriptor. Yields
                /// until it is available.
                int64_t read(boost::asio::yield_context &yield) {
                    uint64_t count = 0;
                    boost::asio::streambuf buffer;
                    boost::asio::async_read(descriptor, buffer,
                        boost::asio::transfer_exactly(sizeof(count)), yield);
                    buffer.sgetn(reinterpret_cast<char *>(&count), sizeof(count));
                    return count;
                }
            };


            /// Allow for an unlimited produer/consumer which never
            /// blocks due to the producer trying to send through too
            /// much work.
            class unlimited {
                /// The IO service
                boost::asio::io_service &service;
                /// The file descriptor
                eventfd::fd fd;

            public:
                /// Constructs the producer/consumer channel
                unlimited(
                    boost::asio::io_service &ios
                ) : service(ios), fd(ios) {
                }

                /// Return the IO service
                boost::asio::io_service &get_io_service() {
                    return service;
                }

                /// Send the amount of work produced to the consumer
                /// side.
                void produced(uint64_t count = 1) {
                    boost::asio::async_write(fd,
                        boost::asio::buffer(&count, sizeof(count)),
                        [](auto error, auto bytes) {
                        });
                }
                /// Return how much to consume. Yields until there is
                /// something available.
                uint64_t consume(boost::asio::yield_context &yield) {
                    return fd.read(yield);
                }
            };


            /// Allows for a limit to be placed on work through a
            /// Boost ASIO reactor. Jobs can be added up to a specified
            /// limit. If the limit is reached then the producer waits for
            /// the consumer to finish at least one job before starting up
            /// again.
            class limiter {
                /// The IO service
                boost::asio::io_service &service;
                /// The yield context that we can use
                boost::asio::yield_context &yield;
                /// The file descriptor
                eventfd::fd fd;
                /// The limit before we block waiting for some of the work
                /// to complete.
                const uint64_t m_limit;
                /// The amount of outstanding work
                uint64_t m_outstanding;

                /// Wait until at least one job has completed. Returns
                /// the number of jobs that have completed.
                uint64_t wait(boost::asio::yield_context &yield) {
                    const uint64_t count = fd.read(yield);
                    assert(count <= m_outstanding);
                    m_outstanding -= count;
                    return count;
                }
            public:
                /// Construct with a given limit
                limiter(
                    boost::asio::io_service &ios, boost::asio::yield_context &y, uint64_t limit
                ) : service(ios), yield(y),
                    fd(ios),
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

                /// The maximum number of outstanding jobs
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
                                [efn](auto error, auto bytes) {
                                    if ( error || bytes != sizeof(uint64_t) )
                                        efn(error, bytes);
                                });
                        }
                    }
                };
                friend class job;

                /// Add another outstanding job and return it
                std::shared_ptr<job> operator ++ () {
                    while ( m_limit && m_outstanding >= m_limit )
                        wait(yield);
                    ++m_outstanding;
                    return std::shared_ptr<job>(new job(*this));
                }
            };


        }


    }


}

