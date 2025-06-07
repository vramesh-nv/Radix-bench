#include <iostream>
#include <chrono>
#include <vector>
#include <set>
#include <map>
#include <random>
#include <algorithm>
#include <iomanip>

extern "C" {
#include "radix.h"
#include "art.h"
}

class Timer {
public:
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    double stop() {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end_time - start_time).count();
    }
    
private:
    std::chrono::high_resolution_clock::time_point start_time;
};

void benchmark_radix_tree(const std::vector<NvU64>& keys, const std::vector<NvU64>& search_keys) {
    Timer timer;
    CUradixTree tree;
    std::vector<CUradixNode> nodes(keys.size());
    
    // Initialize tree
    radixTreeInit(&tree, 64);
    
    // Benchmark insertion
    timer.start();
    for (size_t i = 0; i < keys.size(); ++i) {
        radixTreeInsert(&tree, &nodes[i], keys[i]);
    }
    double insert_time = timer.stop();
    
    // Benchmark lookup
    timer.start();
    size_t found_count = 0;
    for (const auto& key : search_keys) {
        CUradixNode* found = radixTreeFindGEQ(&tree, key);
        if (found && found->key == key) {
            found_count++;
        }
    }
    double lookup_time = timer.stop();
    
    double insert_time_per_op = (insert_time * 1000.0) / keys.size();  // Convert to microseconds per operation
    double lookup_time_per_op = (lookup_time * 1000.0) / search_keys.size();  // Convert to microseconds per operation
    
    std::cout << "Radix Tree Results:\n";
    std::cout << "  Insertion: " << std::fixed << std::setprecision(3) << insert_time_per_op << " us/op\n";
    std::cout << "  Lookup:    " << std::fixed << std::setprecision(3) << lookup_time_per_op << " us/op\n";
    std::cout << "  Found:     " << found_count << "/" << search_keys.size() << " keys\n\n";
}

void benchmark_std_set(const std::vector<NvU64>& keys, const std::vector<NvU64>& search_keys) {
    Timer timer;
    std::set<NvU64> tree;
    
    // Benchmark insertion
    timer.start();
    for (const auto& key : keys) {
        tree.insert(key);
    }
    double insert_time = timer.stop();
    
    // Benchmark lookup
    timer.start();
    size_t found_count = 0;
    for (const auto& key : search_keys) {
        if (tree.find(key) != tree.end()) {
            found_count++;
        }
    }
    double lookup_time = timer.stop();
    
    double insert_time_per_op = (insert_time * 1000.0) / keys.size();  // Convert to microseconds per operation
    double lookup_time_per_op = (lookup_time * 1000.0) / search_keys.size();  // Convert to microseconds per operation
    
    std::cout << "std::set Results:\n";
    std::cout << "  Insertion: " << std::fixed << std::setprecision(3) << insert_time_per_op << " us/op\n";
    std::cout << "  Lookup:    " << std::fixed << std::setprecision(3) << lookup_time_per_op << " us/op\n";
    std::cout << "  Found:     " << found_count << "/" << search_keys.size() << " keys\n\n";
}

void benchmark_std_multiset(const std::vector<NvU64>& keys, const std::vector<NvU64>& search_keys) {
    Timer timer;
    std::multiset<NvU64> multi_set;
    
    // Benchmark insertion
    timer.start();
    for (const auto& key : keys) {
        multi_set.insert(key);
    }
    double insert_time = timer.stop();
    
    // Benchmark lookup
    timer.start();
    size_t found_count = 0;
    for (const auto& key : search_keys) {
        if (multi_set.find(key) != multi_set.end()) {
            found_count++;
        }
    }
    double lookup_time = timer.stop();
    
    double insert_time_per_op = (insert_time * 1000.0) / keys.size();  // Convert to microseconds per operation
    double lookup_time_per_op = (lookup_time * 1000.0) / search_keys.size();  // Convert to microseconds per operation
    
    std::cout << "std::multiset Results:\n";
    std::cout << "  Insertion: " << std::fixed << std::setprecision(3) << insert_time_per_op << " us/op\n";
    std::cout << "  Lookup:    " << std::fixed << std::setprecision(3) << lookup_time_per_op << " us/op\n";
    std::cout << "  Found:     " << found_count << "/" << search_keys.size() << " keys\n\n";
}

void benchmark_libart(const std::vector<NvU64>& keys, const std::vector<NvU64>& search_keys) {
    Timer timer;
    art_tree tree;
    art_tree_init(&tree);
    
    // Benchmark insertion
    timer.start();
    for (const auto& key : keys) {
        // Convert key to byte array for libart
        unsigned char key_bytes[8];
        for (int i = 0; i < 8; i++) {
            key_bytes[7-i] = (key >> (i * 8)) & 0xFF;  // Big-endian for lexicographic order
        }
        art_insert(&tree, key_bytes, 8, (void*)1);  // Use dummy value
    }
    double insert_time = timer.stop();
    
    // Benchmark lookup
    timer.start();
    size_t found_count = 0;
    for (const auto& key : search_keys) {
        unsigned char key_bytes[8];
        for (int i = 0; i < 8; i++) {
            key_bytes[7-i] = (key >> (i * 8)) & 0xFF;  // Big-endian for lexicographic order
        }
        if (art_search(&tree, key_bytes, 8) != nullptr) {
            found_count++;
        }
    }
    double lookup_time = timer.stop();
    
    double insert_time_per_op = (insert_time * 1000.0) / keys.size();  // Convert to microseconds per operation
    double lookup_time_per_op = (lookup_time * 1000.0) / search_keys.size();  // Convert to microseconds per operation
    
    std::cout << "libart (ART) Results:\n";
    std::cout << "  Insertion: " << std::fixed << std::setprecision(3) << insert_time_per_op << " us/op\n";
    std::cout << "  Lookup:    " << std::fixed << std::setprecision(3) << lookup_time_per_op << " us/op\n";
    std::cout << "  Found:     " << found_count << "/" << search_keys.size() << " keys\n\n";
    
    art_tree_destroy(&tree);
}

void benchmark_range_queries(const std::vector<NvU64>& keys) {
    Timer timer;
    
    // Setup data structures
    CUradixTree radix_tree;
    std::vector<CUradixNode> nodes(keys.size());
    radixTreeInit(&radix_tree, 64);
    
    std::set<NvU64> std_set;
    
    for (size_t i = 0; i < keys.size(); ++i) {
        radixTreeInsert(&radix_tree, &nodes[i], keys[i]);
        std_set.insert(keys[i]);
    }
    
    // Test range queries (find smallest element >= query)
    std::vector<NvU64> query_keys;
    for (size_t i = 0; i < 10000; ++i) {
        query_keys.push_back(keys[i % keys.size()] + (i % 100));
    }
    
    // Radix tree range queries
    timer.start();
    size_t radix_found = 0;
    for (const auto& query : query_keys) {
        CUradixNode* found = radixTreeFindGEQ(&radix_tree, query);
        if (found) radix_found++;
    }
    double radix_range_time = timer.stop();
    
    // std::set range queries
    timer.start();
    size_t set_found = 0;
    for (const auto& query : query_keys) {
        auto it = std_set.lower_bound(query);
        if (it != std_set.end()) set_found++;
    }
    double set_range_time = timer.stop();
    
    double radix_range_time_per_op = (radix_range_time * 1000.0) / query_keys.size();  // Convert to microseconds per operation
    double set_range_time_per_op = (set_range_time * 1000.0) / query_keys.size();  // Convert to microseconds per operation
    
    std::cout << "Range Query Results (lower_bound/GEQ):\n";
    std::cout << "  Radix Tree: " << std::fixed << std::setprecision(3) << radix_range_time_per_op << " us/op (" << radix_found << " found)\n";
    std::cout << "  std::set:   " << std::fixed << std::setprecision(3) << set_range_time_per_op << " us/op (" << set_found << " found)\n\n";
}

int main() {
    const size_t num_keys = 100000;
    const size_t num_searches = 50000;
    
    std::cout << "Radix Tree vs C++ Standard Library Benchmark\n";
    std::cout << "=============================================\n";
    std::cout << "Dataset size: " << num_keys << " keys\n";
    std::cout << "Search operations: " << num_searches << " lookups\n\n";
    
    // Generate random keys
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<NvU64> dis(1, 1000000);
    
    std::vector<NvU64> keys;
    keys.reserve(num_keys);
    for (size_t i = 0; i < num_keys; ++i) {
        keys.push_back(dis(gen));
    }
    
    // Sort keys but keep duplicates since we're testing duplicate handling
    std::sort(keys.begin(), keys.end());
    
    // Generate search keys (mix of existing and non-existing)
    std::vector<NvU64> search_keys;
    search_keys.reserve(num_searches);
    
    // 70% existing keys, 30% random keys
    for (size_t i = 0; i < num_searches * 0.7; ++i) {
        search_keys.push_back(keys[dis(gen) % keys.size()]);
    }
    for (size_t i = 0; i < num_searches * 0.3; ++i) {
        search_keys.push_back(dis(gen));
    }
    
    std::cout << "Dataset size: " << keys.size() << " keys (including duplicates)\n\n";
    
    // Run benchmarks
    benchmark_radix_tree(keys, search_keys);
    benchmark_std_set(keys, search_keys);
    benchmark_std_multiset(keys, search_keys);
    benchmark_libart(keys, search_keys);
    benchmark_range_queries(keys);
    
    return 0;
} 