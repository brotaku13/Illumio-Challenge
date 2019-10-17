#ifndef CSV_READER
#define CSV_READER

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

using std::string;
using std::vector;

class CSVReader {
public:
    CSVReader(){}
    CSVReader(const std::string& filename);
    void getNext(std::vector<string>& line);

private:
    std::ifstream file;
};

#endif