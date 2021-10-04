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
std::vector<std::shared_ptr<API_Type>> API_Implementor<API_Type>::s_allInstances = std::vector<std::shared_ptr<API_Type>>();

template <class API_Type>
API_Implementor<API_Type>::API_Implementor(const std::shared_ptr<Session>& session, const std::string& apiPath) :
    k_session(session),
    k_apiPath(apiPath + "/")
{
    // Compile time check that API_Type has a base of API_Implementor.
    static_assert(
      std::is_base_of<API_Implementor, API_Type>::value,
      "API_Implementor<class API_Type> API_Type must be a child of API_Implementor. Check docs for how to use this."
      );

    // Add new instance to the static vector. Designed to cause runtime error on object creation if you do the inheritance wrong.
    s_allInstances.push_back(std::shared_ptr<API_Type>(static_cast<API_Type*>(this)));
}


template <class API_Type>
API_Implementor<API_Type>::API_Implementor(const API_Implementor& apiObject, const std::string& apiPath) :
    API_Implementor<API_Type>(apiObject.k_session, apiPath)
{}

// Constructor specific to the Session class.
template <class API_Type>
API_Implementor<API_Type>::API_Implementor(const std::string& apiPath) :
    k_session(std::shared_ptr<API_Type>(static_cast<API_Type*>(this))),
    k_apiPath(apiPath + "/")
{
    static_assert(std::is_base_of<Session, API_Type>::value, "API_Implementor(const std::string& apiPath) can only be called from Session");
    // Add new instance to the static vector.
    s_allInstances.push_back(k_session);
}


template <class API_Type>
std::vector<std::shared_ptr<API_Type>> API_Implementor<API_Type>::getAllLocal() { return s_allInstances; }


template <class API_Type>
std::shared_ptr<API_Type> API_Implementor<API_Type>::getSharedPtr()
{
    for( std::shared_ptr<API_Type> ptr : s_allInstances )
        if( ptr.get() == this )
            return ptr;

}


template <class API_Type>
template <unsigned int N>
nlohmann::json API_Implementor<API_Type>::ncPOST(const std::string& apiAction, const std::string (& apiArgs)[N][2])
{
    curlpp::Cleanup cleaner;
    curlpp::Easy    request;

    nlohmann::json json;


    request.setOpt(new curlpp::options::Url(k_session->k_apiURL + k_apiPath + apiAction));
    request.setOpt(k_session->k_usrPasswd);

    curlpp::Forms formParts;


    for( unsigned int i = 0; i < N; i++ )
        formParts.push_back(new curlpp::FormParts::Content(apiArgs[i][0], apiArgs[i][1]));

    request.setOpt(new curlpp::options::HttpPost(formParts));

    std::stringstream output;


    output << request;

    json = nlohmann::json::parse(output.str());

    return json;
}


}
