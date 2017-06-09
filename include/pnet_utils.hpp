#ifndef PNET_UTILS_HPP_
#define PNET_UTILS_HPP_

#include <dirent.h>
#include <iostream>
#include <fstream>
#include <regex>
#include <fstream>
#include <cstring>

namespace pnet {

  // Print an error message to console and exists.
  void FATAL(std::string err_str){
    std::cout << "FATAL ERROR: " << err_str << std::endl;
    exit(-1);
  }

  // Check the condition.
  // If not satisfied, raises FATAL ERROR with the given message
  void ASSERT_TRUE(bool condition, const std::string &message) {
    if (!condition)
      FATAL(message);
  }

  namespace utils {

    // Check whether the string ends with one of the given extensions.
    bool stringEndsWith(std::string str, std::vector<std::string> extensions) {
      for(auto &ext : extensions){
        std::regex e("(.*)" + ext);
        if (std::regex_match(str, e)){
          return true;
        }
      }
      return false;
    }

    // Check whether the given directory exists.
    bool directoryExists(std::string dirname) {
      DIR *dir = opendir(dirname.c_str());
      if (dir) {
        closedir(dir);
        return true;
      }
      return false;
    }

    // Check whether the given file exists.
    bool fileExists(std::string filename) {
      FILE *file_ = fopen(filename.c_str(), "rb");
      if (!file_) {
        return false;
      }
      fclose(file_);
      return true;
    }

    // Create a valid path name from directory and filename.
    std::string pathJoin(std::string dirname, std::string filename){
      if(dirname.back()=='/'){
        return dirname + filename;
      } else {
        return dirname + std::string("/") + filename;
      }
    }

    // Returns a vector of strings containing file names in the directory dirname.
    // The files must end with one of the extensions in the extensions vector.
    // If 'fullpath' is set, the file paths are absolute, starting from '/'
    // Returned files are guaranteed to be in the alphabetical order.
    std::vector<std::string> ls(std::string dirname, bool fullpath,
                                std::vector<std::string> extensions) {
      std::vector<std::string> files;
      if( directoryExists(dirname)){
        DIR *dir = opendir(dirname.c_str());
        struct dirent *dp;
        while ((dp = readdir (dir)) != NULL) {
          if(stringEndsWith(dp->d_name, extensions)){
            if(fullpath){
              files.push_back(pathJoin(dirname, dp->d_name));
            } else {
              files.push_back(dp->d_name);
            }
          }
        }
        closedir(dir);
      }
      // Get the files matching the regex.
      sort(files.begin(), files.end());
      return files;
    }

    std::vector<std::string> tokenize(std::string &str, char delimiter){
      std::vector<std::string> tokens;
      char *p = std::strtok(&str[0], &delimiter);
      while (p) {
        tokens.push_back(p);
        p = strtok(nullptr, &delimiter);
      }
      return tokens;
    }

    // Compresses 'src_file' to 'dst_file'.
    bool zip(const std::string &src_file, const std::string &dst_file){
      std::string cmd = "gzip -c " + src_file + " > " + dst_file;
      return !system(cmd.c_str());
    }

    // Decompresses 'src_file' to 'dst_file'.
    bool unzip(const std::string &src_file, const std::string &dst_file){
      std::string cmd = "gunzip -c " + src_file + " > " + dst_file;
      return !system(cmd.c_str());
    }

    // Removes file src_file from the disk.
    bool rm(const std::string &src_file){
      std::string cmd = "rm " + src_file;
      return !system(cmd.c_str());
    }

    // Removes file src_file from the disk.
    bool mkdir(const std::string &dir_name){
      std::string cmd = "mkdir " + dir_name;
      return !system(cmd.c_str());
    }

    void findOrCreate(const std::string &dir_name){
      if(!directoryExists(dir_name) && !mkdir(dir_name)){
        FATAL("Cannot find or create directory : " + dir_name);
      }
    }

    // Returns a valid file pointer for the file with full path 'filename'
    // Gives FATAL ERROR and halts if the file cannot be opened.
    FILE* fopenOrDie(std::string filename, std::string format) {
      FILE *file = fopen(filename.c_str(), format.c_str());
      ASSERT_TRUE(file, "Cannot open file: " + filename);
      return file;
    }
  }
}
#endif