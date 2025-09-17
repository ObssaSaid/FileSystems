#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <string>
#include <iostream>
#include "../libWad/Wad.h"
#include <unistd.h>
#include <string.h>
#include <vector>

using namespace std;

static int getattr_callback(const char* path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    if (((Wad*)fuse_get_context()->private_data)->isDirectory(path)) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    if (((Wad*)fuse_get_context()->private_data)->isContent(path)) {
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        stbuf->st_size = ((Wad*)fuse_get_context()->private_data)->getSize(path);
        return 0;
    }

    return -ENOENT;    
}

static int mknod_callback(const char* path, mode_t mode, dev_t dev){
    ((Wad*)fuse_get_context()->private_data)->createFile(path);
    return 0;
}

static int mkdir_callback(const char* path, mode_t mode){
    ((Wad*)fuse_get_context()->private_data)->createDirectory(path);
    return 0;
}

static int read_callback(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info *fi){
    int bytes;
    bytes = ((Wad*)fuse_get_context()->private_data)->getContents(path, buf, size, offset);
    if (bytes != -1) {
        return bytes;
    }
    return -ENOENT;
}

static int write_callback(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info *fi){
    int bytes;
    bytes = ((Wad*)fuse_get_context()->private_data)->writeToFile(path, buf, size, offset);
    if (bytes != -1) {
        return bytes;
    }
    return -ENOENT;
}

static int readdir_callback(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
    (void) offset;
    (void) fi;

    vector<string> dir;
    int ret;
    ret = ((Wad*)fuse_get_context()->private_data)->getDirectory(path, &dir);

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    if (ret == -1) {
        return -ENOENT;
    } 

    for (int i = 0; i < ret; i++) {
        filler(buf, dir[i].c_str(), NULL, 0);
    }

    return 0;
}

static struct fuse_operations operations = {
    .getattr = getattr_callback,
    .mknod = mknod_callback,
    .mkdir = mkdir_callback,
    .read = read_callback,
    .write = write_callback,
    .readdir = readdir_callback,
};

int main(int argc, char* argv[]){
    if (argc < 3){
        cout << "Not enough arguments" << endl;
        exit(EXIT_SUCCESS);
    }

    string wad_path;
    wad_path = argv[argc - 2];

    if(wad_path.at(0) != '/') {
        wad_path = string(get_current_dir_name())+ "/" + wad_path;
    }

    Wad* w;
    w = Wad::loadWad(wad_path);

    argv[argc - 2] = argv[argc - 1];
    argc = argc - 1;

    return fuse_main(argc, argv, &operations, w);
}
