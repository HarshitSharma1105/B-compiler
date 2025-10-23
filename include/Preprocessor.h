#pragma once


#include<string>
#include<filesystem>
#include<sstream>
#include<fstream>
#include<iostream>


void open_file(const std::string& path,std::string& finalb_sourcecode);

std::string preprocessor(std::string path,const std::string& b_sourcecode);

