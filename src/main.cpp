#include <string>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <assert.h>

#include "Firewall.h"


using std::string;
using std::vector;

/**
 * Checks if a file exists or not
 * @param name string: the name of the file to be checked for existance
 * @return bool: whether the file exists
 */
inline bool exists(const std::string& name) {
    return ( access( name.c_str(), F_OK ) != -1 );
}

//convenience definitions
string in = "inbound";
string out = "outbound";
string tcp = "tcp";
string udp = "udp";

/**
 * Grabs the file from the command line arguments
 * @param argc int: number of command line arguments
 * @param argv char**: the command line argument list
 * @return string: the filepath as a string.
 */
string getFilename(int argc, char* argv[])
{  
    if(argc != 2)
    {
        std::cerr << "Incorrect argument format correct format is" << '\n' << '\t' << "./program [filename]" << '\n';
        std::exit(EXIT_FAILURE);
    }
    string filename(argv[1]);
    if(exists(filename))
        return filename;

    std::exit(EXIT_FAILURE);
}

void simpleRangeTest()
{
    Firewall fw;
    vector<string> vec = {"inbound", "tcp", "1-65535", "0.0.0.0-255.255.255.255"};
    fw.insertRule(vec);
    assert(fw.accept_packet("inbound", "tcp", 1, "0.0.0.0"));
    assert(fw.accept_packet("inbound", "tcp", 65535, "255.255.255.255"));
    assert(fw.accept_packet("inbound", "tcp", 10, "0.0.0.23"));
    assert(fw.accept_packet("inbound", "tcp", 45, "0.0.255.0"));
    assert(fw.accept_packet("inbound", "tcp", 4593, "22.45.234.5"));
    assert(fw.accept_packet("inbound", "tcp", 60000, "1.1.1.1"));

    printf("Passed Basic Range Test\n");
}

void restrictedRangeTest()
{
    Firewall fw;
    vector<string> vec1 = {"inbound", "tcp", "10-20", "0.0.0.0-0.0.0.150"};
    vector<string> vec2 = {"inbound", "udp", "10-20", "0.0.100.0-0.0.150.0"};

    vector<string> vec3 = {"outbound", "tcp", "10-20", "0.100.0.0-0.150.0.0"};
    vector<string> vec4 = {"outbound", "udp", "10-20", "100.0.0.0-150.0.0.0"};
    vector<vector<string>> rules = {vec1, vec2, vec3, vec4};
    for(vector<string>& rule: rules)
    {
        fw.insertRule(rule);
    }

    assert(fw.accept_packet(in, tcp, 10, "0.0.0.10"));
    assert(fw.accept_packet(in, tcp, 20, "0.0.0.150"));
    assert(!fw.accept_packet(in, tcp, 25, "0.0.0.120"));
    assert(!fw.accept_packet(in, tcp, 12, "0.100.0.0"));

    assert(fw.accept_packet(out, tcp, 10, "0.112.0.0"));
    assert(fw.accept_packet(out, tcp, 20, "0.134.0.0"));
    assert(!fw.accept_packet(out, tcp, 25, "0.0.0.120"));
    assert(!fw.accept_packet(out, tcp, 12, "0.0.0.0"));

    printf("Passed restricted range test\n");
}

void overlappingRuleTest()
{
    Firewall fw;
    vector<string> vec1 = {in, tcp, "10-20", "0.0.0.0-0.0.0.150"};
    fw.insertRule(vec1);
    assert(fw.accept_packet(in, tcp, 10, "0.0.0.10"));

    //test with overlap of only IP
    vector<string> overlap = {in, tcp, "10-20", "0.0.0.5-0.0.0.160"};
    fw.insertRule(overlap);
    assert(fw.accept_packet(in, tcp, 10, "0.0.0.160"));
    assert(fw.accept_packet(in, tcp, 12, "0.0.0.5"));
    assert(fw.accept_packet(in, tcp, 20, "0.0.0.0"));
    
    //test with overlap of only port
    Firewall fww;
    vector<string> original = {in, tcp, "10-20", "0.0.0.5-0.0.0.160"};
    fww.insertRule(original);
    vector<string> portOverlap = {in, tcp, "0-30", "0.0.0.7"};
    fww.insertRule(portOverlap);

    assert(fww.accept_packet(in, tcp, 0, "0.0.0.5"));
    assert(fww.accept_packet(in, tcp, 25, "0.0.0.5"));
    assert(fww.accept_packet(in, tcp, 30, "0.0.0.5"));
    assert(!fww.accept_packet(in, tcp, 45, "0.0.0.5"));


    printf("passed overlapping intervals test\n");
}

void runTests()
{
    simpleRangeTest();
    restrictedRangeTest();
    overlappingRuleTest();
}

int main(int argc, char *argv[])
{
    string filename = getFilename(argc, argv);
    Firewall firewall(filename);
    // runTests();
    exit(EXIT_SUCCESS);
}