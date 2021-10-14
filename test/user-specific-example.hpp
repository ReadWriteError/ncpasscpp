/*
 * This file contains user specific variables for tests.
 */
#include <string>

//*****************
// GlOBAL OPTIONS
//*****************

// Used to format output as an argument to std::setw().
#define TEST_WIDTH 32

// Enable the debugger (and any user level process) to be able to attach to the process and read passwords from RAM.
//#define TEST_ENABLE_DEBUGGER




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

//TEST 3 (read/write): The password with the following UUID will changed to the NEW values then reset to the OLD values.
#define TEST_PASSWORD_3_ACCOUNT TEST_PASSWORD_GLOBAL_ACCOUNT
#define TEST_PASSWORD_3_UUID "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
#define TEST_PASSWORD_3_LABEL_NEW "Password Label New"
#define TEST_PASSWORD_3_LABEL_OLD "Password Label Old"
#define TEST_PASSWORD_3_USERNAME_NEW "Password Username New"
#define TEST_PASSWORD_3_USERNAME_OLD "Password Username Old"
#define TEST_PASSWORD_3_PASSWORD_NEW "password123 New"
#define TEST_PASSWORD_3_PASSWORD_OLD "password123 Old"
