#ifndef PNET_UTILS_H_
#define PNET_UTILS_H_

#include <algorithm>
#include <dirent.h>
#include <regex>
#include <vector>


namespace pnet {

  inline bool dir_exists(const std::string &dirname){
    DIR *dir = opendir(dirname.c_str());
    if (dir) {
      closedir(dir);
      return true;
    }
    return false;
  }


  inline bool file_exists(const std::string &filename) {
    FILE *file_ = fopen(filename.c_str(), "rb");
    if (!file_) {
      return false;
    }
    fclose(file_);
    return true;
  }

  inline std::vector<std::string> ls(std::string dirname,
                              std::vector<std::string> extensions = {}){
    std::vector<std::string> files;
    // Open the directory of the files.
    DIR *dir = opendir(dirname.c_str());
    if (!dir) {
      Logger::FATAL("Cannot open directory: " + dirname);
    }
    // Get the files matching the regex.
    struct dirent *dp;
    while ((dp = readdir (dir)) != NULL) {
      if(StringEndsWith(dp->d_name, extensions)){
        if(fullpath){
          files.push_back(PathJoin(dirname, dp->d_name));
        } else {
          files.push_back(dp->d_name);
        }
      }
    }
    closedir(dir);
    sort(files.begin(), files.end());
    return files;
  }

  inline std::string path_join(const std::string &dirname,
                               const std::string &filename){
    if(dirname.back()=='/'){
      return dirname + filename;
    } else {
      return dirname + std::string("/") + filename;
    }
  }

  // Compresses 'src_file' to 'dst_file'.
  inline bool zip(const std::string &src_file, const std::string &dst_file){
    std::string cmd = "gzip -c " + src_file + " > " + dst_file;
    return !system(cmd.c_str());
  }

  inline bool unzip(const std::string &src_file, const std::string &dst_file){
    std::string cmd = "gunzip -c " + src_file + " > " + dst_file;
    return !system(cmd.c_str());
  }

  inline bool rm(const std::string &src_file){
    std::string cmd = "rm " + src_file;
    return !system(cmd.c_str());
  }

}  // namespace spnet

#endif //PNET_UTILS_H_
