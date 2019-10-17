#ifndef FIREWALL
#define FIREWALL

#define DIRECTION 0
#define PROTOCAL 1
#define PORT 2
#define IP 3

#include <string>
#include <vector>
#include <arpa/inet.h>

#include "CSVReader.h"
#include "RuleTree.h"

using std::string;
using std::vector;

class Firewall {
public:
    Firewall(){};
    Firewall(std::string filename);
    bool accept_packet(std::string& direction, std::string& protocal, uint16_t port, std::string& ip_address);
    bool accept_packet(std::string direction, std::string protocal, uint16_t port, std::string ip_address);
    void insertRule(vector<string>& line);
private:
    void initializeRuleTree();
    uint32_t parseIPV4string(string& ipAddress);
    FirewallRule initRule(const vector<string>& line);
    FirewallRule initRule(std::string& direction, std::string& protocal, uint16_t port, std::string& ip_address);
    vector<string> splitRange(const string& str, char c) const;
    void getIpRange(const string& field, uint32_t& start, uint32_t& end);
    void getPortRange(const string& field, uint16_t& start, uint16_t& end);

    CSVReader reader;
    RuleTree ruleTree;
};

#endif