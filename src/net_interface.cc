#include "net_interface.cc"

#include "Logger.h"

namespace{

  void NetworkInterface::stop() {
    listening_ = false;
  }

  void NetworkInterface::close() {
    listening_ = false;
  }

    // (1) Try to open a live interface.
  // (2) If (1) fails, try to open a pcap file
  // (3) Fatal error if (1) & (2) fails.
  PcapInterface::PcapInterface(const std::string &dev) {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_handle_ = pcap_open_live(dev.c_str(), PCAP_SNAPLEN, 1, -1, errbuf);
    if (pcap_handle_ == NULL) {
      pcap_handle_ = pcap_open_offline(dev.c_str(), errbuf);
      if (pcap_handle_ == NULL) {
        FATAL("PcapInterface > Cannot open interface " + dev);
      } else {
        live_ = false;
        STDOUT("PcapInterface > interface = " + dev);
      }
    } else {
      live_ = true;
      STDOUT("NetworkListener > Interface = " + dev);
      STDOUT("NetworkListener > Snapshot length  = "
                     + std::to_string(PCAP_SNAPLEN) + " bytes");
    }
  }

  void PcapInterface::start(){

    const u_char *pkt_data;
    struct pcap_pkthdr *pkthdr;

    Logger::STDOUT("NetworkListener > Start Time = " + Time::Now());
    listening_ = true;
    while(listening_) {
      switch(pcap_next_ex(pcap_handle_, &pkthdr, &pkt_data)){
        case PCAP_NEXT_OK:
          std::cout << "Packet geldi\n";
          break;

        case PCAP_NEXT_EOF:
          listening_ = false;
          STDOUT("PcapInterface > EOF reached");
          break;

        case PCAP_NEXT_ERR:
          ERROR("PcapInterface:: pcap error: " +
                    std::string(pcap_geterr(pcap_handle_)));
          break;

        default:
          // default is time out, nothing to be done...
          break;
      }
    }
  }

};