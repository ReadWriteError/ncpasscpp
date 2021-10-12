# NCPASSCPP

  If you are looking to make a password manager in C++ for [Nextcloud Passwords](https://apps.nextcloud.com/apps/passwords) then you have come to the right place.
  This is a very high level library and all API calls to the server are abstracted and performed asynchronously.
  I've tried my very hardest to make sure this project is as easy to use as possible.
  Anyone should be able quickly and easily learn how to use this project.

  This is a third-party library.


## Design Goals / Features

  - Strong exception safety.
    - TODO: Conflicts will be accessible and able to be resolved.
    - TODO: API errors will be logged and not thrown.
  - Thread safe.
    - There will always be one (or zero) local object for every remote object on the server. Trying to construct the same object twice will return the same instance (a `std::shared_ptr` not a copy) to both.
  - Scopeless (Oooooo fancy word I just made up). This means that local objects are never deconstructed and can be retrieved later, unless they are specifically told to go out of scope.
  - The current thread should never be blocked waiting for an API call to complete unless the developer requests to wait.
    - Only applies after the object is populated (so using this library synchronously isn't too complicated).
  - Any local changes to the password will be stored on the server at some point.
    - Example: If there is a conflict and the user chooses the remote version, the local changes will be pushed to the remote. After that, the remote password will be reverted back to the original version the user choose.
