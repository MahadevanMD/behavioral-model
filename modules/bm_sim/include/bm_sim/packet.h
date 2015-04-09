#ifndef _BM_PACKET_H_
#define _BM_PACKET_H_

#include <memory>
#include <mutex>

#include <cassert>

#include "packet_buffer.h"
#include "phv.h"

typedef unsigned long long packet_id_t;

class Packet {
public:
  Packet();

  Packet(int ingress_port, packet_id_t id, packet_id_t copy_id,
	 PacketBuffer &&buffer);

  Packet(int ingress_port, packet_id_t id, packet_id_t copy_id,
	 PacketBuffer &&buffer, const PHV &src_phv);

  ~Packet();

  packet_id_t get_packet_id() const { return packet_id; }

  int get_egress_port() const { return egress_port; }

  int get_ingress_port() const { return ingress_port; }

  void set_egress_port(int port) { egress_port = port; }

  packet_id_t get_copy_id() const { return copy_id; }

  const std::string get_unique_id() const {
    return std::to_string(packet_id) + "." + std::to_string(copy_id);
  }

  void set_copy_id(packet_id_t id) { copy_id = id; }

  void set_payload_size(size_t size) { payload_size = size; }

  size_t get_payload_size() const { return payload_size; }

  size_t get_data_size() const { return buffer.get_data_size(); }

  char *data() { return buffer.start(); }

  const char *data() const { return buffer.start(); }

  char *payload() { 
    assert(payload_size > 0);
    return buffer.end() - payload_size;
  }

  char *prepend(size_t bytes) { return buffer.push(bytes); }

  char *remove(size_t bytes) {
    assert(buffer.get_data_size() >= payload_size + bytes);
    return buffer.pop(bytes);
  }

  unsigned long long get_signature() const {
    return signature;
  }

  const PacketBuffer &get_packet_buffer() const { return buffer; }

  PHV *get_phv() { return phv.get(); }

  Packet clone(packet_id_t new_copy_id) const;

  Packet(const Packet &other) = delete;
  Packet &operator=(const Packet &other) = delete;

  Packet(Packet &&other) noexcept;
  Packet &operator=(Packet &&other) noexcept;

private:
  int ingress_port;
  int egress_port;
  packet_id_t packet_id;
  packet_id_t copy_id;

  unsigned long long signature{0};

  PacketBuffer buffer;

  size_t payload_size{0};

  std::unique_ptr<PHV> phv;

private:
  class PHVPool {
  public:
    PHVPool(const PHVFactory &phv_factory);
    std::unique_ptr<PHV> get();
    void release(std::unique_ptr<PHV> phv);

  public:
    static PHVPool *get_instance();

  private:
    std::mutex mutex;
    std::vector<std::unique_ptr<PHV> > phvs;
    const PHVFactory &phv_factory;
  };

public:
  static void set_phv_factory(const PHVFactory &phv_factory);
  static void unset_phv_factory();

private:
  // static variable
  // Google style guidelines stipulate that we have to use a raw pointer and
  // leak the memory
  static PHVPool *phv_pool;
};

#endif