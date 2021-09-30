/*
 * ncpasscpp. A library that implements Nextcloud Password's API.
 * Copyright (C) 2021  Reed Krantz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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
#include <sys/mman.h>
#include <ServerCredentials.hpp>
#include "getDbusIDPass.hpp"

using namespace std;


int main(int argc, char** argv)
{
    if( argc != 1 )
    {
        cout << argv[0] << " takes no arguments.\n";

        return 1;
    }

    // Lock all the memory used by this program in ram so that they will never be writen to disk.
    // DISCLAMER: this does not stop memory being writen to the disk during hybernation.
    mlockall(MCL_CURRENT | MCL_FUTURE | MCL_ONFAULT);

    vector<shared_ptr<ncpass::ServerCredentials>> credentials; // A vector containing all of the nextcloud credentials available on this server.


    // How you get your "federatedIDs and passwords" or "usernames, rootServerURLs and passwords" is up to you and your application of this lib.
    // I'm getting this from the Gnome Online Accounts daemon over dbus.
    // You can look in getDbusIDPass.hpp for how I do this (its kinda complicated).
    for( array<string, 2> idPassArr : getDbusIDPass() ) // Gets a vector of string arrays containing { {federatedID, password}, {federatedID, password} } and loops through it.
        credentials.push_back(ncpass::ServerCredentials::create(idPassArr[0], idPassArr[1]));

    bool didAllCredentialsPassVerification = true;


    // Loop through credentials and verify each one.
    for( shared_ptr<ncpass::ServerCredentials> cred : credentials )
    {
        bool didCredPassVerification = cred.use_count() == 1;
        //TODO: bool didCredPassVerification = cred->verify(); // Verify login details with the nextcloud server.

        if( didAllCredentialsPassVerification )
            didAllCredentialsPassVerification = didCredPassVerification;

        //TODO: cout << cred->getFederatedID() << ": " << didCredPassVerification << endl; // Print the federated ID and whether it passed the test.
    }

    return !didAllCredentialsPassVerification;
}
