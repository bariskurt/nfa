#include <pnet.hpp>

void test_read_write(){

  std::cout << "test_read_write...\n";

  pnet::Packet packet;
  inet_pton(AF_INET, "79.123.176.238", &packet.ip_src);
  inet_pton(AF_INET, "54.201.100.251", &packet.ip_dst);
  packet.port_src = htons(80);
  packet.port_dst = htons(443);
  packet.protocol = 6;
  packet.flags = 0;
  packet.size = 80;
  packet.t_arrival = 0;


  pnet::Packet packet2;
  packet2.fromString(packet.toString());

  if (packet.toString() != packet2.toString()){
    std::cout << "ReadWriteTest FAILED !\n";
    std::cout << "source: " << packet.toString() << std::endl;
    std::cout << "dest  : " << packet2.toString()  << std::endl;
  }

  std::cout << "OK.\n" ;
}


int main(){
  test_read_write();
  return 0;
}