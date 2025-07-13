file_name= "please give file name"

mode = "nodebug"

run: compiler
	builds/compiler $(file_name) $(mode)

compiler: src/*
	g++  src/main.cpp -o builds/compiler
	
clean :
	rm compiler