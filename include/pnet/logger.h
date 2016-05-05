// Copyright 2014

#ifndef PNET_LOGGER_H_
#define PNET_LOGGER_H_

#include <fstream>
#include <iostream>

namespace pnet {
/*
  namespace logger{
    static std::ofstream fout_;
    static std::string filename_;

    static void initialize(const std::string &filename = "/tmp/pnet.log"){
      logger::filename_ = filename;
      logger::fout_.rdbuf()->pubsetbuf(0, 0);
      logger::fout_.open(filename.c_str(), std::ios_base::out);
      if( !logger::fout_.is_open() ){
        std::cout << "FATAL ERROR: Cannot open log file: "
        << filename << std::endl;
        exit(-1);
      }
    }

    static void close(){
      if(fout_.is_open()){
        fout_.close();
      }
    }
  }
*/

  inline void INFO(const std::string &message){
    std::cout  << "[INFO] " <<  message << std::endl;
  }

  inline void ERROR(const std::string &message){
    std::cout << "<<ERROR>> " <<  message << std::endl;
  }

  inline void STDOUT(const std::string &message){
    std::cout << "[INFO] " << message << std::endl;
    INFO(message);
  }

  inline void FATAL(const std::string &message){
    std::cout << "<<---FATAL--->> " << message << std::endl;
    /*
    std::cout << "Terminated with FATAL error.\nSee log file:"
              << logger::filename_ << std::endl;
    logger::close();
     */
    exit(-1);
  }

}
#endif  // LOGGER_H_