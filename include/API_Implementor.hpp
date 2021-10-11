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
#include <shared_mutex>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace ncpass
{




class Session; // forward declaration




/**
 * @brief Class used for API calls to the Nextcloud server.
 * Abstract class that needs to be inherited to provide access to the Nextcloud server's API. This is because this class is the **only** class that can access ncpass::Session.
 * @tparam API_Type The derived class you are creating (A.K.A. the type of this).
 * @see ncpass::Session
 * @author Reed Krantz
 */
template <class API_Type>
class NCPASSCPP_PUBLIC API_Implementor : public std::enable_shared_from_this<API_Type>
{
  private:

    /**
     * @brief Struct used to store the shared_ptr for the session.
     * This Struct is never accessed only initialized.
     * All required uses of the shared_ptr should use API_Implementor::k_session instead.
     * The shared_ptr is only stored so it's life can be preserved.
     * The reson this even exists is so ncpass::Session doesn't have to store a shared_ptr to itself as this is initialized to a nullptr in ncpass::Session dedicated constructor.
     * @see ncpass::API_Implementor::k_session
     */
    struct Lockbox
    {
      private:

        const std::shared_ptr<Session> k_session; ///< The life preserving shared_ptr for the object referenced by API_Implementor::k_session.


      public:

        Lockbox(const std::shared_ptr<Session>& session) : k_session(session)
        {}


        Lockbox(const Lockbox& lockbox) : k_session(lockbox.k_session)
        {}
    } const k_lockbox;

    const Session&    k_session; ///< Holds the Nextcloud server this instance is tied to.
    const std::string k_apiPath; ///< The path to append to the URL (example: ncpass::Password would be "password/").

    static std::vector<std::shared_ptr<API_Type>> s_activeInstances;   ///< Contains all the instances currently existing of a certain API_Type that are vaild.
    static std::vector<std::shared_ptr<API_Type>> s_creatingInstances; ///< Contains all the instances currently existing of a certain API_Type that are being created (constructing or first pull).
    static std::vector<std::weak_ptr<API_Type>>   s_deletingInstances; ///< Contains all the instances currently existing of a certain API_Type that are pending deletion.
    static std::shared_mutex s_mutex;                                  ///< Mutex used for locking access to static variables.


  protected:

    typedef API_Implementor<API_Type> _Base; ///< This class. Used for child classes.

    /**
     * @brief Constructor for providing the Nextcloud server's credentials.
     * @param session A shared_ptr to a ncpass::Session instance used as credentials for the Nextcloud server's API.
     * @param apiPath The path to append to the URL (example: ncpass::Password would be "password").
     * @see ncpass::Session
     * @see https://git.mdns.eu/nextcloud/passwords/wikis/Developers/Index
     */
    API_Implementor(const std::shared_ptr<Session>& session, const std::string& apiPath);

    /**
     * @brief Constructor that obtains the ncpass::Session instance from another object. This is done by copying the lockbox and reference within the object.
     * @param apiObject An ncpass::API_Implementor object that will be used to obtain the ncpass::Session.
     * @param apiPath The path to append to the URL (example: ncpass::Password would be "password").
     * @see ncpass::Session
     * @see ncpass::API_Implementor::Lockbox
     * @see https://git.mdns.eu/nextcloud/passwords/wikis/Developers/Index
     */
    API_Implementor(const API_Implementor& apiObject, const std::string& apiPath);

    /**
     * @brief Dedicated constructor for ncpass::Session because it is also an API_Implementor.
     * @param apiPath The path to append to the URL (example: ncpass::Password would be "password").
     * @see ncpass::Session
     * @see https://git.mdns.eu/nextcloud/passwords/wikis/Developers/Index
     */
    API_Implementor(const std::string& apiPath);

    /**
     * @brief Gets all of the instances of the derived class in API_Type that are currently active.
     * @return A copy of API_Implementor::s_allActiveInstances.
     */
    static std::vector<std::shared_ptr<API_Type>> getRegistered();

    /**
     * @brief Adds this instance to the pool of available instances.
     * @return Returns the instance just registered or the instance that is already registered with a matching ID.
     */
    std::shared_ptr<API_Type> registerInstance();

    /**
     * @brief Sets this instance as active meaning its fully populated.
     * @return True if the instance is set as populated. False if the instance is already populated.
     */
    bool setPopulated();

    /**
     * @brief Remove this instance from the active pool to prepare for its deletion.
     * @return True if successful. False if instance is not active.
     */
    bool unregisterInstance();

    /**
     * @brief Make a curl POST request.
     * @param apiAction The final part of the API URL.
     * @param apiArgs The arguments for the POST request in JSON. example JSON: { {"arg1", "value"}, {"arg2", "value"} }
     * @return The returning JSON of the call.
     */
    nlohmann::json ncPOST(const std::string& apiAction, const nlohmann::json& apiArgs);


  public:

    /**
     * @brief Virtual destructor so deleting an instance via a pointer doesn't result in undefined behavior.
     */
    virtual ~API_Implementor();

    /**
     * @brief Gets a unique id for this object.
     * This is usually tied to a value on the server.
     * @return A string that should not be the same as any other object
     */
    virtual std::string getID() const = 0;
};


}
