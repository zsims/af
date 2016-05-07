# af

Simple content based backup system with the following goals:

 1. Provide a **simple** way for users to backup data from any source: Locally, Twitter, Facebook, etc.
 2. Allow backups to be monitored, to give confidence that data is still being backed up
 3. Allow backups to be stored where you want them to be

# Similar Projects
This idea is not unique, there's several other projects that overlap with the above goals:

 * [camlistore](https://camlistore.org/) - "personal storage" project designed for sharing, syncing, claims, etc.
 * [Fossil](https://en.wikipedia.org/wiki/Fossil_(file_system)) - recursive content-addressable file system
 * [Tahoe-LAFS](http://tahoe-lafs.org/) - open and decerntralised cloud storage system
 
## Why another?
af aims to be for end-users, the above projects are highly techinical or research based. af aims to bring some of these ideas for solving end user backups. af was prompted by:

 * Existing projects being too complex for end users to grasp
 * Goals of existing projects being very broad (e.g. not just for backups)
 * Existing backup solutions are focused on just the file system or system images
 * Some of the concerns raised by "rogue archivists" the [Archive Team](http://www.archiveteam.org/) apply to end users. Your data is locked in these third party services and you're at their mercy.
