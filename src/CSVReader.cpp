#include "CSVReader.h"
/**
 * Constructor takes in a string which is a file
 * @param filename string: the filename for the csv file with each row representing a new rule
 */
CSVReader::CSVReader(const std::string& filename): file(filename)
{
    if(file.fail())
        throw "File does not exist error";
}

/**
 * A poor man's iterator through the file, reading each line into memory only when asked for.
 * @param line vector<string>: a vector which the line in the csv will be broken up into and placed in.
 */
void CSVReader::getNext(std::vector<string>& line)
{
    line.clear();
    std::string readline;
    if(getline(file, readline))
    {
        if(file.bad())
            throw "File Read Error";

        std::istringstream stream(readline);
        std::string field;
        while (getline(stream, field, ','))
            line.push_back(field);
    } 
}