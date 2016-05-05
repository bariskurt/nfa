#ifndef PNET_NET_INTERFACE_H_
#define PNET_NET_INTERFACE_H_

#include <pcap.h>

#include <string>

#define PCAP_SNAPLEN 1500
#define PCAP_NEXT_EOF -2
#define PCAP_NEXT_ERR -1
#define PCAP_NEXT_OK 1

namespace pnet{

  class NetworkInterface {

    public:
      NetworkInterface(){}
      virtual ~NetworkInterface(){}

    protected:
      virtual void start() = 0;

      virtual void stop() {
        listening_ = false;
      }

      virtual void close() {
        listening_ = false;
      }

    protected:
      volatile bool listening_;
      bool live_;
  };


  class PcapInterface: public NetworkInterface{
    public:
      explicit PcapInterface(const std::string &dev);
      ~PcapInterface();

    public:
      void start() override;

    private:
      pcap_t *pcap_handle_;
  };

}

#endif
