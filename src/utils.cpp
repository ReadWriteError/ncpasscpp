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

#pragma once
#include <iomanip>
#include <sstream>
#include <string>
#include <openssl/sha.h>


namespace ncpass::utils
{


static std::string SHA1(const std::string& input)
{
    unsigned char pwdHash[SHA_DIGEST_LENGTH];


    ::SHA1((const unsigned char*)input.c_str(), input.size(), pwdHash);

    std::stringstream sstring;


    sstring << std::hex << std::setfill('0');

    for( unsigned int i = 0; i < SHA_DIGEST_LENGTH; i++ )
        sstring << std::setw(2) << static_cast<unsigned>(pwdHash[i]);

    return sstring.str();
}


}
