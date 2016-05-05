// Copyright 2014

#ifndef PNET_LOGGER_H_
#define PNET_LOGGER_H_

#include <fstream>
#include <iostream>

namespace pnet {


  inline void STDOUT(const std::string &message){
    cout_ << "[INFO] " << message << std::endl;
    INFO(message);
  }

  inline void INFO(const std::string &message){
    Logger::log("[INFO] " + message);
  }

  inline void ERROR(const std::string &message){
    Logger::log("<<ERROR>> " + message);
  }

  inline void FATAL(const std::string &message){
    Logger::log( "<<---FATAL--->> " + message);
    std::cout << "Terminated with FATAL error.\nSee log file:"
              << Logger::filename() << std::endl;
    Logger::close();
    exit(-1);
  }

  class Logger {

    public:
      static void log(const std::string &message){
        Logger::CreateOrGet().fout_ << message << std::endl;
      }

      static const std::string& filename(){
        Logger::CreateOrGet().filename_;
      }

      static void close(){
        Logger::CreateOrGet().close();
      }

      Logger& CreateOrGet(){
        if(!logger){
          Initialize();
        }
        return *logger;
      }

      void Initialize(const std::string &filename = "/tmp/pnet.log"){
        logger = new Logger(filename);
      }

    private:
      explicit Logger(std::string filename) {
        filename_ = filename;
        fout_.rdbuf()->pubsetbuf(0, 0);
        fout_.open(filename, "w");
        if( !fout_.is_open() ){
          std::cout << "FATAL ERROR: Cannot open log file: "
          << filename << std::endl;
          exit(-1);
        }
      }

    public:
      ~Logger(){
        if(fout_.is_open()){
          fout_.close();
        }
      }

    public:
      void close(){
        if(fout_.is_open()){
          fout_.close();
        }
      }

    private:
      ifstream fout_;
      std::string filename_;
      static Logger *logger = nullptr;
  };



}
#endif  // LOGGER_H_