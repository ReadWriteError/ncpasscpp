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

#include <atomic>
#include <API_Implementor.hpp>
#include <curl/curl.h>
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

    static std::atomic_flag notFirstSession = false;


    if( !notFirstSession.test_and_set() )
        curl_global_init(CURL_GLOBAL_ALL);
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
nlohmann::json API_Implementor<API_Type>::apiCall(Methods method, const std::string& apiAction, const nlohmann::json& apiArgs)
{
    CURL*    curl;
    CURLcode res;


    curl = curl_easy_init();

    if( curl )
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, strMethods[method]);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(NULL, "Content-Type: application/json"));

        const std::string postFields = apiArgs.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        std::string buffer;
        curl_easy_setopt(
          curl, CURLOPT_WRITEFUNCTION, +[] (void* contents, size_t size, size_t nmemb, void* userp)
            {
                ((std::string*)userp)->append((char*)contents, size * nmemb);

                return size * nmemb;
            }
          );
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        std::shared_lock<std::shared_mutex> lock(k_session._mutex);

        curl_easy_setopt(curl, CURLOPT_URL,      (k_session.k_apiURL + k_apiPath + apiAction).c_str());
        curl_easy_setopt(curl, CURLOPT_USERNAME, k_session.k_username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, k_session._password.c_str());

        res = curl_easy_perform(curl);

        lock.unlock();

        curl_easy_cleanup(curl);

        nlohmann::json json = nlohmann::json::parse(buffer);

        return json;
    }

    return "";
}


template <class API_Type>
API_Implementor<API_Type>::~API_Implementor()
{}


}
