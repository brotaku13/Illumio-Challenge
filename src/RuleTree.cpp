#include "RuleTree.h"

/**
 * Intializes the RuleTree with the first 4 roots. Right now, these are hard coded, since these are 
 * the only known roots in the tree. It's faster and cleaner to place them in here instead of generating
 * them dynaimcally as new rules are added because we don't have any extra if statements in the insertRule
 * function. It's important that function be as streamlined as possible since it takes the most time
 * in the process.
 */
RuleTree::RuleTree()
{
    //hardcoding these for now, since these are the only permuations of the top levels of the tree
    //only disadvantage is not being able to exit early if a rule for any of these permutations hasn't been defined 
    //yet. However, it's only one extra call to check the interval tree, so not too big a deal.
    root["outboundtcp"] = {};
    root["inboundtcp"]  = {};
    root["outboundudp"] = {};
    root["inboundudp"]  = {};
}
/**
 * Given a FirewallRule struct, we attempt to insert the rule into the balanced red-black tree. Each insertion takes 
 * O(logn) time with an additional constant time to rebalance the tree. Ideally, this would be an interval tree as used in 
 * computational geometry. To save time and code, I have chosen to use a set, which in the std is implemented as a red-black tree
 * the balanced nature of the tree ensures O(logn) search and update time, with the added cost of rebalancing on insertion. 
 * 
 * @param rule FirewallRule: the rule to insert into the tree
 */
void RuleTree::insertRule(FirewallRule& rule)
{
    // std::cout << rule << '\n';
    
    //let's check to see if this interval already exists
    IPInterval ip(rule.ip_range, rule.port_range);
    std::string key = makekey(rule);

    auto rootIt = root.find(makekey(rule));
    auto node = rootIt->second.find(ip);
    if(node == rootIt->second.end())
    {
        // didn't find the rule yet, can insert the rule into the RuleTree
        root[key].insert(ip);
    }
    else
    {
        //found an instance of the interval that fit
        //lets's check if the rule is an exact match
        if(node->ip_range.start == ip.ip_range.start && node->ip_range.end == ip.ip_range.end)
        {
            //found an exact match, so we are adding a new port rule
            //let's check if the port rule is a new one using the same method as finding the ip ranges 
            std::set<Interval<uint16_t>>& portTree = node->portTree;
            Interval<uint16_t> portRange(rule.port_range);
            auto portIt = portTree.find(portRange);
            if(portIt == portTree.end())
            {
                //didn't find this range, let's insert
                portTree.insert(portRange);
            }
            else
            {
                //We hvae found a port range which overlaps, so we should expand given the current rule set and the new one
                Interval<uint16_t> newPortRange(std::min(portIt->start, rule.port_range.start), std::max(portIt->end, rule.port_range.end));
                
                //extend the current port range by erasing the previous node and then inserting the new one
                portTree.erase(portIt);
                portTree.insert(newPortRange);
            }
        }
        else
        {
           //reconstruct a new ip range
            Range<uint32_t> newIpRange(std::min(node->ip_range.start, ip.ip_range.start), std::max(node->ip_range.end, ip.ip_range.end));
            std::set<Interval<uint16_t>>& portTree = node->portTree;
            
            //lets find if the port range to be inserted overlaps with any that we have
            Interval<uint16_t> portRange(rule.port_range);
            auto portIt = portTree.find(portRange);
            if(portIt == portTree.end())
            {
                //we did not find any overlapping ranges, we can insert and then copy this tree over to the new interval
                portTree.insert(portRange);
            }
            else
            {
                Interval<uint16_t> newPortRange(std::min(portIt->start, rule.port_range.start), std::max(portIt->end, rule.port_range.end));
                
                //extend the current port range by erasing the previous node and then inserting the new one
                portTree.erase(portIt);
                portTree.insert(newPortRange);
            }

            //create the new interval
            IPInterval newInterval(newIpRange);
            //Copy over the port tree
            newInterval.portTree = portTree;

            //erase the old interval and insert the new one
            rootIt->second.erase(node);
            root[key].insert(newInterval);
        }
    }
}

/**
 * checks the tree to see if the specific ip_address/port has been contained in an interval, and if so, returns true
 * @param rule FirewallRule: a rule to search the tree for. None of the intervals are overlapping, and thus we can stop 
 * at the first found rule, avoiding std::find_if and its O(n) complexity.
 * @return bool: wether the ip address is contained inside a rule
 */
bool RuleTree::contains(FirewallRule& rule)
{
    std::string key = makekey(rule);
    auto rootIt = root.find(key);
    //check for port range
    IPInterval ip(rule.ip_range);
    auto ipRangeIt = rootIt->second.find(ip);
    if(ipRangeIt != rootIt->second.end())
    {
        // found a rule with a matching range
        return ipRangeIt->portTree.find(Interval<u_int16_t>(rule.port_range)) != ipRangeIt->portTree.end();
    }
    else
        return false;
    
}

/**
 * creates a key for the root of the tree based on the direction and protocol
 */
std::string RuleTree::makekey(FirewallRule& rule){ return rule.direction + rule.protocal; }