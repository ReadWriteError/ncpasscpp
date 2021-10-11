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

#include <future>
#include <memory>
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
    _json.erase("status");

    if( _json.contains("id") )
    {
        pull();

        std::thread t1(
          [passwd = std::shared_ptr<Password>(this)] () {
              std::shared_lock<std::shared_mutex> lock(passwd->_memberMutex);
              passwd->_apiConVar.wait(lock, [&passwd] { return passwd->_json.contains("status"); });
              passwd->registerInstance();
          }
          );

        t1.detach();
    }
    else
    {
#ifndef NDEBUG
        assert(_json.contains("password") && _json.contains("label"));
#endif

        std::thread t1(
          [passwd = std::shared_ptr<Password>(this)] () {
              std::this_thread::sleep_for(std::chrono::milliseconds(250));
              // TODO: create api here
              passwd->_apiConVar.notify_all();
              passwd->registerInstance();
          }
          );

        t1.detach();
    }
}


std::shared_ptr<Password> Password::create(const std::shared_ptr<Session>& session, const std::string& label, const std::string& password)
{
    nlohmann::json json;


    json["password"] = password;
    json["label"]    = label;

    Password* toReturn = new Password(session, json);


    return toReturn->shared_from_this();
}


std::shared_ptr<Password> Password::get(const std::shared_ptr<Session>& session, const std::string& id)
{
    nlohmann::json json;


    json["id"] = id;

    auto toReturn = new Password(session, json);


    return toReturn->shared_from_this();
}


std::vector<std::shared_ptr<Password>> Password::getAllKnown() { return _Base::getRegistered(); }


void Password::sync() { pull(); }


void Password::pull()
{
    std::thread t1([this] ()
      {
          std::unique_lock<std::shared_mutex> lock(_memberMutex);

          nlohmann::json json_bak = _json;
          nlohmann::json json_new;

          _apiConVar.wait(lock, [this] { return _apiMutex.try_lock(); });
          lock.unlock();

          json_new = ncPOST("show", { { "id", json_bak.at("id") } });

          lock.lock();
          _apiMutex.unlock();

          //TODO: conflict handling?
          if( json_bak != _json )
              json_new.merge_patch(nlohmann::json::diff(json_bak, _json));

          if( json_new.value("id", "") == _json.at("id") )
          {
              _json     = json_new;
              _lastSync = std::chrono::system_clock::now();
          }

          _apiConVar.notify_all();
      }
      );


    t1.detach();
}


void Password::push() {}


std::string Password::getID() const
{
    std::shared_lock<std::shared_mutex> lock(_memberMutex);


    _apiConVar.wait(lock, [this] { return _json.contains("id"); });

    return _json.at("id");
}


std::string Password::getLabel() const
{
    std::shared_lock<std::shared_mutex> lock(_memberMutex);


    _apiConVar.wait(lock, [this] { return _json.contains("label"); });

    return _json.at("label");
}


void Password::setLabel(const std::string& label)
{
    std::unique_lock<std::shared_mutex> lock(_memberMutex);


    _json["label"] = label;
    push();
}


std::string Password::getUsername() const
{
    std::shared_lock<std::shared_mutex> lock(_memberMutex);


    _apiConVar.wait(lock, [this] { return _json.contains("username"); });

    return _json.at("username");
}


void Password::setUsername(const std::string& username)
{
    std::unique_lock<std::shared_mutex> lock(_memberMutex);


    _json["username"] = username;
    push();
}


std::string Password::getPassword() const
{
    std::shared_lock<std::shared_mutex> lock(_memberMutex);


    _apiConVar.wait(lock, [this] { return _json.contains("password"); });

    return _json.at("password");
}


void Password::setPassword(const std::string& password)
{
    std::unique_lock<std::shared_mutex> lock(_memberMutex);


    _json["password"] = password;
    push();
}


}
