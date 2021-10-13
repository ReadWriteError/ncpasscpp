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


// Test for Password class
// purpose: Create a new password.

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
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

    // print "true"/"false" for bools
    cout << boolalpha;

    shared_ptr<ncpass::Session> session; // The Nextcloud Session to use for this test


    // How you get your "federatedIDs and passwords" or "usernames, rootServerURLs and passwords" is up to you and your application of this lib.
    // I'm getting this from the Gnome Online Accounts daemon over dbus.
    // You can look in getDbusIDPass.hpp for how I do this (its kinda complicated).
    for( array<string, 2> idPassArr : getDbusIDPass() ) // Gets a vector of string arrays containing { {federatedID, password}, {federatedID, password} } and loops through it.
    {
        if( idPassArr[0] == TEST_PASSWORD_2_ACCOUNT ) // If the federatedID of this account is equal to our user specific variable.
            session = ncpass::Session::create(idPassArr[0], idPassArr[1]);  // Create the account Session.
    }

    std::vector<bool> tests; // The results of all the tests.

    // Create our password.
    auto password = ncpass::Password::create(session, TEST_PASSWORD_2_LABEL, TEST_PASSWORD_2_PASSWORD);


    password->setUsername(TEST_PASSWORD_2_USERNAME);

    // Print the tests and add them to the vector.
    tests.push_back(false); cout << setw(TEST_WIDTH) << "ID pass? "                  << (tests.back() = (
      password->getID().size() == 36 &&
      password->getID()[8] == '-' &&
      password->getID()[13] == '-' &&
      password->getID()[18] == '-' &&
      password->getID()[23] == '-'
      )) << endl;
    tests.push_back(false); cout << setw(TEST_WIDTH) << "Label pass? "               << (tests.back() = password->getLabel() == TEST_PASSWORD_2_LABEL) << endl;
    tests.push_back(false); cout << setw(TEST_WIDTH) << "Username pass? "            << (tests.back() = password->getUsername() == TEST_PASSWORD_2_USERNAME) << endl;
    tests.push_back(false); cout << setw(TEST_WIDTH) << "Password pass? "            << (tests.back() = password->getPassword() == TEST_PASSWORD_2_PASSWORD) << endl;


    // return fail status if any tests failed
    for( bool test : tests )
    {
        if( !test )
            return 1;
    }

    return 0;
}
