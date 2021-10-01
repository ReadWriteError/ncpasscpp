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
#include <API_Implementor.hpp>
#include <curlpp/Options.hpp>


namespace ncpass
{




/**
 * @brief Signifies the connection to a nextcloud server.
 * Stores the login details, URL and handles all the encryption including E2EE. This is holds all you need to connect to the Nextcloud server.
 * @author Reed Krantz
 */
class NCPASSCPP_PUBLIC Session : public API_Implementor<Session>
{
  private:

    typedef API_Implementor<Session> _Base; ///< The base of this class.

    const std::string k_apiURL;                 ///< Base url to the api used to connect with the server (example: https://cloud.example.com/apps/passwords/api/1.0/).
    const curlpp::options::UserPwd k_usrPasswd; ///< User and password curl options used for authenticating with the api.


  protected:

    /**
     * @brief Constructor for Session.
     * @param username The user to login to the nextcloud server as.
     * @param serverRoot The root URL of the server without https:// or a path unless necessary for your server (example: cloud.example.com).
     * @param password The password for your login. If you have Two-Factor Authentication enabled this must be an app password.
     */
    Session(const std::string& username, const std::string& serverRoot, const std::string& password);

    /**
     * @brief Constructor for Session.
     * @param federatedID The federated ID of the user to login as (example: user@cloud.example.com).
     * @param password The password for your login. If you have Two-Factor Authentication enabled this must be an app password.
     */
    Session(const std::string& federatedID, const std::string& password);


  public:

    /**
     * @brief Creates a Session object.
     * @param username The user to login to the nextcloud server as.
     * @param serverRoot The root URL of the server without https:// or a path unless necessary for your server (example: cloud.example.com, example.com/cloud).
     * @param password The password for your login. If you have Two-Factor Authentication enabled this must be an app password.
     * @return A shared pointer of the new Session object. This shared pointer gets copied to every child instance of API_Implementor.
     */
    static std::shared_ptr<Session> create(const std::string& username, const std::string& serverRoot, const std::string& password);

    /**
     * @brief Creates a Session object.
     * @param federatedID The federated ID of the user to login as (example: user@cloud.example.com).
     * @param password The password for your login. If you have Two-Factor Authentication enabled this must be an app password.
     * @return A shared pointer of the new Session object. This shared pointer gets copied to every child instance of API_Implementor.
     */
    static std::shared_ptr<Session> create(const std::string& federatedID, const std::string& password);

    /**
     * @brief Gets all of the instances of the requested class.
     * Redeclare ncpass::API_Implementor::getAll() as public
     * @return A vector containing all currently active instances.
     * @see ncpass::API_Implementor::getAll()
     */
    static std::vector<std::shared_ptr<Session>> getAll();

    template <class API_Type>
    friend class API_Implementor;
};


}
