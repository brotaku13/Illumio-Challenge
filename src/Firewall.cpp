#include "Firewall.h"

/**
 * The constructor takes in a file name and produces the Rule Interval Tree upon initialization
 */
Firewall::Firewall(std::string filename) : reader(filename)
{
    initializeRuleTree();
}

/**
 * Takes in a packet definition and finds whether the packet is allowed through the firewall or not.
 * This function has consistant O(logn) time complexity, where n is the number of rules.
 * @param direction string: the direction the packet is flowing, either "outbound" or "inbound"
 * @param protocal string: the protocal of the packet, either "tcp" or "udp"
 * @param port int: a a6 bit integer representing the port number, between 1 and 64435
 * @param ip_address string: the ip address of the packet
 * @return bool: whether the packet is allowed through the firewall 
 */
bool Firewall::accept_packet(std::string& direction, std::string& protocal, uint16_t port, std::string& ip_address)
{
    FirewallRule rule = initRule(direction, protocal, port, ip_address);
    return ruleTree.contains(rule);
}

bool Firewall::accept_packet(std::string direction, std::string protocal, uint16_t port, std::string ip_address)
{
    FirewallRule rule = initRule(direction, protocal, port, ip_address);
    return ruleTree.contains(rule);
}
/** 
 * parses an ipv4 ip string (128.0.0.1) and produces the 32 bit integer associated with the ip_address.
 * taken from https://stackoverflow.com/questions/5328070/how-to-convert-string-to-ip-address-and-vice-versa
 * @param ipAddress char*: a c-string representing the ipAddress
 * @return uint32_t: the integer the IP address represents
 */
uint32_t Firewall::parseIPV4string(string& ipAddress)
{
    const char* ipStr = ipAddress.c_str();
    struct sockaddr_in sa;
    char str[INET_ADDRSTRLEN];
    // store this IP address in sa:
    inet_pton(AF_INET, ipStr, &(sa.sin_addr));
    
    //address is unpacked in network byte order, so we need to flip it to host byte order to get an accurate representation
    return ntohl(sa.sin_addr.s_addr);
}

/**
 * This builds the rule tree from the rules defined in the csv file. 
 * I chose to iterate through the file line by line as opposed to loading the whole file into 
 * memory. I chose this approach since the file could be arbitrarily large, and so the 
 * ruleTree will grow with the size of the file. It may be dangerous to our memory capacity
 * to have both the file and the datastructure held in main memory at the same time. 
 */
void Firewall::initializeRuleTree()
{
    vector<string> line;
    reader.getNext(line);
    while(line.size() > 0)
    {
        FirewallRule new_rule = initRule(line);
        ruleTree.insertRule(new_rule);
        reader.getNext(line);
    }
}

void Firewall::insertRule(vector<string>& line)
{
    FirewallRule new_rule = initRule(line);
    ruleTree.insertRule(new_rule);
}

/**
 * Mini Factory for a FirewallRule struct based on a line from the csv file
 * @param line vector<string>: the line from the csv file, with each field in a separate index
 * @return FirewallRule: the rule to be placed in the tree.
 */
FirewallRule Firewall::initRule(const vector<string>& line)
{
    FirewallRule rule;
    rule.direction = line[DIRECTION];
    rule.protocal = line[PROTOCAL];
    getIpRange(line[IP], rule.ip_range.start, rule.ip_range.end);
    getPortRange(line[PORT], rule.port_range.start, rule.port_range.end);
    return rule;
}

/**
 * Mini Factory for a FirewallRule struct based on input from the accept_packet function
 * @param direction string: the direction the packet is flowing, either "outbound" or "inbound"
 * @param protocal string: the protocal of the packet, either "tcp" or "udp"
 * @param port int: a a6 bit integer representing the port number, between 1 and 64435
 * @param ip_address string: the ip address of the packet
 * @return FirewallRule: the rule to be searched for in the tree
 */
FirewallRule Firewall::initRule(std::string& direction, std::string& protocal, uint16_t port, std::string& ip_address)
{
    FirewallRule rule;
    rule.direction = direction;
    rule.protocal = protocal;
    rule.port_range.start = rule.port_range.end = port;
    rule.ip_range.start = rule.ip_range.end = parseIPV4string(ip_address);
    return rule;
}

/**
 * Splits a string of ip addresses or ports into its range and places them into a vector
 * EX: 127.0.0.1-127.0.0.2 -> vector<string> vec {"127.0.0.1", "127.0.0.2"}
 * @param str string: the string to be split
 * @param c char: the character to split on
 * @return vector<string>: a vector with maximum size of 2
 */
vector<string> Firewall::splitRange(const string& str, char c) const
{
    vector<string> vec;
    string::size_type split = str.find(c);
    if(split == std::string::npos)
    {
        vec.push_back(str);
    }
    else
    {
        vec.push_back(str.substr(0, split));
        vec.push_back(str.substr(split+1));
    }   
    return vec;
}

/**
 * Gets the range of IP addresses represented as 32 bit integers
 * @param field string: the field read from the csv file
 * @param start uint32_t&: the struct field to place the number into
 * @param end uint32_t&: the struct field to place the number into
 */
void Firewall::getIpRange(const string& field, uint32_t& start, uint32_t& end)
{
    vector<string> split = splitRange(field, '-');
    if(split.size() == 1)
    {
        start = end = parseIPV4string(split[0]);
    }
    else 
    {
        start = parseIPV4string(split[0]);
        end = parseIPV4string(split[1]);
    }
}
/**
 * Gets the range of ports represented as 32 bit integers
 * @param field string: the field read from the csv file
 * @param start uint32_t&: the struct field to place the number into
 * @param end uint32_t&: the struct field to place the number into
 */
void Firewall::getPortRange(const string& field, uint16_t& start, uint16_t& end)
{
    vector<string> split = splitRange(field, '-');
    if(split.size() == 1)
    {
        start = end = atoi(split[0].c_str());
    }
    else
    {
        start = atoi(split[0].c_str());
        end = atoi(split[1].c_str());
    }
}