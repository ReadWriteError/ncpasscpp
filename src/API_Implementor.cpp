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

#include <API_Implementor.hpp>
#include <Session.hpp>


namespace ncpass
{


template <class API_Type>
std::vector<std::shared_ptr<API_Type>> API_Implementor<API_Type>::s_allInstances = std::vector<std::shared_ptr<API_Type>>();

template <class API_Type>
API_Implementor<API_Type>::API_Implementor(const std::shared_ptr<Session>& session) :
    k_session(session)
{
    // Compile time check that API_Type has a base of API_Implementor.
    static_assert(std::is_base_of<API_Implementor, API_Type>::value, "API_Implementor<class API_Type> API_Type must be a child of API_Implementor. Check docs for how to use this.");

    // Add new instance to the static vector. Designed to cause runtime error on object creation if you do the inheritance wrong.
    s_allInstances.push_back(std::shared_ptr<API_Type>(static_cast<API_Type*>(this)));
}


template <class API_Type>
API_Implementor<API_Type>::API_Implementor(const API_Implementor& apiObject) :
    API_Implementor<API_Type>(apiObject.k_session)
{}


template <class API_Type>
std::shared_ptr<API_Type> API_Implementor<API_Type>::getSharedPtr()
{
    for( std::shared_ptr<API_Type> ptr : s_allInstances )
        if( ptr.get() == this )
            return ptr;

}


template <class API_Type>
std::vector<std::shared_ptr<API_Type>> API_Implementor<API_Type>::getAll() { return s_allInstances; }

// specific to the Session class
template <>
API_Implementor<Session>::API_Implementor() :
    k_session(std::shared_ptr<Session>(static_cast<Session*>(this)))
{
    // Add new instance to the static vector.
    s_allInstances.push_back(k_session);
}


}
