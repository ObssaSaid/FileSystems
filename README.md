# P3 - File Systems

## Synopsis
In this project a userspace filesystem is implemented using FUSE API to access data in WAD format. A n-ary tree data structure is used to represent files. A library is developed to read and write for files and directories functionality. Via FUSE, a userspace daemon is implemented to access the directory structure once mounted.

## Description

### file_1: Wad.h
Declares Wad class and functions, as well as File struct for n-ary tree.

### file_2: Wad.cpp
Defines functions.

### file_3: wadfs.cpp
Provides read and write access with FUSE callback functions.

### static Wad* Wad::loadWad(...)
Wad file is read with fstream. N-ary tree stores Wad filesystem, each node represent a file containing a vector of strings that represent children. A stack is used to keep track of directory nesting. New files have name, offset, length, and boolean to check if it is directory or file.

### string Wad::getMagic()
Returns magic.

### bool Wad::isContent(...)
Boolean in node checks if there is content, return true if it is a valid path to an existing content file.

### bool Wad::isDirectory(...)
Boolean in node checks if it's a directory, return true for valid directories.

### int Wad::getSize(...)
Returns size of content file at path.

### int Wad::getContents(...)
Subtracts offset from file length to find available byes. Fstream object reads available bytes up to length from Wad file into buffer.

### int Wad::getDirectory(...)
Iterates through children vector and pushes them back to vector input parameter.

### void Wad::createDirectory(...)
Checks if path is valid. Reads contents into buffer to shift descriptor list, move forward 32 bytes with seekp, writes back to file. Seekp and write is used to update Wad file descriptor, and at the end of parent directory, data is written in created space.

### void Wad::createFile(...)
Applies the createDirectory process, content is written instead of directory file, and adds 16 bytes of a new descriptor to Wad file.

### int Wad::writeToFile(...)
Fstream functions updates Wad file variables and the data structure. The bytes of length are shifted forward to increment lump of Wad file.

### File* Wad::file_find(...)
File is found using file path and n-ary tree recursive search.

### string Wad::correct_name(...)
Removes null characters from name.

### bool Wad::isValid(...)
Returns true or false for if name is valid.

### int Wad::descriptor_find(...)
Returns integer for offset number of file.

### getattr_callback(...)
Determines permissions to assign with file path passed in isContent() and isDirectory(). Empty point in main_fuse allows libWad function and holds Wad object pointer.

### mknod_callback(...)
Parameters are passed into createFile(), returns 0.

### mkdir_callback(...)
Parameters are passed into createDirectory(), returns 0.

### read_callback(...)
Parameters are passed into getContents(), returns value if valid.

### write_callback(...)
Parameters are passed into writeToFile(), returns value if valid.

### readdir_callback(...)
Parameters are passed into getDirectory(), fills filler object with the returned directory string names.

### operations(...)
FUSE callback functions are configured.

### fuse_main(...)
The first and third command line arguments are used in main, absolute filepath loads Wad object, fourth parameter in fuse_main is passed by pointer to Wad object.

## Testing
Library testing suite and sample Wad files are used for testing to ensure all tests passed. Fuse and wadfs file is linked with library to mount to a directory to test reading and writing.

## Bugs
N/A

## Demo
https://youtu.be/SR5GiGl72i0

## References
University of Florida, "P3: File Systems, COP4600 Project PDF"

Lorenzo Fontana, "Write a Filesystem with FUSE", https://engineering.facile.it/blog/eng/write-filesystem-fuse/

"FUSE tutorial", https://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/html/

"CS137 FUSE Documentation", https://www.cs.hmc.edu/~geoff/classes/hmc.cs137.201601/homework/fuse/fuse_doc.html

## Author
Obssa Said
