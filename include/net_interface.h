#ifndef PNET_NET_INTERFACE_H_
#define PNET_NET_INTERFACE_H_

namespace pnet{

  class NetworkInterface {
    public:
      static NetworkInterface CreateInterface();

    protected:
      NetworkInterface();
      virtual ~NetworkInterface();

    protected:
      virtual void start() = 0;
      virtual void stop();
      virtual void close();

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


  class PktpInterface: public NetworkInterface{
    public:
      explicit PktInterface(const std::string &dev);
      ~PktpInterface();

    public:
      void start() override;

    private:
      std::vector<std::string> pkt_list;
  };

}

#endif
