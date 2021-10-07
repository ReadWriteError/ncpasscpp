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
#include <internal/FuturePromise.hpp>
#include <nlohmann/json.hpp>
#include <Password.hpp>
#include "API_Implementor.cpp"


namespace ncpass
{


Password::Password(const std::shared_ptr<Session>& session, const nlohmann::json& password_json, std::shared_ptr<internal::FuturePromise<void>> futProm) :
    _Base(session, "password", futProm),
    _json(password_json)
{
    if( _json.contains("id") )
    {
        pull();

        if( futProm.use_count() == 1 )
        {
            std::cout << "password: releasing base lock" << std::endl;
            futProm->getPromise().set_value();
        }
    }
    else
    {
        assert(_json.contains("password") && _json.contains("label"));

        std::unique_lock<std::shared_mutex> lock(_mutex);

        if( futProm.use_count() == 1 )
        {
            std::cout << "password: releasing base lock" << std::endl;
            futProm->getPromise().set_value();
        }
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
    for( auto password : getAllLocal() )
    {
        if( password->getID() == id )
            return password;
    }

    nlohmann::json json;


    json["id"] = id;

    Password* toReturn = new Password(session, json);


    return toReturn->shared_from_this();
}


std::vector<std::shared_ptr<Password>> Password::getAllLocal() { return _Base::getAllLocal(); }


void Password::sync() { pull(); }


void Password::pull()
{
    nlohmann::json json = ncPOST("show", { { "id", getID() } });
    std::unique_lock<std::shared_mutex> lock(_mutex);


    if( json.value("id", "") == _json.at("id") ) // Will not throw if new json doesn't have an ID but will if current json doesn't.
    {
        _json     = json;
        _lastSync = std::chrono::system_clock::now();
    }
}


void Password::push() {}


std::string Password::getID() const
{
    return _json.at("id");
}


std::string Password::getLabel()
{
    if( !_json.contains("label") )
        pull();


    return _json.at("label");
}


void Password::setLabel(const std::string& label)
{
    _json["label"] = label;
    push();
}


std::string Password::getUsername()
{
    if( !_json.contains("username") )
        sync();


    return _json.at("username");
}


void Password::setUsername(const std::string& username)
{
    _json["username"] = username;
    push();
}


std::string Password::getPassword()
{
    if( !_json.contains("password") )
        sync();


    return _json.at("password");
}


void Password::setPassword(const std::string& password)
{
    _json["password"] = password;
    push();
}


}
