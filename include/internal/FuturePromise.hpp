/*
 * ncpasscpp. A library that implements Nextcloud Password's API.
 * Copyright (C) 2021  Reed Krantz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#include <memory>
#if defined _WIN32 || defined __CYGWIN__
    #ifdef BUILDING_NCPASSCPP
        #define NCPASSCPP_PUBLIC __declspec(dllexport)
    #else
        #define NCPASSCPP_PUBLIC __declspec(dllimport)
    #endif
#else
    #ifdef BUILDING_NCPASSCPP
        #define NCPASSCPP_PUBLIC __attribute__ ((visibility("default")))
    #else
        #define NCPASSCPP_PUBLIC
    #endif
#endif

#include <future>

namespace ncpass::internal
{




/**
 * @brief With this class you can obtain futures for synchronization before you create the promise.
 * This is useful for performing an action in the Base class, like unlocking a private mutex, after the child class is ready for it.
 * @author Reed Krantz
 */
template <typename PromiseType>
class NCPASSCPP_PUBLIC FuturePromise
{
  protected:

    std::unique_ptr<std::promise<PromiseType>> _promise = nullptr;
    std::shared_future<PromiseType> _future;


  public:

    /**
     * @return A std::shared_future to a promise that has not yet been created.
     */
    std::shared_future<PromiseType> getFuture()
    {
        if(_promise == nullptr)
        {
            _promise.reset(new std::promise<PromiseType>);
            _future = _promise->get_future().share();
        }

        return _future;
    }

    /**
     * @return The promise to the previous futures obtained with getFuture() since the previous getPromise() call.
     */
    std::promise<PromiseType> getPromise()
    {
        return std::move(*_promise);
    }
};


}
