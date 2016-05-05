#include <iostream>

#include "pnet/logger.h"
#include "pnet/interface.h"

using namespace pnet;

int main(int argc, char* argv[]){

  PcapInterface net_iface = PcapInterface(argv[1]);
  net_iface.start();

  return 0;
}