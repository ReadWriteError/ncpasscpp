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
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

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
    const std::string k_apiPath;                                  ///< The path to append to the URL (example: ncpass::Password would be "password/").
    static std::vector<std::shared_ptr<API_Type>> s_allInstances; ///< Contains all the instances currently existing of a certain API_Type.


  protected:

    /**
     * @brief Constructor for providing the Nextcloud server's credentials.
     * @param session A shared_ptr to a ncpass::Session instance used as credentials for the Nextcloud server's API.
     * @param apiPath The path to append to the URL (example: ncpass::Password would be "password").
     * @see ncpass::Session
     * @see https://git.mdns.eu/nextcloud/passwords/wikis/Developers/Index
     */
    API_Implementor(const std::shared_ptr<Session>& session, const std::string& apiPath);

    /**
     * @brief Constructor that obtains the ncpass::Session instance from another object.
     * @param apiObject An ncpass::API_Implementor object that will be used to obtain the ncpass::Session. This is done by copying the shared_ptr<ncpass::Session> within the object.
     * @param apiPath The path to append to the URL (example: ncpass::Password would be "password").
     * @see ncpass::Session
     * @see https://git.mdns.eu/nextcloud/passwords/wikis/Developers/Index
     */
    API_Implementor(const API_Implementor& apiObject, const std::string& apiPath);

    /**
     * @brief Constructor for ncpass::Session itself because it is also an API_Implementor.
     * @param apiPath The path to append to the URL (example: ncpass::Password would be "password").
     * @see ncpass::Session
     * @see https://git.mdns.eu/nextcloud/passwords/wikis/Developers/Index
     */
    API_Implementor(const std::string& apiPath);

    /**
     * @brief Gets all of the instances of the derived class in API_Type.
     * @return A vector containing all currently active instances.
     */
    static std::vector<std::shared_ptr<API_Type>> getAllLocal();

    /**
     * @brief Gets the shared pointer for the current instance.
     * @return A copy of the shared pointer for the current instance.
     */
    std::shared_ptr<API_Type> getSharedPtr();

    /**
     * @brief Make a curl POST request.
     * @param apiAction The final part of the API URL.
     * @param apiArgs The arguments for the POST request. The layout for the array is like this { {"arg1", "value"}, {"arg2", "value"} }
     * @return The returning JSON of the call.
     */
    template <unsigned int N>
    nlohmann::json ncPOST(const std::string& apiAction, const std::string (& apiArgs)[N][2]);
};


}
