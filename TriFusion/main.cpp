#include <iostream>
#include <fstream>
#include <vector>

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int callFork(unsigned int min, unsigned int max);

void createBinaryFile(const std::string& input)
{
     // Open file
     std::string output = input + "_random.bin";

     std::ofstream ofile(output.c_str(), std::ios::binary);
     std::ifstream ifile(input.c_str(), std::ios::binary);

     // Read and put each number into value vector
     // If we can't read the file, we won't convert anything
     if(ifile) {
          int tmp = 0;
          ifile >> tmp;
          while(!ifile.eof()) {
               // Use a space to not have a HUGE number
               // (but rather a number list)
               ofile.write((char*)&tmp, sizeof(int));
               // Read next value
               ifile >> tmp;
          }
          ofile.close();
          ifile.close();

     }
}

void createAnsiFile(const std::string& input)
{
     // Open file
     std::string output = input;
	output.resize(output.size()-4); // remove the .bin

     std::ofstream ofile(output.c_str(), std::ios::binary);
     std::ifstream ifile(input.c_str(), std::ios::binary);

     if(ifile) {
          ifile.seekg(0, std::ios::beg);
          int tmp;
          while(ifile.read((char*)&tmp, sizeof(int))) {
               ofile << tmp << " ";
          }
          ofile.close();
          ifile.close();

     }
}

int main(int argc, char** argv)
{
     std::string input;
     std::string output;
     // Check argument
     if(argc == 2) {
          input = output = argv[1];
     } else {
          std::cerr << "Wrong number of argument" << std::endl;
          return -1;
     }
     // Convert the plain text
     createBinaryFile(argv[1]);

     // Create filename
     output += "_sorted.bin";
     input += "_random.bin";

     // Open file
     std::ofstream ofile(output.c_str(), std::ios::binary);
     std::ifstream ifile(input.c_str(), std::ios::binary);


     unsigned int min = 0, max = 0;
     // Read
     if(ifile) {
          // Go to the end, tell us the size
          ifile.seekg(0, std::ios_base::end);
          max = ifile.tellg() / 4;
	printf("max = %d", max);
          // And return at the beginning of the file
          ifile.seekg(0, std::ios::beg);
          int tmp;
          std::cout << "Values are: " << std::endl;
          while(ifile.read((char*)&tmp, sizeof(int))) {
               std::cout << tmp << std::endl;
          }
     }

     pid_t originalPID = getpid();

     // First part of the merging sort
     std::ifstream readedFile(input.c_str());
     callFork(min, max);

     // Now only close buffer and all if I'm the original one
     if(originalPID == getpid()) {
          readedFile.close();

          ofile.close();
          ifile.close();

          // So now, file is now sorted and writted in _sorted.bin
          createAnsiFile(output);
     }
     return 0;
}

int callFork(unsigned int min, unsigned int max)
{
     pid_t pid_courant = getpid();
     pid_t pid_pere;
     printf("I'm the first process number %d\n", pid_courant);

     int nbFils = 0;
     unsigned int minFils, maxFils;

     while (min < max && nbFils < 2) {
          for (int i = 1; i <= 2; i++) {
               pid_pere = pid_courant;

               // Determining intervals for each son
               if (i == 1) {
                    minFils = min;
                    maxFils = (min + max) / 2;
               } else {
                    minFils = ((min + max) / 2) + 1;
                    maxFils = max;
               }

               // Son's creation
               switch(fork()) {
               case 0: // Son's code
                    // Value's update
                    pid_courant = getpid();
                    min = minFils;
                    max = maxFils;
		    nbFils = 0;
                    printf("%d Created %d (%d/2); Interval : %d - %d\n", pid_pere, pid_courant, i, min, max);
                    break;
               default: // Father's code
                    nbFils++;
                    break;
               }
               if (getpid() != pid_pere) // If I'm the first son
                    break;
          }
     }

     // If I'm the father
     if (pid_pere == pid_courant) {
          printf("%d : Waiting\n", pid_courant);

          // Wait until son finished
          pid_t process;
          for (int j = 1; j <= 2; j++) {
               process = wait(NULL);
               printf("%d : %d ended (%d/2)\n", pid_courant, process, j);
          }

          printf("%d : Wait end\n", pid_courant);
     }

     return 0;
}
