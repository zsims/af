# af

Simple content based backup system with the following goals:

 1. Provide a **simple** way for users to backup data from any source: Locally, Twitter, Facebook, etc.
 2. Allow backups to be monitored, to give confidence that data is still being backed up
 3. Allow backups to be stored where you want them to be

See [the wiki](https://github.com/zsims/af/wiki) for internal documentation.

# Contributing
 1. Raise a PR for your changes
 2. Get a :shipit: from someone, unless it's small (e.g. doco or updating a `.gitignore`)
 3. Wait for the verifies (Windows 32, Windows 64)
 4. Merge

# Building (af services, c++)

You'll need:
 * CMake 3.5+
 * Windows:
  * Visual Studio 2015

## Configure
 * Windows: `configure-vs2015-win64.bat` or `configure-vs2015-win32.bat`

## Verify
A verify will compile everything and run all tests. Test results can be found in the build directory
under `test_results`, e.g. `build64/test_results`.

To verify run: `cmake --build build64 --config Release --target ALL_VERIFY`

# Building (web client)

You'll need:
 * Node
 * Some form of JS editor (Visual Studio Code is pretty good!)

## Configure
 * From the web_client directory, run `npm init`

## Start development server
 * Run `npm start`
