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
#include <sstream>
#include <string>
#include <type_traits>
#include <API_Implementor.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Form.hpp>
#include <curlpp/Options.hpp>
#include <nlohmann/json.hpp>
#include <Session.hpp>


namespace ncpass
{


template <class API_Type>
std::vector<std::shared_ptr<API_Type>> API_Implementor<API_Type>::s_activeInstances;
template <class API_Type>
std::vector<std::shared_ptr<API_Type>> API_Implementor<API_Type>::s_creatingInstances;
template <class API_Type>
std::vector<std::weak_ptr<API_Type>> API_Implementor<API_Type>::s_deletingInstances;

template <class API_Type>
std::shared_mutex API_Implementor<API_Type>::s_mutex;


template <class API_Type>
API_Implementor<API_Type>::API_Implementor(const std::shared_ptr<Session>& session, const std::string& apiPath) :
    k_lockbox(session),
    k_session(*session),
    k_apiPath(apiPath + "/")
{
    // Compile time check that API_Type has a base of API_Implementor.
    static_assert(
      std::is_base_of<API_Implementor, API_Type>::value,
      "API_Implementor<class API_Type>: API_Type should be set to your child class."
      );
}


template <class API_Type>
API_Implementor<API_Type>::API_Implementor(const API_Implementor& apiObject, const std::string& apiPath) :
    k_lockbox(apiObject.k_lockbox),
    k_session(apiObject.k_session),
    k_apiPath(apiPath + "/")
{
    // Compile time check that API_Type has a base of API_Implementor.
    static_assert(
      std::is_base_of<API_Implementor, API_Type>::value,
      "API_Implementor<class API_Type>: API_Type should be set to your child class."
      );
}


template <class API_Type>
API_Implementor<API_Type>::API_Implementor(const std::string& apiPath) :
    k_lockbox(std::shared_ptr<Session>()),
    k_session(static_cast<Session&>(*this)),
    k_apiPath(apiPath + "/")
{
    static_assert(std::is_base_of<Session, API_Type>::value, "API_Implementor(const std::string& apiPath) can only be called from Session");
}


template <class API_Type>
std::vector<std::shared_ptr<API_Type>> API_Implementor<API_Type>::getRegistered()
{
    std::shared_lock<std::shared_mutex> lock(s_mutex);


    return s_activeInstances;
}


template <class API_Type>
std::shared_ptr<API_Type> API_Implementor<API_Type>::registerInstance()
{
    std::shared_ptr<API_Type> newInstance;
    auto id = this->getID();


    try
    {
        newInstance = this->shared_from_this();
    }
    catch( const std::bad_weak_ptr& e )
    {
        newInstance = std::shared_ptr<API_Type>(static_cast<API_Type*>(this));
    }

    std::unique_lock<std::shared_mutex> lock(s_mutex);


    // verify that the object to register doesn't already have a duplicate
    for( auto apiImp : s_creatingInstances )
    {
        if( apiImp->getID() == id )
            return apiImp;  // if it does return that instead.
    }

    for( auto apiImp : s_activeInstances )
    {
        if( apiImp->getID() == id )
            return apiImp;
    }

    for( auto itr = s_deletingInstances.begin(); itr != s_deletingInstances.end(); itr++ )
    {
        if( auto apiImp = itr->lock() )
        {
            if( apiImp->getID() == id )
                return apiImp;
        }
        else
        {
            s_deletingInstances.erase(itr);
            itr--;
        }
    }

    // Add new instance to the static vector.
    s_activeInstances.push_back(newInstance);

    return newInstance;
}


template <class API_Type>
bool API_Implementor<API_Type>::setPopulated()
{
    auto thisPtr = this->shared_from_this();

    std::unique_lock<std::shared_mutex> lock(s_mutex);


    for( auto itr = s_creatingInstances.begin(); itr != s_creatingInstances.end(); itr++ )
    {
        if( *itr == thisPtr )
        {
            s_activeInstances.push_back(*itr);
            s_creatingInstances.erase(itr);

            return true;
        }
    }

    return false;
}


template <class API_Type>
bool API_Implementor<API_Type>::unregisterInstance()
{
    auto thisPtr = this->shared_from_this();

    std::unique_lock<std::shared_mutex> lock(s_mutex);


    for( auto currentVector : { &s_activeInstances, &s_creatingInstances } )
    {
        for( auto itr = currentVector.begin(); itr != currentVector.end(); itr++ )
        {
            if( *itr == thisPtr )
            {
                s_deletingInstances.push_back(std::weak_ptr(*itr));
                currentVector.erase(itr);

                return true;
            }
        }
    }

    return false;
}


template <class API_Type>
nlohmann::json API_Implementor<API_Type>::ncPOST(const std::string& apiAction, const nlohmann::json& apiArgs)
{
    curlpp::Cleanup cleaner;
    curlpp::Easy    request;

    nlohmann::json json;

    std::shared_lock<std::shared_mutex> lock(k_session._mutex);


    request.setOpt(new curlpp::options::Url(k_session.k_apiURL + k_apiPath + apiAction));
    request.setOpt(k_session._usrPasswd);

    curlpp::Forms formParts;


    for( auto&[key, value] : apiArgs.items() )
        formParts.push_back(new curlpp::FormParts::Content(key, value));

    request.setOpt(new curlpp::options::HttpPost(formParts));

    std::stringstream output;


    output << request;

    lock.unlock();

    json = nlohmann::json::parse(output.str());

    return json;
}


template <class API_Type>
API_Implementor<API_Type>::~API_Implementor()
{}


}
