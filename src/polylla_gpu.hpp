#ifndef POLYLLA_GPU_HPP
#define POLYLLA_GPU_HPP

#include <string>

// Forward declaration of GPU version class
// The actual implementation is in polylla.cu
class PolyllaGPU {
public:
    // Constructors
    PolyllaGPU();
    PolyllaGPU(int size);
    PolyllaGPU(std::string off_file, bool use_regions = false);
    PolyllaGPU(std::string node_file, std::string ele_file, std::string neigh_file, bool use_regions = false);
    PolyllaGPU(std::string node_file, std::string ele_file, bool use_regions = false);
    
    // Destructor
    ~PolyllaGPU();
    
    // Main methods
    void print_stats(std::string filename);
    void print_OFF(std::string filename);
    
    // Region support methods
    void set_use_regions(bool use_regions);
    bool get_use_regions() const;
};

#endif // POLYLLA_GPU_HPP 