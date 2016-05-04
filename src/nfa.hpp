#ifndef NFA_NFA_H_
#define NFA_NFA_H_

#include <arpa/inet.h>

/*
  v1: This is the famous 5-tuple that identifies a packet (and flow).
*/

class Key {
  public:
    Key::Key(): port_src_(0), port_dst_(0), protocol_(0) {
      ip_src_.s_addr = 0;
      ip_dst_.s_addr = 0;
    }

    Key::Key(struct in_addr ip_src, struct in_addr ip_dst,
             uint16_t port_src, uint16_t port_dst, uint8_t protocol)
        : ip_src_(ip_src), ip_dst_(ip_dst),
          port_src_(port_src), port_dst_(port_dst),
          protocol_(protocol) {}

    // This function checks whether two Keys are equal in a bidirectional way.
    // Source and destination IP's and ports are checked in two possible ways.
    bool Key::operator==(const Key& k) const {
      if (protocol != k.protocol) {
        return (false);
      }
      if ((port_src == k.port_src) && (port_dst == k.port_dst)
          && (ip_src.s_addr == k.ip_src.s_addr)
          && (ip_dst.s_addr == k.ip_dst.s_addr)) {
        return (true);
      }
      if ((port_src == k.port_dst) && (port_dst == k.port_src)
          && (ip_src.s_addr == k.ip_dst.s_addr)
          && (ip_dst.s_addr == k.ip_src.s_addr)) {
        return (true);
      }
      return (false);
    }

  public:
    struct in_addr ip_src_;     // 4 bytes
    struct in_addr ip_dst_;     // 4 bytes
    uint16_t       port_src_;   // 2 bytes
    uint16_t       port_dst_;   // 2 bytes
    uint32_t       protocol_;   // 4 bytes
    // Total = 16 bytes
};


class Packet {
public:
    // Read/Write  To/From Buffer
    void Serialize(char* buffer) const;
    bool Deserialize(const char* buffer);

    // Read/Write  To/From Binary File
    void fwrite(FILE *file) const;
    bool fread(FILE *file);

    // Read/Write  To/From Text File
    void fprintf(FILE *file) const;
    bool fscanf(FILE *file);

public:
    Key key_;              // 16 bytes: 5 tuple
    uint16_t flags_;       //  2 bytes: TCP flags (zero for non-TCP packets)
    uint16_t size_;        //  2 bytes: packet size
    fsd::Time t_arrival_;  //  8 bytes: seconds + microseconds
    // 28 bytes: Total
};



#endif