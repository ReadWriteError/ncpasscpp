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

#include <memory>
#include <vector>

namespace ncpass
{




class Session;




/**
 * @brief Class used for API calls to the Nextcloud server.
 * Abstract class that needs to be inherited to provide access to the Nextcloud server's API. This is because this class is the **only** class that can access ncpass::Session.
 * @tparam API_Type The derived class you are creating.
 * @see ncpass::Session
 * @author Reed Krantz
 */
template <class API_Type>
class NCPASSCPP_PUBLIC API_Implementor
{
  private:

    const std::shared_ptr<Session> k_session;                     ///< Holds the Nextcloud server this instance is tied to.
    static std::vector<std::shared_ptr<API_Type>> s_allInstances; ///< Contains all the instances currently existing of a certain API_Type.


  protected:

    /**
     * @brief Constructor for providing the Nextcloud server's credentials.
     * @param session A shared_ptr to a ncpass::Session instance used as credentials for the Nextcloud server's API.
     * @see ncpass::Session
     */
    API_Implementor(const std::shared_ptr<Session>& session);

    /**
     * @brief Constructor that obtains the ncpass::Session instance from another object.
     * @param apiObject An ncpass::API_Implementor object that will be used to obtain the ncpass::Session. This is done by copying the shared_ptr<ncpass::Session> within the object.
     * @see ncpass::Session
     */
    API_Implementor(const API_Implementor& apiObject);

    /**
     * @brief Constructor for ncpass::Session itself because it is also an API_Implementor.
     * @see ncpass::Session
     */
    API_Implementor();

    /**
     * @brief Gets the shared pointer for the current instance.
     * @return A copy of the shared pointer for the current instance.
     */
    std::shared_ptr<API_Type> getSharedPtr();

    /**
     * @brief Gets all of the instances of the requested class.
     * @return A vector containing all currently active instances.
     */
    static std::vector<std::shared_ptr<API_Type>> getAll();
};


}
