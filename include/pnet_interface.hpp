#ifndef PNET_INTERFACE_HPP_
#define PNET_INTERFACE_HPP_

#include <pnet_shared_buffer.hpp>

/*  This class is implemented with a Factory Design pattern
  You cannot create a NetworkInterface by calling constructor.
  The only way to create one is to call the static function
  FSDServerController::CreateInterface() which creates one for
  you according to the Flags.
*/

namespace pnet {

  class NetworkInterface {

    friend class NetworkListener;

    public:
      NetworkInterface() : listening_(false), live_(false) { }
      virtual ~NetworkInterface() {}

    protected:
      virtual bool open(const std::string dev) = 0;
      virtual void loop() = 0;
      virtual void logStats() = 0;
      virtual void close() {
        listening_ = false;
        is_open_ = false;
        logStats();
        Logger::STDOUT("NetworkListener > Packets produced = "
                       + std::to_string(shared_buffer_->getNumPacketsProcessed()));
        Logger::STDOUT("NetworkListener > Closed.");
      }

    protected:
      void scheduler();
      void stop();

    protected:
      SharedBuffer *shared_buffer_;
      bool is_open_;
      volatile bool listening_;  // is interface in listening mode?
      bool live_;                // is interface a live (or file) interface?

  };



  NetworkInterface* NetworkInterface::createInterface(const std::string &name){
    NetworkInterface *net_iface = nullptr;
    std::vector<std::string> pkt_extentsions = {".txt", ".pkt", ".pkta", ".pkt.gz", ".pkta.gz"};
    if (utils::dir_exists(iname) || utils::ends_with(name, pkt_extentsions )){
      net_iface = new PktInterface(name);
    } else {
      net_iface = new PcapInterface(name);
    }
    net_iface->shared_buffer_ = SharedBuffer::CreateOrGet();
    return net_iface;
  }

} // namespace pnet

#endif  // NETWORKINTERFACE_H_