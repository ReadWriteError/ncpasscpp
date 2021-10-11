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

#include <memory>
#include <Session.hpp>
#include "API_Implementor.cpp"

namespace ncpass
{


Session::Session(const std::string& username, const std::string& serverRoot, const std::string& password) :
    _Base("session"),
    k_apiURL("https://" + serverRoot + (serverRoot.back() != '/' ? "/" : "") + "apps/passwords/api/1.0/"),
    k_federatedID(username + "@" + (serverRoot.back() != '/' ? serverRoot : serverRoot.substr(0, serverRoot.size() - 1))),
    _usrPasswd(username + ":" + password)
{}


std::shared_ptr<Session> Session::create(const std::string& username, const std::string& serverRoot, const std::string& password)
{
    return (new Session(username, serverRoot, password))->registerInstance();
}


std::shared_ptr<Session> Session::create(const std::string& federatedID, const std::string& password)
{
    return create(federatedID.substr(0, federatedID.find('@')), federatedID.substr(federatedID.find('@') + 1, federatedID.size()), password);
}


std::vector<std::shared_ptr<Session>> Session::getAllKnown() { return _Base::getRegistered(); }


std::string Session::getID() const { return k_federatedID; }


}
