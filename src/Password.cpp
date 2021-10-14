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

#include <chrono>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <nlohmann/json.hpp>
#include <Password.hpp>
#include "API_Implementor.cpp"
#include "utils.cpp"


namespace ncpass
{


Password::Password(const std::shared_ptr<Session>& session, const nlohmann::json& password_json) :
    _Base(session, "password"),
    _json(password_json)
{
    if( !_json.contains("id") )
    {
#ifndef NDEBUG
        assert(_json.contains("password") && _json.contains("label"));
#endif

        //TODO: Remove read only properties.

        if( !_json.contains("hash") )
            _json["hash"] = utils::SHA1(_json.at("password"));

        std::thread t1(
          [passwd = std::shared_ptr<Password>(this), apiLock = std::unique_lock(_apiMutex)] (nlohmann::json json_bak) {
              std::this_thread::sleep_for(std::chrono::milliseconds(250));


              std::unique_lock memberLock(passwd->_memberMutex);
              {
                  const nlohmann::json emptyJSON = nlohmann::json::parse("{}");
                  nlohmann::json currentPatch    = nlohmann::json::diff(emptyJSON, json_bak);

                  for( nlohmann::json& patch : passwd->_jsonPushQueue )
                  {
                      for( nlohmann::json& op : patch )
                      {
                          for( nlohmann::json& currentOP : currentPatch )
                          {
                              if( op.at("path") == currentOP.at("path") )
                                  goto applyCurrentPatch;
                          }
                      }

                      for( nlohmann::json& op : patch )
                          currentPatch.push_back(op);

                      passwd->_jsonPushQueue.pop_front();
                  }

                applyCurrentPatch:

                  json_bak = emptyJSON.patch(currentPatch);
              }
              memberLock.unlock();


              nlohmann::json json_new = passwd->apiCall(POST, "create", json_bak);


              if( json_new.contains("id") && json_new.contains("revision") )
              {
                  memberLock.lock();

                  passwd->_json["id"]       = json_new.at("id");
                  passwd->_json["revision"] = json_new.at("revision");

                  memberLock.unlock();
                  passwd->_updateConVar.notify_all();

                  passwd->registerInstance();
                  passwd->pull();
              }
              else
              {
                  //TODO: Implement failure action.
              }
          },
          _json
          );

        t1.detach();
    }
}


void Password::pull()
{
    std::thread t1([passwd = shared_from_this()] ()
      {
          std::unique_lock apiLock(passwd->_apiMutex, std::defer_lock);
          std::unique_lock memberLock(passwd->_memberMutex, std::defer_lock);
          std::lock(apiLock, memberLock);

          // Only pull once every 250 milliseconds.
          if( std::chrono::system_clock::now() > passwd->_lastSync + std::chrono::milliseconds(250) )
          {
              const std::string id = passwd->_json.at("id");

              memberLock.unlock();

              nlohmann::json json_new = passwd->apiCall(POST, "show", { { "id", id } }); // Actual pull here.

              // Verify that json_new is valid and not an error code.
              if( (json_new.value("id", "") == id) && json_new.contains("revision") )
              {
                  memberLock.lock();

                  // apply all pending patches to the new json
                  for( nlohmann::json& patch : passwd->_jsonPushQueue )
                      json_new = json_new.patch(patch);

                  // detect changes
                  if( passwd->_json != json_new )
                  {
                      // If there are no pending patches to push or the current JSON doesn't contain "revision" (meaning it's the first pull) or the 2 objects have the same "revision" UUID then write new json.
                      if( passwd->_jsonPushQueue.empty() || !passwd->_json.contains("revision") || (passwd->_json.at("revision") == json_new.at("revision")) )
                      {
                          passwd->_json = json_new;
                          passwd->setPopulated();
                      }
                      // If none of the above then we have a conflict.
                      else
                      {
                          //TODO: Register conflict here.
                      }
                  }

                  passwd->_lastSync = std::chrono::system_clock::now();

                  passwd->_updateConVar.notify_all();
              }
              else
              {
                  //TODO: Implement failure action.
              }
          }
      }
      );


    t1.detach();
}


void Password::push() {}


void Password::sync() { pull(); }


void Password::wait()
{
    std::unique_lock apiLock(_apiMutex);


    _updateConVar.wait(
      apiLock, [this]
      {
          std::shared_lock memberLock(_memberMutex);

          return _jsonPushQueue.empty();
      }
      );
}


std::shared_ptr<Password> Password::create(const std::shared_ptr<Session>& session, const std::string& label, const std::string& password)
{
    nlohmann::json json;


    json["password"] = password;
    json["label"]    = label;

    return (new Password(session, json))->shared_from_this();
}


std::shared_ptr<Password> Password::fetch(const std::shared_ptr<Session>& session, const std::string& id)
{
    nlohmann::json json;


    json["id"] = id;

    std::shared_ptr<Password> toReturn = (new Password(session, json))->registerInstance();


    toReturn->pull();

    return toReturn;
}


std::vector<std::shared_ptr<Password>> Password::getAll() { return _Base::getRegistered(); }


std::string Password::getID() const
{
    std::shared_lock lock(_memberMutex);


    _updateConVar.wait(lock, [this] { return _json.contains("id"); });

    return _json.at("id");
}


std::string Password::getLabel() const
{
    std::shared_lock lock(_memberMutex);


    _updateConVar.wait(lock, [this] { return _json.contains("label"); });

    return _json.at("label");
}


void Password::setLabel(const std::string& label)
{
    std::unique_lock lock(_memberMutex);
    nlohmann::json   json_bak = _json;


    _json["label"] = label;
    _jsonPushQueue.push_back(nlohmann::json::diff(json_bak, _json));
    _updateConVar.notify_all();

    push();
}


std::string Password::getUsername() const
{
    std::shared_lock lock(_memberMutex);


    _updateConVar.wait(lock, [this] { return _json.contains("username"); });

    return _json.at("username");
}


void Password::setUsername(const std::string& username)
{
    std::unique_lock lock(_memberMutex);
    nlohmann::json   json_bak = _json;


    _json["username"] = username;
    _jsonPushQueue.push_back(nlohmann::json::diff(json_bak, _json));
    _updateConVar.notify_all();

    push();
}


std::string Password::getPassword() const
{
    std::shared_lock lock(_memberMutex);


    _updateConVar.wait(lock, [this] { return _json.contains("password"); });

    return _json.at("password");
}


void Password::setPassword(const std::string& password)
{
    std::unique_lock lock(_memberMutex);
    nlohmann::json   json_bak = _json;


    _json["password"] = password;
    _json["hash"]     = utils::SHA1(password);
    _jsonPushQueue.push_back(nlohmann::json::diff(json_bak, _json));
    _updateConVar.notify_all();

    push();
}


}
