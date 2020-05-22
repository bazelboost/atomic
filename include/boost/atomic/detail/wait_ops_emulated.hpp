/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2020 Andrey Semashev
 */
/*!
 * \file   atomic/detail/wait_ops_emulated.hpp
 *
 * This header contains emulated (lock-based) implementation of the wait/notify atomic operations.
 */

#ifndef BOOST_ATOMIC_DETAIL_WAIT_OPS_EMULATED_HPP_INCLUDED_
#define BOOST_ATOMIC_DETAIL_WAIT_OPS_EMULATED_HPP_INCLUDED_

#include <cstddef>
#include <boost/memory_order.hpp>
#include <boost/atomic/detail/config.hpp>
#include <boost/atomic/detail/lock_pool.hpp>
#include <boost/atomic/detail/wait_operations_fwd.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {
namespace atomics {
namespace detail {

//! Emulated implementation of wait/notify operations
template< typename Base >
struct emulated_wait_operations :
    public Base
{
    typedef Base base_type;
    typedef typename base_type::storage_type storage_type;
    typedef lock_pool::scoped_lock< base_type::storage_alignment, true > scoped_lock;
    typedef lock_pool::scoped_wait_state< base_type::storage_alignment > scoped_wait_state;

    static BOOST_FORCEINLINE storage_type wait(storage_type const volatile& storage, storage_type old_val, memory_order) BOOST_NOEXCEPT
    {
        storage_type const& s = const_cast< storage_type const& >(storage);
        scoped_wait_state wait_state(&storage);
        storage_type new_val = s;
        while (new_val == old_val)
        {
            wait_state.wait();
            new_val = s;
        }

        return new_val;
    }

    static BOOST_FORCEINLINE void notify_one(storage_type volatile& storage) BOOST_NOEXCEPT
    {
        scoped_lock lock(&storage);
        lock_pool::notify_one(lock.get_lock_state(), &storage);
    }

    static BOOST_FORCEINLINE void notify_all(storage_type volatile& storage) BOOST_NOEXCEPT
    {
        scoped_lock lock(&storage);
        lock_pool::notify_all(lock.get_lock_state(), &storage);
    }
};

template< typename Base, std::size_t Size >
struct wait_operations< Base, Size, false > :
    public emulated_wait_operations< Base >
{
};

} // namespace detail
} // namespace atomics
} // namespace boost

#endif // BOOST_ATOMIC_DETAIL_WAIT_OPS_EMULATED_HPP_INCLUDED_
