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
std::vector<std::shared_ptr<API_Type>> API_Implementor<API_Type>::s_allInstances;

template <class API_Type>
std::shared_mutex API_Implementor<API_Type>::s_mutex;


template <class API_Type>
void API_Implementor<API_Type>::addToVector(std::shared_ptr<internal::FuturePromise<void>>& futProm)
{
    // lock write access to s_allInstances
    std::unique_lock<std::shared_mutex> lock(s_mutex);


    // Add new instance to the static vector. Designed to cause runtime error on object creation if you do the inheritance wrong.
    s_allInstances.push_back(std::shared_ptr<API_Type>(static_cast<API_Type*>(this)));


    assert(futProm.use_count() > 1);

    std::thread t1(
      [](__attribute__ ((unused)) std::unique_lock<std::shared_mutex> lock, std::shared_future<void> fut) {
          std::cout << "waiting for notification" << std::endl;
          fut.wait();
          std::cout << "base unlocked" << std::endl;
      },
      std::move(lock),
      futProm->getFuture()
      );


    t1.detach();
}


template <class API_Type>
API_Implementor<API_Type>::API_Implementor(const std::shared_ptr<Session>& session, const std::string& apiPath, std::shared_ptr<internal::FuturePromise<void>> futProm) :
    k_lockbox(session),
    k_session(*session),
    k_apiPath(apiPath + "/")
{
    // Compile time check that API_Type has a base of API_Implementor.
    static_assert(
      std::is_base_of<API_Implementor, API_Type>::value,
      "API_Implementor<class API_Type> API_Type must be a child of API_Implementor. Check docs for how to use this."
      );
    addToVector(futProm);
}


template <class API_Type>
API_Implementor<API_Type>::API_Implementor(const API_Implementor& apiObject, const std::string& apiPath, std::shared_ptr<internal::FuturePromise<void>> futProm) :
    k_lockbox(apiObject.k_lockbox),
    k_session(apiObject.k_session),
    k_apiPath(apiPath + "/")
{
    // Compile time check that API_Type has a base of API_Implementor.
    static_assert(
      std::is_base_of<API_Implementor, API_Type>::value,
      "API_Implementor<class API_Type> API_Type must be a child of API_Implementor. Check docs for how to use this."
      );
    addToVector(futProm);
}


template <class API_Type>
API_Implementor<API_Type>::API_Implementor(const std::string& apiPath, std::shared_ptr<internal::FuturePromise<void>> futProm) :
    k_lockbox(std::shared_ptr<Session>()),
    k_session(static_cast<Session&>(*this)),
    k_apiPath(apiPath + "/")
{
    static_assert(std::is_base_of<Session, API_Type>::value, "API_Implementor(const std::string& apiPath) can only be called from Session");
    addToVector(futProm);
}


template <class API_Type>
std::vector<std::shared_ptr<API_Type>> API_Implementor<API_Type>::getAllLocal()
{
    std::shared_lock<std::shared_mutex> lock(s_mutex);


    return s_allInstances;
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


}
