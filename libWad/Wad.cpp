#include "Wad.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ostream>
#include <iostream>
#include <stack>
#include <vector>

using namespace std;


Wad* Wad::loadWad(const string &path){
    //Invokes the constructor by creating a Wad object with new, then returns the pointer to it.
    Wad* WAD;
    WAD = new Wad();
    WAD->filePath = path;
    fstream inputFile;
    inputFile.open(path, ios::in | ios::out | ios::binary);

    if (!inputFile.is_open()){
        return nullptr;
    }
    
    char magic[5];
    magic[4] = '\0';
    inputFile.read(magic, 4);
    WAD->magic = string(magic, 4);

    stack<File*> stackDFS;
    uint32_t offsetDesc;
    uint32_t numDesc;


    inputFile.read((char*)&numDesc, 4);
    WAD->numDescriptor = numDesc;
    inputFile.read((char*)&offsetDesc, 4);
    WAD->offsetDescriptor = offsetDesc;

    WAD->root = new File("/", 0, 0, true);

    inputFile.seekg(WAD->offsetDescriptor, ios_base::beg);

    // depth-first search algorithm that can keep track of directory nesting

    stackDFS.push(WAD->root);

    for (int i = 0; i < WAD->numDescriptor; i++){
        uint32_t length;
        uint32_t offset;

        char filename[9];
        filename[8] = '\0';

        inputFile.read((char*)&offset, 4);
        inputFile.read((char*)&length, 4);
        inputFile.read(filename, 8);
        string new_name = correct_name(string(filename, 8));

        if (length == 0){
            if (isdigit(new_name[1])&&isdigit(new_name[3])&&new_name[0] == 'E'&&new_name[2] == 'M'){
                File* file1;
                File* file2;
                File* file3;
                File* new_file;
                new_file = new File(new_name, offset, length, true);
                stackDFS.top()->children.push_back(new_file);
                stackDFS.push(new_file);

                for (int j = 0; j < 10; j++){
                    uint32_t offset_mapmarker;
                    uint32_t length_mapmarker;
                    char filename_mapmarker[9];
                    filename_mapmarker[8] = '\0';

                    inputFile.read((char*)&offset_mapmarker, 4);
                    inputFile.read((char*)&length_mapmarker, 4);
                    inputFile.read(filename_mapmarker, 8);
                    string mapName;
                    mapName = correct_name(string(filename_mapmarker, 8));                    

                    File* file_mapmarker;
                    file_mapmarker = new File(mapName, offset_mapmarker, length_mapmarker, false);
                    new_file->children.push_back(file_mapmarker);
                    i++;
                }
                stackDFS.pop();
            }
            else if (new_name.find("_END") != string::npos) {
                stackDFS.pop();
            }
            else if (new_name.find("_START") != string::npos) {
                File* new_file;
                File* fil1;
                new_file = new File(new_name, offset, length, true);
                new_file->filename =  new_file->filename.substr(0,(int)new_file->filename.find("_START"));
                stackDFS.top()->children.push_back(new_file);
                stackDFS.push(new_file);
            }
            else {
                File* new_file;
                File* file2;
                new_file = new File(new_name, offset, length, false);
                stackDFS.top()->children.push_back(new_file);
            }
        }
        else{
            File* new_file;
            File* file3;
            new_file = new File(new_name, offset, length, false);
            stackDFS.top()->children.push_back(new_file);
        }
    }
    inputFile.close();

    return WAD;
}

string Wad::getMagic(){
    return this->magic;
}

bool Wad::isContent(const string &path){
    File* file;
    File* content;
    file = file_find(path, this->root);
    if (file == nullptr){
        return false;
    }
    else
    //return true if it is a valid path to an existing content file
    return (!file->isDir);
}

bool Wad::isDirectory(const string &path){
    File* file;
    file = file_find(path, this->root);
    if (path == ""||file == nullptr){
        return false;
    }
    else
    // return true for valid directories
    return (file->isDir);   
}

int Wad::getSize(const string &path){
    if (isContent(path) == false){
        return -1;
    }
    else {
        File* file;
        file = file_find(path, this->root);
        //Returns the size of the file at path,
        return file->length;
    }
}

int Wad::getContents(const string &path, char *buffer, int length, int offset){
    /*If path represents content, copies as many bytes as are available, up to length, 
    of content's data into the preexisting buffer. If offset is provided, data should be copied starting from that 
    byte in the content. Returns number of bytes copied into buffer, 
    or -1 if path does not represent content (e.g., if it represents a directory). */
    if (isContent(path) == false){
        return -1;
    }
    else {
        File* contents;
        contents = this->file_find(path, this->root);
        int bytes;
        int len = contents->length;
        bytes = len - offset;
        if (bytes <= 0){
            return 0;
        }
        else{
        fstream inputFile;
        inputFile.open(this->filePath, ios::in | ios::out | ios::binary);
        if (!inputFile.is_open()){
            return -1;
        }
        inputFile.seekg(contents->offset + offset, ios_base::beg);
        inputFile.read(buffer, min(bytes,length));
        inputFile.close();

        return min(bytes,length);
        }
    }
}

int Wad::getDirectory(const string &path, vector<string> *directory){
    //Returns the number of elements in the directory, or -1 if path does not represent a directory
    File* dirFile;
    File* dF2;
    if (!isDirectory(path)){
        return -1;
    }

    else {
        File* directory_file;
        File* dirF3;
        directory_file = this->file_find(path, this->root);
        directory->clear();

        for (int i = 0; i < directory_file->children.size(); i++){
            if (directory_file->children[i] != nullptr){
                directory->push_back(directory_file->children[i]->filename);
            }
        }
        return directory_file->children.size();
    }
}

void Wad::createDirectory(const string &path){
    /*path includes the name of the new directory to be created. If given a valid path, creates a new directory 
using namespace markers at path. The two new namespace markers will be added just before the “_END” 
marker of its parent directory. New directories cannot be created inside map markers.
It will be a namespace directory, meaning you will need to add a start and end descriptor, and the filename must be at most 2 characters.
*/
    fstream inputFile;
    inputFile.open(this->filePath, ios::in | ios::out | ios::binary);

    if (inputFile.is_open() == false){
        return;
    }

    string p = path;
    if (path[path.size()-1] == '/'){
        p = path.substr(0,path.size()-1);
    }

    int index;
    index = p.rfind('/');
    uint32_t numDesc;

    string parentPath;
    parentPath = p.substr(0,index);


    string newDir;
    newDir = p.substr(index + 1);
    if (newDir.length() > 2) {
        return;
    }

    string parentDir;
    parentDir = parentPath;

    if (parentPath == ""){
        parentPath = "/";
        parentDir = "/";
    }
    else{
        parentDir = parentPath.substr(parentPath.rfind('/') + 1);
        if (parentDir.length() > 2 || isDirectory(parentPath) == false) {
            return;
        }
    }
 
    //update decriptor number in file
    inputFile.seekg(4, ios_base::beg);
    //uint32_t numDesc;
    inputFile.read((char*)&numDesc, 4);
    numDesc = numDesc + 2;
    inputFile.seekp(4, ios_base::beg);
    inputFile.write(reinterpret_cast<const char*>(&numDesc), sizeof(numDesc));
    
    if (parentDir == "/"){
        File* parentFile;
        parentFile = root;
        File* newFile;
        newFile = new File(newDir,0,0, true);
        parentFile->children.push_back(newFile);
        this->numDescriptor = this->numDescriptor + 2;
        const char padding = '\0';
        uint32_t data;
        data = 0;
        string newDirStart;
        string newDirEnd;
        string newStart = newDir + "_START";
        string newEnd = newDir + "_END";
        newDirStart = newStart;

        inputFile.seekp(0, ios_base::end);     
        inputFile.write(reinterpret_cast<const char*>(&data), sizeof(data));
        inputFile.write(reinterpret_cast<const char*>(&data), sizeof(data));

        int padSize = 8 - newDirStart.size();
        int paddingSize = padSize;
        inputFile.write(newDirStart.c_str(), newDirStart.size());
        for (int i = 0; i < paddingSize; ++i) {
            inputFile.write(&padding, 1);
        }

        newDirEnd = newEnd;
        inputFile.write(reinterpret_cast<const char*>(&data), sizeof(data));
        inputFile.write(reinterpret_cast<const char*>(&data), sizeof(data));

        paddingSize = 8 - newDirEnd.size();
        inputFile.write(newDirEnd.c_str(), newDirEnd.size());
        for (int i = 0; i < paddingSize; ++i) {
            inputFile.write(&padding, 1);
        }
        inputFile.close();
        return;
    }

    File* parentFile;
    File* newFile;
    uint32_t parentDirOffset;
    uint32_t data = 0;
    const char padding = '\0';
    string newDirStart;
    string newDirEnd;



    parentFile = file_find(parentPath, this->root);
    newFile = new File(newDir, 0, 0, true);
    parentFile->children.push_back(newFile);

    //uint32_t parentDirOffset;
    parentDirOffset = descriptor_find(path);
    inputFile.seekg(parentDirOffset, ios_base::beg);

    vector<char> buffer(istreambuf_iterator<char>(inputFile), {});
    inputFile.seekp(parentDirOffset+32, ios_base::beg);
    inputFile.write(buffer.data(), buffer.size());

    inputFile.seekp(parentDirOffset, ios_base::beg);
    string newStart = newDir + "_START"; 
    newDirStart = newStart;        
    inputFile.write(reinterpret_cast<const char*>(&data), sizeof(data));
    inputFile.write(reinterpret_cast<const char*>(&data), sizeof(data));

    int padSize = 8 - newDirStart.size();
    int paddingSize = padSize;
    inputFile.write(newDirStart.c_str(), newDirStart.size());
    for (int i = 0; i < paddingSize; ++i) {
        inputFile.write(&padding, 1);
    }

    string newEnd = newDir + "_END";
    newDirEnd = newEnd;
    inputFile.write(reinterpret_cast<const char*>(&data), sizeof(data));
    inputFile.write(reinterpret_cast<const char*>(&data), sizeof(data));

    paddingSize = 8 - newDirEnd.size();
    inputFile.write(newDirEnd.c_str(), newDirEnd.size());
    for (int i = 0; i < paddingSize; ++i) {
        inputFile.write(&padding, 1);
    }

    this->numDescriptor = this->numDescriptor + 2;
    inputFile.close();
    return;
}

void Wad::createFile(const string &path){
    /*path includes the name of the new file to be created. If given a valid path, creates an empty file at path, 
with an offset and length of 0. The file will be added to the descriptor list just before the “_END” marker 
of its parent directory. New files cannot be created inside map markers.
Takes in a path to a file that does not yet exist, and must be created.*/

    File* file_create;
    fstream inputFile;
    inputFile.open(this->filePath, ios::in | ios::out | ios::binary);

    if (inputFile.is_open() == false){
        return;
    }

    std::string p;
    p = path;
    if (path[path.size()-1] == '/'){
        p = path.substr(0,path.size()-1);
    }

    int index;
    index = p.rfind('/');

    std::string parentPath;
    parentPath = p.substr(0,index);
    std::string newFileAdded;
    newFileAdded = p.substr(index + 1);
    uint32_t numDesc;

    if (is_valid(newFileAdded)){
        return;
    }

    std::string parentDir;
    parentDir = parentPath;

    if (parentPath != ""){
        parentDir = parentPath.substr(parentPath.rfind('/') + 1);
        if (parentDir.length() > 2 || isDirectory(parentPath) == false) {
            return;
        }
    }
    else{
        parentPath = "/";
        parentDir = "/";
    }

    inputFile.seekg(4, std::ios_base::beg);
    inputFile.read((char*)&numDesc, 4);
    numDesc = numDesc + 1;
    inputFile.seekp(4, std::ios_base::beg);
    inputFile.write(reinterpret_cast<const char*>(&numDesc), sizeof(numDesc));
 
    if (parentDir == "/"){
        File* newFile;
        File* parentFile;
        uint32_t data = 0;
        const char padding = '\0';

        parentFile = root;
        newFile = new File(newFileAdded,0,0, false);
        parentFile->children.push_back(newFile);
        this->numDescriptor = this->numDescriptor + 1;

        inputFile.seekp(0, std::ios_base::end);       
        inputFile.write(reinterpret_cast<const char*>(&data), sizeof(data));
        inputFile.write(reinterpret_cast<const char*>(&data), sizeof(data));

        int padSize = 8 - newFileAdded.size();
        int paddingSize = padSize;
        inputFile.write(newFileAdded.c_str(), newFileAdded.size());
        for (int i = 0; i < paddingSize; ++i) {
            inputFile.write(&padding, 1);
        }

        inputFile.close();
        return;
    }

    File* newFile;
    File* parentFile;
    uint32_t data = 0;
    const char padding = '\0';

    parentFile = file_find(parentPath, this->root);
    //File* newFile;
    File* f1;
    File* f2;
    newFile = new File(newFileAdded, 0, 0, false);
    parentFile->children.push_back(newFile);

    uint32_t parentDirOffset = descriptor_find(path);
    inputFile.seekg(parentDirOffset, std::ios_base::beg);

    std::vector<char> buffer(std::istreambuf_iterator<char>(inputFile), {});
    inputFile.seekp(parentDirOffset+16, std::ios_base::beg);
    inputFile.write(buffer.data(), buffer.size());

    inputFile.seekp(parentDirOffset, std::ios_base::beg);        
    //uint32_t data = 0;
    inputFile.write(reinterpret_cast<const char*>(&data), sizeof(data));
    inputFile.write(reinterpret_cast<const char*>(&data), sizeof(data));

    //const char padding = '\0';
    int paddingSize = 8 - newFileAdded.size();
    inputFile.write(newFileAdded.c_str(), newFileAdded.size());
    for (int i = 0; i < paddingSize; ++i) {
        inputFile.write(&padding, 1);
    }

    this->numDescriptor = this->numDescriptor + 1;
    inputFile.close();
    return;
}

int Wad::writeToFile(const std::string &path, const char *buffer, int length, int offset){
/*If given a valid path to an empty file, augments file size and generates a lump offset, then writes length amount 
of bytes from the buffer into the file’s lump data. If offset is provided, data should be written starting from that 
byte in the lump content. Returns number of bytes copied from buffer, or -1 if path does not represent content*/

    std::fstream inputFile;
    File* file;
    uint32_t fileOffset;
    uint32_t offsetDesc;

    inputFile.open(this->filePath, std::ios::in | std::ios::out | std::ios::binary);
    if (inputFile.is_open() == false){
        return -1;
    }
    if (isDirectory(path) == true) {
        return -1;
    }
    if (isContent(path) == false){
        return -1;
    }
    //File* file;
    file = file_find(path, this->root);
    if (file->length != 0){
        return 0;
    }

    //uint32_t fileOffset;
    fileOffset = descriptor_find(path);
    inputFile.seekp(fileOffset, std::ios_base::beg);
    inputFile.write(reinterpret_cast<const char*>(&offsetDescriptor), sizeof(offsetDescriptor));
    inputFile.write(reinterpret_cast<const char*>(&length), sizeof(length));

    File* f01;
    File* f02;

    inputFile.seekg(offsetDescriptor, std::ios_base::beg);
    std::vector<char> buffer2(std::istreambuf_iterator<char>(inputFile), {});
    inputFile.seekp(offsetDescriptor+length, std::ios_base::beg);
    inputFile.write(buffer2.data(), buffer2.size());

    inputFile.seekp(offsetDescriptor, std::ios_base::beg);
    inputFile.write(buffer, length);

    inputFile.seekg(8, std::ios_base::beg);
    //uint32_t offsetDesc;
    inputFile.read((char*)&offsetDesc, 4);
    //offsetDesc += length;
    offsetDesc = offsetDesc + length;
    inputFile.seekp(8, std::ios_base::beg);
    inputFile.write(reinterpret_cast<const char*>(&offsetDesc), sizeof(offsetDesc));
    File* f03;

    file->length = length;
    file->offset = offsetDescriptor;
    this->offsetDescriptor = this->offsetDescriptor + length;

    inputFile.close();
    return length;
}

File* Wad::file_find(const std::string &path, File* file){
//Recursively searches through n-ary tree
    std::string directory;

    if (path == "/"){
        return file;
    }
    if (path == ""){
        return file;
    }
    const std::string new_path = path.substr(path.find('/') + 1);
    //std::string directory;
    directory = new_path;

    if (new_path.find('/') != std::string::npos){
        directory = new_path.substr(0, new_path.find('/'));
    }
    File* f1;

    for (int i = 0; i < file->children.size(); i++){
        if (file->children[i]->filename == directory) {
            return this->file_find(new_path.substr(directory.length()), file->children[i]);
        }
    }
    return nullptr;
}

std::string Wad::correct_name(std::string filename){
    //null characters remover
    File* f1;
    std::string new_name = "";
    for (int i = 0; i < filename.length(); i++){
        if (filename[i] == '.'){
            new_name.push_back(filename[i]);
        }
        if (filename[i] == '_'){
            new_name.push_back(filename[i]);
        }
        if (isdigit(filename[i])){
            new_name.push_back(filename[i]);
        }
        if (isalpha(filename[i])){
            new_name.push_back(filename[i]);
        }
    }
    return new_name;
}

bool Wad::is_valid(std::string filename){
    //is the name valid?
    if (filename.length() == 4){
        if (filename[0] == 'E' && filename[2] == 'M'){
            if (isdigit(filename[3]) && (isdigit(filename[1]))){
                return true;
            }
        }
    }
    if (filename.length() > 8 || filename.find("_START") != std::string::npos || filename.find("_END") != std::string::npos){
        return true;
    }
    return false;    
}

uint32_t Wad::descriptor_find(const std::string &path){
    //Returns offset number for file
    File* f0;
    File* f1;
    std::fstream inputFile;
    inputFile.open(this->filePath, std::ios::in | std::ios::out | std::ios::binary);

    if (inputFile.is_open() == false){
        return -1;
    }

    std::vector<std::string> nodes = {"/"};
    std::string child;
    child = path;
    int index;
    std::vector<std::string> dirStack;
    uint32_t fileLocation;
    std::string new_name;


    if (path[path.size()-1] == '/'){
        child = path.substr(0,path.size()-1);
    }


    index = child.find("/");
    File* f2;
    File* f3;

    child = child.substr(index+1);

    while (child.find("/") != std::string::npos) {
        index = child.find("/");
        nodes.push_back(child.substr(0,index));
        child = child.substr(index + 1);
    }

    inputFile.seekg(offsetDescriptor, std::ios_base::beg);

    //std::vector<std::string> dirStack;
    //uint32_t fileLocation;
    fileLocation = this->offsetDescriptor;
    dirStack.push_back("/");

    for (int i = 0; i < this->numDescriptor; i++){
        uint32_t length;
        uint32_t offset;

        inputFile.read((char*)&offset, 4);
        fileLocation = fileLocation + 4;

        //uint32_t length;
        inputFile.read((char*)&length, 4);
        fileLocation = fileLocation + 4;

        char filename[9];
        filename[8] = '\0';
        inputFile.read(filename, 8);
        fileLocation = fileLocation + 8;

        //std::string new_name;
        new_name = correct_name(std::string(filename, 8));

        if (new_name.find("_START") != std::string::npos){
            int ind;
            ind = new_name.find("_START");
            dirStack.push_back(new_name.substr(0, ind));
        }
        else if (new_name.find("_END") != std::string::npos) {
            if (dirStack == nodes) {
                fileLocation = fileLocation - 16;
                inputFile.close();
                return fileLocation;
            }
            dirStack.pop_back();
        }
        else if (new_name == child) {
             if (dirStack == nodes) {
                fileLocation = fileLocation - 16;
                inputFile.close();
                return fileLocation;
            }           
        }
    }
    inputFile.close();
    return fileLocation;
}

File::~File(){
    for (File* child: this->children) {
        delete child;
    }
}

Wad::~Wad(){
    delete root;
    root = nullptr;
}

Wad::Wad(){
}
