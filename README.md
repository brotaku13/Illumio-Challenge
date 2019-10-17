# Illumio Takehome Coding Challenge

First of all, I'd like to say that this was by far the most interesting coding challenge I've received this hiring season. As someone who is immensely interested in the work that Illumio does, I found it enlightening on both the nature of the work involved, as well as the type of company that Illumio is.

## Program Details
The program is written and compiled against C++ 11 on a linux machine. To compile, navigate to the project directory and use the `make` command.
```
~$ make
```

Which should create the bin directory and place the executable `program` inside. **You** can then run with 

```
~$ ./bin/program path/to/csv/file.csv
```

To run only the unit tests, open `main.cpp` and alter the main function to look like this
```c++
int main(int argc, char *argv[])
{
    // string filename = getFilename(argc, argv);
    // Firewall firewall(filename);
    runTests();
    exit(EXIT_SUCCESS);
}
```

## Initial Ideas and Approach
When I first read through the problem definition and began brainstorming possible data structures to store large quantities of IP addresses, I initially thought of using a prefix tree (trie). With the repetitive nature of string based IP addresses, at first this seemed like a good idea. 

But then I had a realization that the real critical part of this assignment wasn't how you dealt with individual rules for single IP addresses and ports, but rather, how I chose to deal with the ip and port ranges and the efficient storage and querying of such a data structure.

I needed to find a way to efficiently store ranges so that I could quickly search through intervals and find whether a given key was contained in an interval, preferably in better than $O(n)$ time. 

And so I entered the research rabbit hole and discovered Interval trees and Segment trees, specialized balanced Binary Search Trees designed to store intervals. Segment trees were recommended since they were designed to store ranges and query points within that range, but the structure required us to store each IP address as a leaf, leaving me no better than when I began. 

So I began looking at Interval Trees. Initially developed for computational geometry, they allowed for the efficient storage and querying of overlapping intervals. Implemented as an AVL tree, I could do efficient search and insertion with guarenteed $O(logn)$ time and $O(n)$ space. I settled on this data structure as the core component of my design. 

## Design Methodology
I began programming this assignment in Python. I saw that Illumio utilizes both Python and Java in their stack and wanted my code to be both relavent and telling of my skillset. However, while I know how the internals of an AVL tree work, and the basic concepts, I was hesitant to set out to code one. Surely there had to be a better way. 

This is when I came upon the key insight into my design. I remembered that the `std::set/std::multiset` in the C++ standard library were implemented as red-black trees; another balanced BST. I decided to utilize those extensively, along with custom data types to store as nodes. These data types were based around a simple object, an interval:

```c++
// Represents a range
template <typename T>
struct Interval
{
    Interval(T start, T end): start(start), end(end){}
    Interval(Range<T>& range): start(range.start), end(range.end){}
    T start;
    T end;
    ...
};
```

But there were technically two types of intervals to implement, an IP Interval and a Port Interval. 

My Basic design works by having an interval tree of ip addresses. Each interval of IP addresses is associated with an interval tree of port numbers. In this way, I could maintain my rules associated with each interval, and have a guarenteed search time of $2O(log(n))$.

In order to get this to work with the std::set, I simply needed to redefine the comparison operators for intervals. Technically, I only needed to redefine `operator<` since that is what is used in the comp function, but I also did `operator>` and `operator==` as well. The crucial part of this was to define Intervals as being equal as long as they overlapped. 

```c++
bool operator==(const Interval<T>& t) const
{
    return (this->start <= t.start && this->end >= t.end) || 
            (t.start <= this->start && t.end >= this->end);
} 
```

This tells the tree that when I want to look for an interval, I just need to find a node in which that interval is contained. Finally, I brought all this together with the idea that single address/port packets of the type `inbound tcp 60 198.168.0.1` were really just intervals with starting and ending points at the same number. 

Using these ideas, I could efficiently store and search for ip addresses and ports in the Interval tree by searching for the node which contains an overlapping interval. 


## Testing
I am well known among my friends for going crazy with tests at times. I once wrote an 800 line testing file which automatically generated and ran test cases for my systems design course. Additionally, I designed and wrote an entire testing framework for same course as a grader the following quarter. I really enjoy creating utilities that extensiely stress test programs such as this. 

Unfortunately, I was unable to realize some of the ideas I had. I was only able to implement some basic test cases which tested some key aspects of the program as well as some edge cases. 

If I had additional time, or if I was writing test cases for a production environment, I would have created a python script which automatically generated a large csv rules list (1-2 million rules) as well as the correct output and tested against that for both efficiency and correctness. 

As it stands, I did basic unit tests in `main.cpp`, testing some basic test cases and some edge cases. I think the most complex part of the code is the union of overlapping rules, and so I spent some time testing that as well. 

## Code Thoughts and Tradeoffs.

There are several key areas in my code that I see as lacking and which require additional attention. Unfortunately, I was trying to abide by the time as much as possible and was unable to implement some ideas that I had while coding which would make this program more robust. 

### CSVReader as a file Iterator
When I was designing the CSV file reader, I came to the realization that the file that I was reading in could be huge. So huge in fact that I would not have wanted to read the whole file into memory. 

I came up with the idea of making a CSV reader as an iterator and doing something akin to the C++ version of
```py
for line in file:
    yield line
```

However I had never implemented an iterator before and when I researched it I found that I was spending too much time on a part of the project that was not the main focus. Instead I chose to use a basic `getline` call, reading in each field into a vector to be processed. But this could be improved upon and be more maintainable as a custom iterator.

### Overlapping Rules and complexity
The project description did not make any mention of overlapping rules being provided. For example, 
```
outbound,tcp,10-20,0.0.0.0-0.0.0.255
outbound,tcp,12-28,0.0.0.5-0.0.0.100
```

These two rules overlap eachother, overwriting sections of the allowed ports. In practice, this could be really complex because each ip/address port range may have associated allowed IP addresses etc, so when we encounter a rule intersection, combining them would be difficult. 

Here, I essentially just took the union of the overlapping rules. One negative in my design is that it requires the port tree and ip tree to be copied and recreated, using up a lot of resources ($O(klogn)$ where k is the size of the tree) and compute power if the trees are very large. This is unavoidable due to the nature of the Red-Black tree, where the keys are immutable objects. 

### Construction complexity of an AVL tree. 
Right now, the rules are only added to the data structure once, and then never altered. This is done one rule at a time, resulting in a total construction complexity of $O(nlogn)$, since I need to insert $n$ rules and each rule takes $O(logn)$ insertion time. In theory, if I had all of the rules that would be used at the beginning, I could have just used a normal BST and constructed it bottom up in $O(n)$ time. 

Three problems exist with this.

1. This assumes that rules are not going to be added later and is thus not maintainable or scalable.
2. This would mean loading the whole CSV file into memory at construction time, which may not be the best option due to memory limitations. 
3. This requires the rules to be pre-sorted beforehand by IP address and port number, which is not how data usually arrives. If it was not sorted, it would be the same complexity of $O(nlogn)$ 


## Team Placement
I happen to be immensely interested in the **Platform** team at Illumio, and would very much like to hear about any opportunities for placement on that team, especially regarding the cache layer optimization support and metrics subsystem.  

## Conclusion
Overall, I probably had too much fun with this project. I enjoyed the assignment because it was one of those classical CS problems which looks quite simple at first but becomes more and more complex the deeper you dive. I hope you enjoy my implementation and creativity as much as I enjoyed writing it. 