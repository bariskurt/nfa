#ifndef PNET_LOGGER_HPP_
#define PNET_LOGGER_HPP_

#include <pnet_utils.hpp>
#include <pnet_time.hpp>

namespace pnet {

  class Logger {
    public:
      static void INIT( std::string filename ) {
        ASSERT_TRUE(!logger_instance_, "Logger Already Initialized");
        logger_instance_ = new Logger(filename);
      }

      static void INFO(const std::string &message) {
        ASSERT_TRUE(logger_instance_, "Logger Uninitialized");
        fprintf(logger_instance_->file_, "[INFO] %s\n", message.c_str());
      }

      static void STDOUT(const std::string &message) {
        ASSERT_TRUE(logger_instance_, "Logger Uninitialized");
        fprintf(logger_instance_->file_, "[INFO] %s\n", message.c_str());
        printf("[INFO] %s\n", message.c_str());
      }

      static void ERROR(const std::string &message) {
        ASSERT_TRUE(logger_instance_, "Logger Uninitialized");
        fprintf(logger_instance_->file_, "<<ERROR>> %s\n", message.c_str());
      }

      static void FATAL(const std::string &message) {
        printf("FATAL: %s\n", message.c_str());
        ASSERT_TRUE(logger_instance_, "Logger Uninitialized");
        fprintf(logger_instance_->file_, "<<---FATAL--->> %s\n",
                message.c_str());
        printf("Terminated with FATAL error.\nSee log file: %s\n",
               logger_instance_->filename_.c_str());
        CLOSE();
        exit(-1);
      }

      static void CLOSE() {
        if (logger_instance_) {
          delete logger_instance_;
          logger_instance_ = nullptr;
        }
      }

      ~Logger() {
        if (file_ != stdout && file_ != nullptr) {
          fclose(file_);
          file_ = nullptr;
        }
      }

    private:
      Logger(std::string filename) {
        filename_ = filename;
        file_ = utils::fopenOrDie(filename_, "w");
        setvbuf(file_, nullptr, _IONBF, 0);
      }

    private:
      static Logger *logger_instance_;
      FILE *file_;
      std::string filename_;
  };

  Logger *Logger::logger_instance_ = nullptr;
}

#endif // PNET_LOGGER_HPP_
