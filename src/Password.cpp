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

#include <nlohmann/json.hpp>
#include <Password.hpp>
#include "API_Implementor.cpp"


namespace ncpass
{


Password::Password(const std::shared_ptr<Session>& session, const nlohmann::json& password_json) : _Base(session, "password"), _lastSync(std::chrono::system_clock::now())
{
    setFromJSON(password_json);
}


void Password::setFromJSON(nlohmann::json password_json)
{
    std::map<std::string, std::string*> variableMaps;


    if( _id.empty() )
        variableMaps["id"] = &_id;

    variableMaps["label"]    = &_label;
    variableMaps["username"] = &_username;
    variableMaps["password"] = &_password;

    for( nlohmann::json::iterator it = password_json.begin(); it != password_json.end(); it++ )
        if( variableMaps.find(it.key()) != variableMaps.end() )
            *variableMaps[it.key()] = (std::string)it.value();

}


std::shared_ptr<Password> Password::create(const std::shared_ptr<Session>& session)
{
    Password* toReturn = new Password(session);


    return toReturn->getSharedPtr();
}


std::shared_ptr<Password> Password::get(const std::shared_ptr<Session>& session, const std::string& id)
{
    for( auto password : getAll() )
        if( password->_id == id )
            return password;

    nlohmann::json json;


    json["id"] = id;

    Password* toReturn = new Password(session, json);


    return toReturn->getSharedPtr();
}


void Password::sync() {}


void Password::pull()
{
    setFromJSON(ncPOST("show", { { "id", _id } }));
    _lastSync = std::chrono::system_clock::now();
}


void Password::push() const {}


std::string Password::getID() const { return _id; }


std::string Password::getLabel() const { return _label; }


void Password::setLabel(const std::string& label)
{
    _label = label;
    push();
}


std::string Password::getUsername() const { return _username; }


void Password::setUsername(const std::string& username)
{
    _username = username;
    push();
}


std::string Password::getPassword() const { return _password; }


void Password::setPassword(const std::string& password)
{
    _password = password;
    push();
}


}
