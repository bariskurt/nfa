include_directories(../include/)

link_libraries(pcap)

add_executable(test_pnet test.cc)