#include <pnet_logger.hpp>

void test_logger(){
  std::cout << "test_logger...\n";

  std::string log_name = "/tmp/deneme.log";

  pnet::Logger::INIT(log_name);
  pnet::Logger::INFO("This is INFO");
  pnet::Logger::STDOUT("This is STDOUT");
  pnet::Logger::ERROR("This is ERROR");
  pnet::Logger::FATAL("This is FATAL");

  pnet::utils::rm(log_name);
  std::cout << "OK.\n";
}


int main(){
  test_logger();
  return 0;
}