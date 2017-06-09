#ifndef PNET_SHARED_BUFFER_HPP_
#define PNET_SHARED_BUFFER_HPP_

#include <pnet_flow.hpp>
#include <pnet_logger.hpp>

#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/shm.h>

#define NUM_PACKETS 50000

namespace pnet{

  // Block of memory to bu used as circular queue.
  typedef struct memory{
    Packet packets[NUM_PACKETS];
    sem_t sem_read;
    sem_t sem_write;
  } Memory;

  class SharedBuffer {

    public:
      static SharedBuffer* createOrGet(){
        if( !shared_buffer_ )
          shared_buffer_ = new SharedBuffer();
        return shared_buffer_;
      }

      static void destroy(){
        if(shared_buffer_) {
          delete shared_buffer_;
          shared_buffer_ = nullptr;
        }
      }

    private:
      SharedBuffer() {
        // create or get a shared memory segment
        shared_mem_id = shmget(IPC_PRIVATE, sizeof(Memory), 0644 );
        if(shared_mem_id == -1)
          Logger::FATAL("SharedMemory > Unable to allocate memory\n");
        //attach
        memory = (Memory*) shmat(shared_mem_id, (void *)0, 0);
        // mark for deletion after detaching
        shmctl(shared_mem_id, IPC_RMID, NULL);
        // create or connect to a semaphore set
        sem_init(&memory->sem_write, 1, NUM_PACKETS);
        sem_init(&memory->sem_read, 1, 0);
        // set counters
        current_packet = memory->packets;
        memory_end =  memory->packets +  NUM_PACKETS;
        num_packets_processed = 0;
      }

      ~SharedBuffer() {
        shmdt(memory);
      }

    public:
      bool produce(Packet &pkt) {
        // Exit on interrupt
        if(sem_wait(&memory->sem_write))
          return false;
        memcpy(current_packet, &pkt, sizeof(Packet));
        // iterate current packet circularly
        if(++current_packet == memory_end)
          current_packet = memory->packets;
        sem_post(&memory->sem_read);
        return true;
      }

      bool consume(Packet &pkt) {
        // Exit on interrupt
        if (sem_wait(&memory->sem_read))
          return false;
        memcpy(&pkt, current_packet, sizeof(Packet));
        if(++current_packet == memory_end)
          current_packet = memory->packets;
        ++num_packets_processed;
        sem_post(&memory->sem_write);
        return true;
      }

      void printSemaphores(){
        int sem_val;
        if(!sem_getvalue(&memory->sem_read, &sem_val))
          printf("sem_read : %d\n",sem_val);
        if(!sem_getvalue(&memory->sem_write, &sem_val))
          printf("sem_write : %d\n",sem_val);
      }

      uint64_t getNumPacketsProcessed(){
        return num_packets_processed;
      }

    private:
      int shared_mem_id;
      static SharedBuffer* shared_buffer_;

    private:
      Memory *memory;
      Packet *current_packet;
      Packet *memory_end;
      uint64_t num_packets_processed;

  };
  SharedBuffer* SharedBuffer::shared_buffer_ = nullptr;
}

#endif // PNET_SHARED_BUFFER_HPP_