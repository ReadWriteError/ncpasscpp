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

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <API_Implementor.hpp>
#include <nlohmann/json.hpp>

namespace ncpass
{




/**
 * @brief Password objects tied to the nextcloud server.
 * Changes to local Password objects will always be asynchronously synced to the server. The same is not true for changes to the remote password object. So call ncpass::Password::sync() often.
 * @see Password::sync()
 * @author Reed Krantz
 */
class NCPASSCPP_PUBLIC Password : public API_Implementor<Password>
{
  private:

    std::chrono::system_clock::time_point _lastSync; ///< The last time this password was synced with the server.
    nlohmann::json _json;                            ///< The JSON for the password.
    std::deque<nlohmann::json> _jsonPushQueue;       ///< The queue for json patches to be sent to the server.

    mutable std::shared_mutex           _memberMutex; ///< The mutex used to lock any member variables of this instance.
    mutable std::mutex                  _apiMutex;    ///< The mutex used to prevent 2 simultanious api calls. Never allow this to wait while you have a lock on _memberMutex or you will have a deadlock.
    mutable std::condition_variable_any _updateConVar;   ///< Used whenever the password is updated in any way.


  protected:

    /**
     * @brief Creates a new password or links to an existing remote password.
     * @param session An active Nextcloud session.
     * @param password_json A JSON object containing "id" to link with an existing password or "label" and "password" to create a new one.
     * @see ncpass::Session
     */
    Password(const std::shared_ptr<Session>& session, const nlohmann::json& password_json);

    /**
     * @brief Pulls data from the server.
     */
    void pull();

    /**
     * @brief Pushes data to the server. This only updates the server's data if the local data is a newer version.
     */
    void push();


  public:

    /**
     * @brief Pulls/pushes the most recent data from/to the server.
     */
    void sync();

    /**
     * @brief Blocks the thread and waits for all pending changes to be pushed and any current API call to be completed.
     */
    void wait();

    /**
     * @brief Creates a new password.
     * @param session A shared_ptr to a ncpass::Session instance. Used as credentials for the Nextcloud server's API.
     * @param label The label of the Password that you're creating.
     * @param password The password of the Password you're creating.
     * @return A shared_ptr to the ncpass::Password instance.
     * @see ncpass::Session
     */
    static std::shared_ptr<Password> create(const std::shared_ptr<Session>& session, const std::string& label, const std::string& password);

    /**
     * @brief Fetches a Password from the server based on the given ID.
     * @param session A shared_ptr to a ncpass::Session instance. Used as credentials for the Nextcloud server's API.
     * @param id The ID of an existing password on the nextcloud server.
     * @return A shared_ptr to the ncpass::Password instance of the given ID.
     * @see ncpass::Session
     */
    static std::shared_ptr<Password> fetch(const std::shared_ptr<Session>& session, const std::string& id);

    /**
     * @brief Gets all the passwords from the given Nextcloud server session asynchronously.
     * @param session A shared_ptr to a ncpass::Session instance. Used as credentials for the Nextcloud server's API.
     * @see ncpass::Session
     */
    static void fetchAll(const std::shared_ptr<Session>& session);

    /**
     * @brief Gets all of the active instances of the Password class.
     * This is a local only actionand does not create or sync with new passwords in nextcloud.
     * @return A vector containing all currently active instances.
     */
    static std::vector<std::shared_ptr<Password>> getAll();

    /**
     * @return The UUID of the password.
     */
    std::string getID() const;

    /**
     * @return User defined label of the password.
     */
    std::string getLabel() const;

    /**
     * @brief Set the passwords label asynchronously.
     * @param label User defined label of the password.
     */
    void setLabel(const std::string& label);

    /**
     * @return Username associated with the password.
     */
    std::string getUsername() const;

    /**
     * @brief Set the passwords username asynchronously.
     * @param username Username associated with the password.
     */
    void setUsername(const std::string& username);

    /**
     * @return The actual password.
     */
    std::string getPassword() const;

    /**
     * @brief Set the passwords password asynchronously.
     * @param password The actual password.
     */
    void setPassword(const std::string& password);
};


}
