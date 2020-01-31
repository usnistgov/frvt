#pragma once
#include <string>
#include <vector>


void save_img(const char * name, void * p_img);

void save_float(const char * name, const float * data, int size);

std::vector<std::string> str_split(const std::string& s, const char& delimiter);
