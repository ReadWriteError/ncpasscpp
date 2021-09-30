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
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>


namespace ncpass
{


template <class API_Type>
API_Implementor<API_Type>::API_Implementor(const std::shared_ptr<ServerCredentials>& serverCredentials) :
    _serverCredentials(serverCredentials)
{
    BOOST_STATIC_ASSERT(boost::is_base_of<API_Implementor, API_Type>::value);
    s_allInstances.push_back(std::shared_ptr<API_Type>(static_cast<API_Type*>(this)));
}


template <class API_Type>
API_Implementor<API_Type>::API_Implementor(const API_Implementor& apiObject) :
    API_Implementor<API_Type>(apiObject._serverCredentials)
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


}
