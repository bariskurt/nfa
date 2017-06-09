include_directories(../include/)

link_libraries(pcap)

add_executable(test_packet test_packet.cc)

add_executable(test_logger test_logger.cc)