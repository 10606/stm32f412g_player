#ifndef HEADERS_H
#define HEADERS_H

#include <cstddef>
#include <string>
#include <iostream>

const char cur_song_info = 0x01;
const char displayed_song_info = 0x02;
const char pl_list_info = 0x03;
const char volume_info = 0x04;
const char state_info = 0x05;

const size_t song_name_sz = 28;
const size_t group_name_sz = 28;
const size_t pl_name_sz = 20; 
const size_t name_offset = 5;
const size_t count_offset = 30;
const size_t volume_width = 10;

void extract (std::string & data, size_t & state);

#endif

