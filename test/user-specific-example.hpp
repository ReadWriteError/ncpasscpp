/*
 * This file contains user specific variables for tests.
 */
#include <string>

//****************
// SESSION TESTS
//****************

// The federated IDs of all the Nextcloud accounts stored in Gnomes Online Accounts.
const std::string TEST_SESSION_FEDERATEDIDS[] = { "user@cloud.example.com", "user2@cloud.example2.com" };




//*****************
// PASSWORD TESTS
//*****************

// Account this example password is on.
#define TEST_PASSWORD_ACCOUNT "user@cloud.example.com"

// The example password itself. This information will be used to verify the libraries data.
#define TEST_PASSWORD_UUID "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
#define TEST_PASSWORD_LABEL "Password Label"
#define TEST_PASSWORD_USERNAME "Password Username"
#define TEST_PASSWORD_PASSWORD "password123"
