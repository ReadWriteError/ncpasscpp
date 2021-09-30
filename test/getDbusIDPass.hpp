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

#include <array>
#include <string>
#include <vector>
#include <dbus-c++/dbus.h>
using namespace std;


inline std::vector<std::array<std::string, 2>> getDbusIDPass()
{
    std::vector<std::string> accountIDs; // Vector for strings of accounts in Gnome Online Accounts (GOA).

    DBus::BusDispatcher disp;
    DBus::default_dispatcher = &disp; // Set the dispatcher for dbus to use.

    DBus::Connection connection = DBus::Connection::SessionBus(); // Using the session (user level) bus.

    DBus::CallMessage msgAccounts( // Message to dbus that gets all the accounts registered in GOA.
      "org.gnome.OnlineAccounts",
      "/org/gnome/OnlineAccounts/Accounts",
      "org.freedesktop.DBus.Introspectable",
      "Introspect"
      );


    // Loop through the XML pushing to accountIDs when a new account is found.
    for(
        std::string xml = connection.send_blocking(msgAccounts).reader().get_string(); // Get the XML from dbus.
        xml.find("<node name=") != std::string::npos;                                  // See if there is another node tag left in the XML.
        xml = xml.substr(xml.find("/>") + 1)                                           // Erase the node tag that was just processed from the string.
        )
    {
        xml = xml.substr(xml.find("\"", xml.find("<node name=")) + 1); // Find the first '"' after the node tag and update the xml variable to start at the quoted value.
        accountIDs.push_back(xml.substr(0, xml.find("\"")));           // Get a substr from the start of the current string up to the next '"' and push that to accountIDs.
    }


    // Loop through are accountIDs and remove the ones that aren't Nextcloud accounts.
    for( unsigned int i = 0; i < accountIDs.size(); i++ )
    {
        DBus::CallMessage msgProvider( // Message to dbus that gets a property.
          "org.gnome.OnlineAccounts",
          ("/org/gnome/OnlineAccounts/Accounts/" + accountIDs[i]).data(),
          "org.freedesktop.DBus.Properties",
          "Get"
          );
        msgProvider.writer().append_string("org.gnome.OnlineAccounts.Account"); // Set the interface (who you talk to) to org.gnome.Online.Account.
        msgProvider.writer().append_string("ProviderName");                     // Set the property to read to ProviderName (the account type ex: Nextcloud, Google).

        DBus::MessageIter msgItr = connection.send_blocking(msgProvider).reader(); // Make the ProviderName call to dbus and get the result as a message iterator.

        // Read the message iterator and delete the account if the result is not Nextcloud.
        if( DBus::Variant(msgItr).reader().get_string() == (std::string)"Nextcloud" )
            accountIDs.erase(accountIDs.cbegin() + i--);  // Delete the current element and decrease i by 1 (so that the for loop properly selects the next account when it runs i++).
    }

    std::vector<std::array<std::string, 2>> idPass; // The return object containing: { {federated ID, password}, {federated ID, password} }.


    // Loop through accountIDs and get federated IDs and passwords for all the accounts.
    for( auto accountID : accountIDs )
    {
        DBus::CallMessage msgFederatedID( // Message to dbus that gets the federated ID of the nextcloud user.
          "org.gnome.OnlineAccounts",
          ("/org/gnome/OnlineAccounts/Accounts/" + accountID).data(),
          "org.freedesktop.DBus.Properties",
          "Get"
          );
        msgFederatedID.writer().append_string("org.gnome.OnlineAccounts.Account"); // Set the interface (who you talk to) to org.gnome.Online.Account.
        msgFederatedID.writer().append_string("PresentationIdentity");             // Set the property to read to PresentationIdentity (the federated ID).

        DBus::CallMessage msgPassword( // Message to dbus that gets the password of the account.
          "org.gnome.OnlineAccounts",
          ("/org/gnome/OnlineAccounts/Accounts/" + accountID).data(),
          "org.gnome.OnlineAccounts.PasswordBased",
          "GetPassword"
          );
        msgPassword.writer().append_string(accountID.data()); // Set the account ID of the account.

        DBus::MessageIter msgItr = connection.send_blocking(msgFederatedID).reader(); // Make the federated ID call to dbus and get the result as a message iterator.


        idPass.push_back({ std::string(), std::string() }); // Create new strings for the federated ID and password.

        idPass.back()[0] = DBus::Variant(msgItr).reader().get_string();                 // Read the message iterator and store the federated ID in newly created string array.
        idPass.back()[1] = connection.send_blocking(msgPassword).reader().get_string(); // Make the call to dbus and store the result in the newly created string array.
    }

    return idPass; // And finaly return the object... wow thats a lot of confusing code.
}
