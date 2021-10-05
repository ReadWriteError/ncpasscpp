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

#include <iostream>
#include <Password.hpp>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <Session.hpp>
#include "getDbusIDPass.cpp"
#include "user-specific.hpp"

using namespace std;


int main(int argc, char** argv)
{
    if( argc != 1 )
    {
        cout << argv[0] << " takes no arguments.\n";

        return 1;
    }

    // Disable access to this processes ram from non root users.
    // This line needs to be commented out if you want to use a debugger.
    prctl(PR_SET_DUMPABLE, false);

    // Lock all the memory used by this program in ram so that they will never be writen to disk.
    // DISCLAMER: this does not stop memory being writen to the disk during hybernation.
    mlockall(MCL_CURRENT | MCL_FUTURE | MCL_ONFAULT);

    shared_ptr<ncpass::Session> session; // The Nextcloud Session to use for this test


    // How you get your "federatedIDs and passwords" or "usernames, rootServerURLs and passwords" is up to you and your application of this lib.
    // I'm getting this from the Gnome Online Accounts daemon over dbus.
    // You can look in getDbusIDPass.hpp for how I do this (its kinda complicated).
    for( array<string, 2> idPassArr : getDbusIDPass() ) // Gets a vector of string arrays containing { {federatedID, password}, {federatedID, password} } and loops through it.
    {
        if( idPassArr[0] == TEST_PASSWORD_ACCOUNT ) // If the federatedID of this account is equal to our user specific variable.
            session = ncpass::Session::create(idPassArr[0], idPassArr[1]);  // Create the account Session.
    }

    auto password = ncpass::Password::get(session, TEST_PASSWORD_UUID); // Create our password.

    bool didAllPass = true;
    bool tests[]    = { // All the tests to perform.
        password->getID() == TEST_PASSWORD_UUID,
        password->getLabel() == TEST_PASSWORD_LABEL,
        password->getUsername() == TEST_PASSWORD_USERNAME,
        password->getPassword() == TEST_PASSWORD_PASSWORD
    };


    // Print the status of those tests.
    cout << "ID pass?       " << tests[0] << endl;
    cout << "Label pass?    " << tests[1] << endl;
    cout << "Username pass? " << tests[2] << endl;
    cout << "Password pass? " << tests[3] << endl;


    // check if every test passed and change didAllPass if they didn't
    for( unsigned int i = 0; didAllPass && i < sizeof(tests) / sizeof(tests[0]); i++ )
        didAllPass = tests[i];

    return !didAllPass;
}
