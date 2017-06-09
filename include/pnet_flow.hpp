#ifndef PNET_FLOW_HPP_
#define PNET_FLOW_HPP_


#include <arpa/inet.h>
#include <cstring>
#include <inttypes.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sstream>

#include <pnet_time.hpp>
#include <pnet_utils.hpp>


namespace pnet {

  // 5-tuple describing the communication link.
  //   - Source and Destination IP's
  //   - Source and Destination Ports
  //   - Level-3 protocol : UDP, TCP, etc..
  class Key{

    public:
      Key(){}

      Key(struct in_addr ip1, struct in_addr ip2,
          uint16_t port1, uint16_t port2, uint32_t proto)
          : ip_src(ip1), ip_dst(ip2),
            port_src(port1), port_dst(port2), protocol(proto) {}

      Key(const Key& key): ip_src(key.ip_src), ip_dst(key.ip_dst),
                           port_src(key.port_src), port_dst(key.port_dst),
                           protocol(key.protocol) {}

      // Equality of two keys regardless of direction.
      // If the source and destination directions are reversed,
      // it's still the same key.
      bool operator==(const Key& k) const {
        if (protocol != k.protocol) {
          return false;
        }
        if ((port_src == k.port_src) && (port_dst == k.port_dst)
            && (ip_src.s_addr == k.ip_src.s_addr)
            && (ip_dst.s_addr == k.ip_dst.s_addr)) {
          return true;
        }
        if ((port_src == k.port_dst) && (port_dst == k.port_src)
            && (ip_src.s_addr == k.ip_dst.s_addr)
            && (ip_dst.s_addr == k.ip_src.s_addr)) {
          return true;
        }
        return false;
      }


    public:
      struct in_addr  ip_src;     // 4 bytes
      struct in_addr  ip_dst;     // 4 bytes
      uint16_t        port_src;   // 2 bytes
      uint16_t        port_dst;   // 2 bytes
      uint32_t        protocol;   // 4 byte
      // 16 bytes in total.
  };

  // IP packet is a Key together with size, arrival time and TCP flag info.
  class Packet : public Key{

    public:
      Packet(){}

      Packet(const Key &key): Key(key), flags(0), size(0), t_arrival(0){}

      // Binary stream input.
      friend std::istream& operator>>(std::istream &in, Packet &packet) {
        in.read(reinterpret_cast<char *>(&packet), sizeof(Packet));
        return in;
      }

      // Binary stream output.
      // Use "std::cout << packet.to_string();" for printing.
      friend std::ostream& operator<<(std::ostream &out, const Packet &packet) {
        out.write(reinterpret_cast<const char *>(&packet), sizeof(Packet));
        return out;
      }

      // Packets are compared with their arrival times
      bool operator<(const Packet &rhs) const {
        return t_arrival  < rhs.t_arrival;
      }

      // Pretty formats packet info into a string.
      std::string to_string() const {
        char buffer[250];
        std::sprintf(buffer, "%-15s\t%5d\t",
                     inet_ntoa(ip_src), ntohs(port_src));
        std::sprintf(buffer+strlen(buffer), "%-15s\t%5d\t",
                     inet_ntoa(ip_dst),  ntohs(port_dst));
        std::sprintf(buffer+strlen(buffer), "%2d\t%hu\t%4hu\t",
                     protocol, flags, size);
        std::sprintf(buffer+strlen(buffer), "%s",
                     t_arrival.to_string().c_str());
        return std::string(buffer);
      }

      bool from_string(std::string source){

        std::istringstream sin(source);
        std::string tmp;

        sin >> tmp >> port_src ;
        inet_pton(AF_INET, tmp.c_str(), &ip_src);
        port_src = htons(port_src);

        sin >> tmp >> port_dst ;
        inet_pton(AF_INET, tmp.c_str(), &ip_dst);
        port_dst= htons(port_dst);

        sin >> protocol >> flags >> size >> tmp;
        t_arrival = Time(tmp);
        return true;
      }


  public:
      uint16_t flags;   // 2 bytes: TCP flags (zero for non-TCP packets)
      uint16_t size;    // 2 bytes: packet size [
      Time t_arrival;   // 8 bytes: seconds + microseconds
      // 28 bytes in total.
  };

  // Records packet information into binary files.
  // File names are created by using timestamps.
  // Each file contains at most "max_packets_per_file_" packets.
  // Packet files end with .pkt or .pkt.gz (if gzipped) extensions.
  class PacketRecorder {

    public:
      static const uint64_t max_records_per_file = 1000000;

    public:
      // Input:
      //    - output_dir: to store record files.
      //    - compressed: for compressed storage.
      PacketRecorder(const std::string &output_dir_,
                     bool compressed_ = false){
        if(!utils::dir_exists(output_dir_)){
          FATAL("Recorder:: output directory does not exits: " + output_dir );
        }
        output_dir = output_dir_;
        compressed = compressed_;
        filename = "";
        record_counter = max_records_per_file;
      }

      ~PacketRecorder(){
        close();
      }

      void write(const Packet &packet) {
        // Packet limit per file reached.
        // Save current file and open a new one.
        if (record_counter == max_records_per_file) {
          close();
          filename = utils::path(output_dir,
                                 packet.t_arrival.to_date() + ".pkt");
          out.open(filename, std::ios::binary);
          if(!out.is_open()){
            FATAL("Recorder:: cannot open file : " + filename );
          }
          record_counter = 0;
        }
        out << packet;
        ++record_counter;
      }

    private:
      void close(){
        if(out.is_open()){
          out.close();
          if(compressed){
            std::string dest = filename+".gz";
            utils::zip(filename, dest);
            utils::rm(filename);
          }
        }
      }

    private:
      // current # packets in the current record file
      uint64_t record_counter;

      bool compressed;
      std::string output_dir;
      std::string filename;
      std::ofstream out;
  };


  // Reads a .pkt or .pkt.gz file into memory.
  class PacketReader {

    public:
      static const std::string temp_file;

    public:

      // Reads all packets from file unless max_packets is set.
      PacketReader(const std::string &filename, int64_t max_packets = -1){

        // Pkt file does not exist, break;
        if( !utils::file_exists(filename) ){
          FATAL("PacketReader:: file not found: " + filename );
        }

        if( !utils::ends_with(filename, {"pkt", "pkt.gz"}) ){
          FATAL("PacketReader:: not a pkt file: "  + filename);
        }

        // Pkt is compressed, uncompress in temporary location;
        is_compressed = utils::ends_with(filename, {".gz"});

        // Open file:
        std::ifstream fin;
        std::string file_to_open = filename;
        if(is_compressed) {
          utils::unzip(filename, temp_file);
          file_to_open = temp_file;
        }
        fin.open(file_to_open, std::ios::binary);
        if( !fin.is_open() ){
          FATAL("PacketReader:: cannot open file: " + file_to_open);
        }

        // Read all packets into memory;
        Packet temp;
        while(max_packets != 0){
          fin >> temp;
          if(fin.eof()){
            break;
          }
          packets.push_back(temp);
          max_packets--;
        }

        // Remove temp file if used.
        if( is_compressed ) {
          utils::rm(temp_file);
        }
      }

    public:
      std::vector<Packet> packets;
      bool is_compressed;

  };
  const std::string PacketReader::temp_file = "/tmp/temp.pkt";


  class Flow : public Key{

    public:
      static const uint32_t TimeOutInMicroseconds = 60000000; // 60 seconds

    public:
      // A small struct to store necessary information for each packet.
      // Since keys of each packet is the same, we store only a vector of
      // PacketInfo inside the flow.
      struct PacketInfo{
          PacketInfo():updown(0), size(0), t_arrival(0){}
          PacketInfo(int16_t updown_ ,uint16_t size_, Time t_arrival_):
              updown(updown_), size(size_), t_arrival(t_arrival_){}
          int16_t updown;
          uint16_t size;
          Time t_arrival;
      };

    public:
      // A flow is always constructed with its first packet.
      explicit Flow(const Packet & packet)
          : Key(packet), nbytes(0), prev(nullptr), next(nullptr) {
        insert(packet);
      }

      // Size is the number of packets.
      uint64_t size() const{
        return packets.size();
      }

      // Bytes is the number of total packet sizes.
      uint64_t bytes() const{
        return nbytes;
      }

      Time t_first_packet(){
        return packets.front().t_arrival;
      }

      Time t_last_packet(){
        return packets.back().t_arrival;
      }

      // A flow expires if its last packet has arrived
      // earlier than TimeOutInMicroseconds before.
      bool expired(Time t_now) const {
        Time elapsed = t_now - packets.back().t_arrival;
        return elapsed > TimeOutInMicroseconds;
      }

      // Flows are compared according to the arrival of their last packets.
      bool operator<(const Flow &rhs) const {
        return packets.back().t_arrival  < rhs.packets.back().t_arrival;
      }

      // Determines the direction of the packet inside the flow.
      // Up = 1 is the direction of the first packet.
      // Down = -1 is the reverse direction.
      int16_t direction(const Packet &packet){
        if(ip_src.s_addr == packet.ip_src.s_addr){
          return 1;
        }
        return -1;
      }

      // Insert a new packet to the flow.
      // Determine the packet direction and accumulate stats.
      void insert(const Packet &packet) {
        nbytes += packet.size;
        packets.emplace_back(direction(packet),
                             packet.size,
                             packet.t_arrival);
      }

      std::string to_string() const {
        char buffer[500];
        std::sprintf(buffer, "%-15s\t%5d\t",
                     inet_ntoa(ip_src), ntohs(port_src));
        std::sprintf(buffer+strlen(buffer), "%-15s\t%5d\t",
                     inet_ntoa(ip_dst),  ntohs(port_dst));
        std::sprintf(buffer+strlen(buffer), "%2d\t%" PRIu64 "\t%" PRIu64 "\t",
                     protocol, size(), bytes());
        std::sprintf(buffer+strlen(buffer), "%s\t",
                     packets.front().t_arrival.to_string().c_str());
        std::sprintf(buffer+strlen(buffer), "%s",
                     packets.back().t_arrival.to_string().c_str());
        return std::string(buffer);
      }

    public:
      uint64_t nbytes;
      std::vector<PacketInfo> packets;

    public:
      Flow *prev;
      Flow *next;
  };


  class FlowRecorder {

    public:
      static const uint64_t max_records_per_file = 1000000;

    public:
      // Input:
      //    - output_dir: to store record files.
      //    - compressed: for compressed storage.
      FlowRecorder(const std::string &output_dir_,
                   bool compressed_ = false){
        if(!utils::dir_exists(output_dir_)){
          FATAL("FlowRecorder:: output directory does not exits: "
                + output_dir );
        }
        compressed = compressed_;
        output_dir = output_dir_;
        filename = "";
        record_counter = max_records_per_file;
      }

      ~FlowRecorder(){
        close();
      }

      void write(const Flow &flow) {
        // Packet limit per file reached.
        // Save current file and open a new one.
        if (record_counter == max_records_per_file) {
          close();
          filename = utils::path(output_dir,
                                 flow.packets.back().t_arrival.to_date()  + ".flw");
          out.open(filename, std::ios::out);
          if(!out.is_open()){
            FATAL("FlowRecorder:: cannot open file : " + filename );
          }
          record_counter = 0;
        }
        out << flow.to_string() << std::endl;
        ++record_counter;
      }

    private:
      void close(){
        if(out.is_open()){
          out.close();
          if(compressed){
            std::string dest = filename+".gz";
            utils::zip(filename, dest);
            utils::rm(filename);
          }
        }
      }

    private:
      // current # packets in the current record file
      uint64_t record_counter;
      bool compressed;
      std::string output_dir;
      std::string filename;
      std::ofstream out;
  };

  class FlowQueue{

    public:
      FlowQueue(): head(nullptr),tail(nullptr),num_elements(0){}

    public:
      // Insert flow at the end of the doubly linked list.
      // Clearly, this is O(1) time.
      // Returns pointer to the inserted Node
      Flow* emplace_back(const Packet &packet) {
        Flow *new_flow= new Flow(packet);
        insert(new_flow);
        return new_flow;
      }

      // Remove and destroy arbitrary Flow by its pointer
      void del(Flow *flow){
        if(!flow){
          FATAL("FlowQueue:: Trying to remove null Flow*");
        }
        remove(flow);
        delete flow;
      }

      // Take a Flow from an arbitrary position and push it at the end
      // of the queue
      void move_back(Flow *flow){
        if(!flow){
          FATAL("FlowQueue:: Trying to put_back null Flow*");
        }
        remove(flow);
        insert(flow);
      }

    private:
      void insert(Flow *new_flow){
        if (num_elements > 0) {
          // append to the end
          new_flow->prev = tail;
          new_flow->next = nullptr;
          tail->next = new_flow;
          tail = new_flow;
        } else {
          // insert first element into queue
          head = tail = new_flow;
        }
        ++num_elements;
      }

      void remove(Flow *flow){
        if(flow == head){
          head = flow->next;
          if(head) {
            head->prev = nullptr;
          }
        } else {
          flow->prev->next = flow->next;
        }
        if(flow == tail) {
          tail = flow->prev;
          if(tail) {
            tail->next= nullptr;
          }
        } else {
          flow->next->prev = flow->prev;
        }
        flow->next = flow->prev = nullptr;
        --num_elements;
      }

    public:
      bool empty() const{
        return num_elements == 0;
      }

      friend std::ostream& operator<<(std::ostream &out, const FlowQueue& q){
        Flow *flow = q.head;
        while(flow ){
          out << flow->to_string() << "  ";
          flow = flow ->next;
        }
        return out;
      }

    public:
      Flow* head;
      Flow* tail;
      uint64_t num_elements;

  };


  // A time-out map for flows.
  class FlowTable{
    public:
      static const uint64_t HASH_SIZE_HINT;
      static const uint64_t NUM_FSD_BINS;

    public:
      // Custom hash function for Keys.
      class KeyHash {
      public:
          std::size_t operator()(const Key& key) const {
            return (key.ip_src.s_addr + key.ip_dst.s_addr + key.port_src
                    + key.port_dst + key.protocol);
          }
      };

    public:
      FlowTable(FlowRecorder *recorder_=nullptr) {
        flow_recorder = recorder_;
        flow_hash.reserve(HASH_SIZE_HINT);
        packet_counter = 0;
      }

      ~FlowTable(){
        Flush();
      }

      // Clear the flow table and record the flows (if recorder is set)
      void Flush(){
        while(!flows.empty()){
          if(flow_recorder){
            flow_recorder->write(*flows.head);
          }
          flows.del(flows.head);
        }
      }

      friend std::ostream& operator<<(std::ostream &out,
                                      const FlowTable &ftable) {
        Flow* current = ftable.flows.head;
        while(current){
          out << current->to_string() << std::endl;
          current = current->next;
        }
        return out;
      }

      // Iterates over the FlowQueue and collects FSD statistics.
      // If no protocol is given, overall FSD is returned.
      std::vector<uint64_t> current_fsd(uint16_t proto = 0) const{
        std::vector<uint64_t> counts(NUM_FSD_BINS,0);
        Flow *current = flows.head;
        while(current) {
          if (proto == 0 || current->protocol == proto) {
            counts[std::min(NUM_FSD_BINS, current->size()) - 1]++;
          }
          current = current->next;
        }
        return counts;
      }

      // Insert a new packet to the hash table.
      // Create a new flow if necessary, otherwise update an existing flow.
      bool insert(const Packet &pkt){
        // Update timers and counters
        if(packet_counter == 0){
          t_first_packet = pkt.t_arrival;
        }
        t_last_packet = pkt.t_arrival;
        ++packet_counter;

        // Clear expired flows.
        expire_flows(pkt.t_arrival);

        Flow *flow = find(pkt);
        if(flow){
          flow->insert(pkt);
          // Update time-out location by putting the flow at the back of the
          // queue. The flow_hash location should also get updated.
          flows.move_back(flow);
          return false;
        } else {
          Flow *new_flow = flows.emplace_back(pkt);
          flow_hash.insert({pkt, new_flow});
          return true;
        }
      }

      // Remove all expired flows from the table.
      void expire_flows(Time t_now) {
        while (!flows.empty()) {
          if (flows.head->expired(t_now)) {
            // record flow just before deleting from table:
            if(flow_recorder){
              flow_recorder->write(*flows.head);
            }
            flow_hash.erase(*flows.head); // remove from flow hash
            flows.del(flows.head);        // delete flow
          } else {
            break;
          }
        }
      }

      Flow* find(const Key &key){
        auto it = flow_hash.find(key);
        if ( it  == flow_hash.end() ) {
          return nullptr;
        }
        return it->second;
      }

      Time up_time(){
        return t_last_packet - t_first_packet;
      }

    public:
      FlowQueue flows;
      std::unordered_map<Key, Flow*, KeyHash> flow_hash;
      FlowRecorder *flow_recorder;
      uint64_t packet_counter;
      Time t_first_packet;
      Time t_last_packet;

  };
  const uint64_t FlowTable::NUM_FSD_BINS = 50;
  const uint64_t FlowTable::HASH_SIZE_HINT = 10000000;

} // namespace pnet

#endif