/*
 * This file contains user specific variables for tests.
 */
#include <string>

//*****************
// GlOBAL OPTIONS
//*****************

// Used to format output as an argument to std::setw()
#define TEST_WIDTH 32




//****************
// SESSION TESTS
//****************

// The federated IDs of all the Nextcloud accounts stored in Gnomes Online Accounts.
const std::string TEST_SESSION_1_FEDERATEDIDS[] = { "user@cloud.example.com", "user2@cloud.example2.com" };




//*****************
// PASSWORD TESTS
//*****************

// Account used as the default for all the tests.
#define TEST_PASSWORD_GLOBAL_ACCOUNT "user@cloud.example.com"

//TEST 1 (read only): The values here specify an already existing password on the server.
#define TEST_PASSWORD_1_ACCOUNT TEST_PASSWORD_GLOBAL_ACCOUNT
#define TEST_PASSWORD_1_UUID "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
#define TEST_PASSWORD_1_LABEL "Password Label"
#define TEST_PASSWORD_1_USERNAME "Password Username"
#define TEST_PASSWORD_1_PASSWORD "password123"

//TEST 2 (read/write): The values here describe a password that will be created.
#define TEST_PASSWORD_2_ACCOUNT TEST_PASSWORD_GLOBAL_ACCOUNT
#define TEST_PASSWORD_2_LABEL "Password Label"
#define TEST_PASSWORD_2_USERNAME1 "Username revision1"
#define TEST_PASSWORD_2_USERNAME2 "Username revision2"
#define TEST_PASSWORD_2_PASSWORD "password123"
