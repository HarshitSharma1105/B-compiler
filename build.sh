g++ -O3 main.cpp -o main.exe     # Compile with optimization
currdir=$(pwd)                  # Get current directory
./main.exe $1 $2 $currdir # Run the executable with 3 arguments
