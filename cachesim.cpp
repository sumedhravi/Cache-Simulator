#include "cachesim.hpp"

#include <iostream>
#include <unordered_map>
#include <list>
#include <vector>
#include <utility>
#include <math.h>

// Structure to store tag data
struct address_block
{
    uint64_t tag;
    uint64_t index;
    bool isDirty;

    address_block(uint64_t tag, uint64_t index, bool dirty=false)
    {
        this->tag = tag;
        this->index = index;
        this->isDirty = dirty;
    }
};

// Structure to represent a cache block
struct cacheblock
{
    std::list<address_block> tags;
    std::unordered_map<uint64_t, std::list<address_block>::iterator> hashmap;
};

class associative_cache
{
    private:
        std::vector<cacheblock> blocks;
    public:
        int setSize;
        uint64_t numberOfSets;
        uint64_t cache_size;
        insert_policy_t insertion_policy;
        write_strat_t write_strategy;
        bool fully_associative; 
        associative_cache(const cache_config_t& config, bool isFullyAssociative);
        associative_cache(uint64_t cache_size, uint64_t block_size, bool isFullyAssociative);
        int access(uint64_t element, char rw);
        std::pair<bool, address_block> insert(uint64_t element, bool dirty);
        void replace(uint64_t old_element, uint64_t new_element, bool dirty);
        uint64_t generate_tag(uint64_t address);
        uint64_t generate_index(uint64_t address);
        uint64_t generate_address(uint64_t tag, uint64_t index);
};

associative_cache::associative_cache(const cache_config_t& config, bool isFullyAssociative = false)
{
    setSize = pow(2, config.s);
    numberOfSets = pow(2, (config.c-config.b-config.s));
    cache_size = pow(2, config.c);
    fully_associative = isFullyAssociative;
    blocks.resize(numberOfSets);
    write_strategy = config.write_strat;
    insertion_policy = config.insert_policy;
}

associative_cache::associative_cache(uint64_t cache_size, uint64_t block_size, bool isFullyAssociative = true)
{
    setSize = cache_size/block_size;
    numberOfSets = 1;
    this->cache_size = cache_size;
    fully_associative = isFullyAssociative;
    blocks.resize(numberOfSets);
    write_strategy = WRITE_STRAT_WTWNA;
    insertion_policy = INSERT_POLICY_MIP;
}

// Get Tag value from address
uint64_t associative_cache:: generate_tag(uint64_t address)
{
    uint64_t tag;
    if (!fully_associative)
        tag = address >> (int(log2(cache_size)) - int(log2(setSize))); // shift by (c-s) bits to get tag
    else
        tag = address >> int((cache_size/setSize)); // tag = shift by b bits to get tag 
    return tag;
}

// Get index value from address
uint64_t associative_cache:: generate_index(uint64_t address)
{
    uint64_t index;
    if (!fully_associative)
        {
            uint64_t tag = address >> (int(log2(cache_size)) - int(log2(setSize))); // shift by (c-s) bits to get tag
            tag = tag << (int(log2(cache_size)) - int(log2(setSize)));
            index = address - tag;
            index = index >> ((int(log2(cache_size)) - int(log2(setSize)) - int(log2(numberOfSets)))); // b
        }
    else
        index = 0; // Only 1 set
    return index;
}

// Get address from tag and index
uint64_t associative_cache::generate_address(uint64_t tag, uint64_t index)
{
    uint64_t address;
    if (!fully_associative)
    {
        uint64_t tag_index = tag << int(log2(numberOfSets)); //Left shift by number of index bits
        tag_index = tag_index | index; // Bitwise OR index bits.	
        address = tag_index << ((int(log2(cache_size)) - int(log2(setSize))) - (int(log2(numberOfSets)))); // b
    }
    else 
    {
        address = tag << (cache_size/setSize);
    }
    return address;
}

// Access an address block in the cache
int associative_cache::access(uint64_t element, char rw)
{
    uint64_t tag = generate_tag(element);
    int index = generate_index(element);
    auto block = &blocks[index];
    if(block->hashmap.find(tag)==block->hashmap.end())
        return -1;
    block->tags.splice(block->tags.begin(),block->tags,block->hashmap[tag]);
    (*block->hashmap[tag]).isDirty = (*block->hashmap[tag]).isDirty or (rw == WRITE);
    return (*block->hashmap[tag]).isDirty;
}

// Add a block to the cache
std::pair<bool, address_block> associative_cache::insert(uint64_t element, bool dirty)
{
    uint64_t tag = generate_tag(element);
    int index = generate_index(element);
    auto block = &blocks[index];
    address_block victim_block = address_block(0,index,false);
    if (block->hashmap.find(tag) != block->hashmap.end())
    {
        block->tags.splice(block->tags.begin(), block->tags, block->hashmap[tag]);
        block->hashmap[tag] = block->tags.begin();
        (*block->hashmap[tag]).isDirty = dirty;
        return std::make_pair(false, victim_block);
    }

    bool didEvict = false;
    if (block->hashmap.size() == setSize)
    {
        auto evict = &block->tags.back();
        block->tags.pop_back();
        block->hashmap.erase(evict->tag);
        victim_block = address_block(evict->tag, index, evict->isDirty);
        didEvict = true;
    }
    if (insertion_policy == INSERT_POLICY_MIP)
    {
        block->tags.push_front(address_block(tag, index, dirty));
        block->hashmap[tag] = block->tags.begin();
    }
    else 
    {
        block->tags.push_back(address_block(tag, index, dirty));
        block->hashmap[tag] = (--block->tags.end());
    }
    return std::make_pair(didEvict, victim_block);
}

// Swap elements in the cache
void associative_cache::replace(uint64_t old_element, uint64_t new_element, bool dirty)
{
    int index = 0;
    uint64_t tag_old = generate_tag(old_element);
    uint64_t tag_new = generate_tag(new_element);
    auto block = &blocks[index];
    if (block->hashmap.find(tag_old) != block->hashmap.end())
    {
        block->tags.erase(block->hashmap[tag_old]);
        block->hashmap.erase(tag_old);
        block->tags.push_front(address_block(tag_new, index, dirty));
        block->hashmap[tag_new] = block->tags.begin();
    }
}

/**
 * Subroutine for initializing the cache simulator. You many add and initialize any global or heap
 * variables as needed.
 * TODO: You're responsible for completing this routine
 */
associative_cache* L1;
associative_cache* victim;
associative_cache* L2;
bool victimEnabled = false;
bool L2_enabled = false;

void sim_setup(sim_config_t *config) {
    L1 = new associative_cache(config->l1_config);
    L2_enabled = !config->l2_config.disabled;
    if (L2_enabled)
        L2 = new associative_cache(config->l2_config);
    victimEnabled = config->victim_cache_entries > 0;
    if (victimEnabled)
        victim = new associative_cache(config->victim_cache_entries*config->l1_config.b, config->l1_config.b, true);
}

/**
 * Subroutine that simulates the cache one trace event at a time.
 * TODO: You're responsible for completing this routine
 */

void sim_access(char rw, uint64_t addr, sim_stats_t* stats) {
    if (rw == READ)
        stats->reads += 1;
    else if (rw == WRITE)
        stats->writes += 1;

    stats->accesses_l1 += 1;
    if (L1->access(addr, rw) != -1)
    {
        // If L1 cache has the block
        stats->hits_l1 += 1;
        return;
    }
    stats->misses_l1 += 1;

    // Search in victim
    if (victimEnabled)
    {
        int victim_access = victim->access(addr, READ);
        if (victim_access != -1)
        {
            stats->hits_victim_cache += 1;
            // Swap victim hit with L1 based on MIP.
            auto new_victim = L1->insert(addr, rw == WRITE or victim_access);
            if (new_victim.first)
                victim->replace(addr, L1->generate_address(new_victim.second.tag, new_victim.second.index), new_victim.second.isDirty);
            return;
        }
    }
    
    stats->misses_victim_cache += 1;
    stats->reads_l2 += 1;

    // Access L2 cache if enabled 
    if (L2_enabled)
    {
        // Access mode is READ if L1 is writealloc or rw = READ
        if (L2->access(addr, READ) != -1)
            stats->read_hits_l2 += 1;
        else
        {
            stats->read_misses_l2 += 1;
            // Insert into L2 cacbe
            auto L2evict = L2->insert(addr, false);
        }
    }
    else
        stats->read_misses_l2 += 1;

    // Insert into L1 cache
    auto evict = L1->insert(addr, (rw == WRITE));

    if (evict.first)
    {
        if (victimEnabled)
        {
            evict = victim->insert(L1->generate_address(evict.second.tag, evict.second.index), evict.second.isDirty);
        }
    }
    if (evict.first)
    {
        if (evict.second.isDirty)
        // Write back
        {
            stats->write_backs_l1_or_victim_cache += 1;
            stats->writes_l2 += 1;
        }

        if (L2_enabled)
        {
            uint64_t address;
            if (victimEnabled)
                address = victim->generate_address(evict.second.tag, evict.second.index);
            else
                address = L1->generate_address(evict.second.tag, evict.second.index);
            if (evict.second.isDirty)
                auto access = L2->access(address, READ);
        }
    }
}

/**
 * Subroutine for cleaning up any outstanding memory operations and calculating overall statistics
 * such as miss rate or average access time.
 * TODO: You're responsible for completing this routine
 */
void sim_finish(sim_stats_t *stats) {
    // Calculating statistics
    stats->hit_ratio_l1 = double(stats->hits_l1)/stats->accesses_l1;
    stats->miss_ratio_l1 = double(stats->misses_l1)/stats->accesses_l1;
    stats->hit_ratio_victim_cache = double(stats->hits_victim_cache)/stats->misses_l1;
    stats->miss_ratio_victim_cache = double(stats->misses_victim_cache)/stats->misses_l1;
    stats->read_hit_ratio_l2 = double(stats->read_hits_l2)/stats->reads_l2;
    stats->read_miss_ratio_l2 = double(stats->read_misses_l2)/stats->reads_l2;
    if (L2_enabled)
        stats->avg_access_time_l2 = L2_HIT_TIME_CONST + log2(L2->setSize)*L2_HIT_TIME_PER_S + stats->read_miss_ratio_l2*DRAM_ACCESS_PENALTY;
    else
        stats->avg_access_time_l2 = stats->read_miss_ratio_l2*DRAM_ACCESS_PENALTY;
    stats->avg_access_time_l1 = (L1_HIT_TIME_CONST + log2(L1->setSize)*L1_HIT_TIME_PER_S) + stats->miss_ratio_l1*stats->miss_ratio_victim_cache*stats->avg_access_time_l2;
    // Free memory allocated
    delete L1;
    delete victim;
    delete L2;
}
