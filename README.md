# ncpasscpp

This library is not ready for use.
Everything is subject to change at this point.

If you are looking to make a password manager in C++ for [Nextcloud Passwords](https://apps.nextcloud.com/apps/passwords) then you have come to the right place.
This is a very high level library and all API calls to the server are abstracted and performed asynchronously.
I've tried my very hardest to make sure this project is as easy to use as possible.
Anyone should be able quickly and easily learn how to use this project.

This is a third-party library.


## Design Goals / Features

  - [x] Strong exception safety.
    - [ ] Conflicts will be accessible and able to be resolved.
    - [ ] API / Networking errors will be logged and not thrown.
  - [x] Thread safe.
    - [x] There will always be one (or zero) local object for every remote object on the server. Trying to construct the same object twice will return the same instance (a `std::shared_ptr` not a copy) to both.
  - [x] Scopeless (Oooooo fancy word I just made up). This means that local objects are never deconstructed and can be retrieved later. That is unless they are specifically told to be deallocated.
  - [x] The current thread should never be blocked waiting for an API call to complete unless the developer requests to wait.
    - Only applies after the object is populated (so false values will not be reported).
  - [ ] Any local changes to the password will be stored on the server at some point.
    - Example: If there is a conflict and the user chooses the remote version, the local changes will be pushed to the remote. After that, the remote password will be reverted back to the original version the user choose.


## OK so what API related stuff works so far?

so far the following works:

  - Password
    - [X] retrieve a password from the server using its UUID
    - [ ] create a new password
    - [x] read properties
    - [ ] write properties
    - available properties
      - id (read-only)
      - label
      - username
      - password

Yeah so not much.
But, I do it in a really complicated way that I think is pretty cool.
I've been mostly working on "Design Goals / Features".
I'll implement more API related features soon.


## Usage

### Security Information
If you are planning on using this library there are some security considerations you need to know about.
All objects including passwords and the session information are stored locally in RAM in plain text.
So if you are making a password manager it's your responsibility to **make sure that no user level process can access your process RAM**.
On linux this can be done with `prctl(PR_SET_DUMPABLE, false)` from `#include <sys/prctl.h>` at the start of your main() function.
It's also your responsibility to **make sure your process's RAM is never stored on the disk**.
On linux this can be done with `mlockall(MCL_CURRENT | MCL_FUTURE | MCL_ONFAULT);` from `#include <sys/mman.h>` at the start of your main() function.
Additionally you need to **make sure your process locks itself before hybernation**.
I haven't figured out how to do this yet.
But on linux I would research *DBUS signals* and *UPower*.
Equivalents exist for Windows and MacOS for all I talked about above.
I just don't know what they are.

### Basic Password Example
So you want to print a password from the server and modify its values?
Here is some example code that does this:
``` c++
// auto type is "shared_ptr<ncpass::Session>"
// Create an object that represents a connection to a server.
auto session = ncpass::Session::create("user@cloud.example.com", "whydoialwaysforgetmypassword");

// auto type is "shared_ptr<ncpass::Password>"
// Here is where you get an existing password. You pass a Session and the UUID of the password you want to get.
auto password = ncpass::Password::get(session, "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx");

// Print some before information.
cout << "UUID:     " << password->getID()       << endl;
cout << "Label:    " << password->getLabel()    << endl;
cout << "Username: " << password->getUsername() << endl;
cout << "Password: " << password->getPassword() << endl;

// This also changes the value of the password property on the server.
password.setPassword("plsdontsteal");

cout << "Password: " << password->getPassword() << endl; // Will output "Password: plsdontsteal"
```
pretty simple.


## Compile

### Don't enable the tests
Seriously don't.
I made the tests for myself and they currently scan Gnome Online Accounts for nextcloud accounts and will potentially wreak havoc on the servers.
For this reason they are disabled by default.
assuming you don't care about the data on your server or really trust that I'm always perfect and my code never fails, you can enable the tests by copying `test/user-specific-example.hpp` to `test/user-specific.hpp` and filling out the variables with user specific information.

### How to Compile
This project uses meson as the build system.

  1. Install the project and set your current working directory to the projects root.

#### GNU/Linux (Debian)
  2. Install meson:
  ``` bash
  sudo apt install meson
  ```

  3. Install any dependencies of this project:
  ``` bash
  sudo apt install libcurlpp-dev nlohmann-json3-dev libsodium-dev openssl
  [ -f "./test/user-specific.hpp" ] && sudo apt install libdbus-c++-dev   # extra dependency for the tests if enabled
  ```

  4. Then run `setup` and `compile`
  ``` bash
  meson setup build/
  meson compile -C build/
  ```

#### MacOS
Not really sure. Don't have a mac.
I've heard about something called `brew`. Maybe you can adapt the linux guide and use `brew` instead of `apt`.
  - TODO: Research `brew`.

#### Windows
Also not sure. Never programmed on Windows before.
  - TODO: Learn how people survive without package managers.
