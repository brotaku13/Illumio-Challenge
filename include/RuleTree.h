#ifndef RULE_TREE
#define RULE_TREE

#include <set>
#include <unordered_map>
#include <cstdint>
#include <string>
#include <iostream>

// Represents a range
template <typename T>
struct Range 
{
    Range(){}
    Range(T start, T end): start(start), end(end){}
    T start;
    T end;
};

//Represents an interval which is the core component of the interval tree
template <typename T>
struct Interval
{
    Interval(T start, T end): start(start), end(end){}
    Interval(Range<T>& range): start(range.start), end(range.end){}
    T start;
    T end;

    bool operator<(const Interval<T>& t) const
    { 
        return this->end < t.start;
    } 
    bool operator>(const Interval<T>& t) const
    { 
        return this->start > t.end;
    }
    //checking for overlapping 
    bool operator==(const Interval<T>& t) const
    {
        return (this->start <= t.start && this->end >= t.end) || 
                (t.start <= this->start && t.end >= this->end);
    } 
};

//an IP interval is special since it contains an instance of a interval tree to hold the respective ports for this ip_range. 
struct IPInterval
{
    IPInterval(Range<uint32_t>& iprange) : ip_range(iprange){};
    IPInterval(Range<uint32_t>& iprange, Range<uint16_t>& portrange): ip_range(iprange)
    {
        portTree.insert(Interval<uint16_t>(portrange));
    }

    Interval<uint32_t> ip_range;

    //Forgot that set values are const once emplaced. To get around having to reconstruct the 
    //port tree every time I want to add a port range, I'm using this dirty mutable hack
    mutable std::set<Interval<uint16_t>> portTree; 

    bool operator<(const IPInterval& t) const
    { 
        return this->ip_range < t.ip_range;
    }
    bool operator>(const IPInterval& t) const
    { 
        return this->ip_range > t.ip_range;
    }
    bool operator==(const IPInterval& t) const
    {
        return this->ip_range == t.ip_range;
    } 
};

//represents a rule to be inserted or searched for in the tree
struct FirewallRule 
{
    std::string direction;
    std::string protocal;
    Range<uint32_t> ip_range;
    Range<uint16_t> port_range;

    friend std::ostream& operator<<(std::ostream& os, const FirewallRule& rule);
};

inline std::ostream& operator<<(std::ostream& os, const FirewallRule& rule)
{
    os << rule.direction << ' ' << rule.protocal << ' ';
    os << rule.ip_range.start << '-' << rule.ip_range.end << ' ' << rule.port_range.start << '-' << rule.port_range.end;

    return os;
}

/**
 * Represented as an Interval tree of ip ranges, each of which in turn has an interval tree of 
 * port ranges.
 */
class RuleTree 
{
public:
    RuleTree();
    void insertRule(FirewallRule& rule);
    bool contains(FirewallRule& rule);

private:
    std::string makekey(FirewallRule& rule);
    std::unordered_map<std::string, std::set<IPInterval>> root;
};

#endif