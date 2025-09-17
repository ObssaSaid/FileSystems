#pragma once
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ostream>
#include <iostream>
#include <stack>
#include <vector>

struct File {
    std::string filename;
    uint32_t offset;
    uint32_t length;
    std::vector<File*> children;
    bool isDir;

    File(std::string filename, uint32_t offset, uint32_t length, bool isDir){
        this->filename = filename;
        this->offset = offset;
        this->length = length;
        this->isDir = isDir;
    }
    ~File();
};

class Wad {
    private:
        std::string magic;
        uint32_t numDescriptor;
        uint32_t offsetDescriptor;
        File* root;
        std::string filePath;
 
        File* file_find(const std::string &path, File* file);
        static std::string correct_name(std::string filename);
        static bool is_valid(std::string filename);
        uint32_t descriptor_find(const std::string &path);

    public:
        static Wad* loadWad(const std::string &path);
        std::string getMagic();
        bool isContent(const std::string &path);
        bool isDirectory(const std::string &path);
        int getSize(const std::string &path);
        int getContents(const std::string &path, char *buffer, int length, int offset = 0);
        int getDirectory(const std::string &path, std::vector<std::string> *directory);
        void createDirectory(const std::string &path);
        void createFile(const std::string &path);
        int writeToFile(const std::string &path, const char *buffer, int length, int offset = 0);

        Wad();
        ~Wad();
};
