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
#include <future>
#include <memory>
#include <mutex>
#include <ratio>
#include <shared_mutex>
#include <thread>
#include <nlohmann/json.hpp>
#include <Password.hpp>
#include "API_Implementor.cpp"


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

        _json["hash"] = _json.at("password");

        std::thread t1(
          [passwd = std::shared_ptr<Password>(this)] () {
              std::this_thread::sleep_for(std::chrono::milliseconds(250));

              std::unique_lock memberLock(passwd->_memberMutex, std::defer_lock);
              std::unique_lock apiLock(passwd->_apiMutex, std::defer_lock);
              std::lock(apiLock, memberLock);

              passwd->_json = passwd->_json.patch(passwd->_jsonPushQueue.front());
              passwd->_jsonPushQueue.pop();

              memberLock.unlock();

              nlohmann::json json_new = passwd->ncPOST("create", passwd->_json);

              if( json_new.contains("id") && json_new.contains("revision") )
              {
                  passwd->_json["id"]       = json_new.at("id");
                  passwd->_json["revision"] = json_new.at("revision");

                  passwd->_apiConVar.notify_all();
                  passwd->registerInstance();
                  passwd->pull();
              }
              else
              {
                  //TODO: Implement failure action.
              }
          }
          );

        t1.detach();
    }
}


void Password::pull()
{
    std::thread t1([passwd = shared_from_this()] ()
      {
          std::unique_lock memberLock(passwd->_memberMutex, std::defer_lock);
          std::unique_lock apiLock(passwd->_apiMutex, std::defer_lock);
          std::lock(apiLock, memberLock);

          if( std::chrono::system_clock::now() > passwd->_lastSync + std::chrono::milliseconds(250) )
          {
              memberLock.unlock();

              nlohmann::json json_new = passwd->ncPOST("show", { { "id", passwd->_json.at("id") } });

              if( json_new.value("id", "") == passwd->_json.at("id") )
              {
                  json_new = nlohmann::json::diff(passwd->_json, json_new);

                  memberLock.lock();

                  if( !json_new.empty() ) //TODO: Detect new revisions instead of changes?
                  {
                      if( passwd->_jsonPushQueue.empty() || !passwd->_json.contains("revision") )
                      {
                          passwd->_json = passwd->_json.patch(json_new);
                          passwd->setPopulated();
                      }
                      else
                      {
                          //TODO: Register conflict here.
                      }
                  }

                  passwd->_lastSync = std::chrono::system_clock::now();

                  passwd->_apiConVar.notify_all();
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


std::shared_ptr<Password> Password::create(const std::shared_ptr<Session>& session, const std::string& label, const std::string& password)
{
    nlohmann::json json;


    json["password"] = password;
    json["label"]    = label;

    return (new Password(session, json))->shared_from_this();
}


std::shared_ptr<Password> Password::get(const std::shared_ptr<Session>& session, const std::string& id)
{
    nlohmann::json json;


    json["id"] = id;

    std::shared_ptr<Password> toReturn = (new Password(session, json))->registerInstance();


    toReturn->pull();

    return toReturn;
}


std::vector<std::shared_ptr<Password>> Password::getAllKnown() { return _Base::getRegistered(); }


std::string Password::getID() const
{
    std::shared_lock lock(_memberMutex);


    _apiConVar.wait(lock, [this] { return _json.contains("id"); });

    return _json.at("id");
}


std::string Password::getLabel() const
{
    std::shared_lock lock(_memberMutex);


    _apiConVar.wait(lock, [this] { return _json.contains("label"); });

    return _json.at("label");
}


void Password::setLabel(const std::string& label)
{
    std::unique_lock lock(_memberMutex);


    _json["label"] = label;
    push();
}


std::string Password::getUsername() const
{
    std::shared_lock lock(_memberMutex);


    _apiConVar.wait(lock, [this] { return _json.contains("username"); });

    return _json.at("username");
}


void Password::setUsername(const std::string& username)
{
    std::unique_lock lock(_memberMutex);


    _json["username"] = username;
    push();
}


std::string Password::getPassword() const
{
    std::shared_lock lock(_memberMutex);


    _apiConVar.wait(lock, [this] { return _json.contains("password"); });

    return _json.at("password");
}


void Password::setPassword(const std::string& password)
{
    std::unique_lock lock(_memberMutex);


    _json["password"] = password;
    push();
}


}
