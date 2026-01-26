#pragma once


#include<string>
#include<filesystem>
#include<sstream>
#include<fstream>
#include<iostream>
#include <format>



void open_file(const std::string& path,std::string& finalb_sourcecode);

std::string preprocessor(std::string path,const std::string& b_sourcecode);


template<class... Args>
[[noreturn]]
void errorf(std::format_string<Args...> fmt, Args&&... args) {
    std::cout << std::format(fmt, std::forward<Args>(args)...) << '\n';
    exit(EXIT_FAILURE);
}