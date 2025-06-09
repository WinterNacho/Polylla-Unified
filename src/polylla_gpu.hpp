#ifndef POLYLLA_GPU_HPP
#define POLYLLA_GPU_HPP

#include <string>
#include <stdexcept>

#ifdef CUDA_AVAILABLE
// Forward declarations for the C wrapper functions from polylla.cu
extern "C" {
    void* polylla_gpu_create_default();
    void* polylla_gpu_create_int(int size);
    void* polylla_gpu_create_off(const char* off_file, bool use_regions);
    void* polylla_gpu_create_neigh(const char* node_file, const char* ele_file, const char* neigh_file, bool use_regions);
    void* polylla_gpu_create_ele(const char* node_file, const char* ele_file, bool use_regions);
    void polylla_gpu_destroy(void* instance);
    void polylla_gpu_print_stats(void* instance, const char* filename);
    void polylla_gpu_print_off(void* instance, const char* filename);
    void polylla_gpu_set_use_regions(void* instance, bool use_regions);
    bool polylla_gpu_get_use_regions(void* instance);
}
#endif

// GPU version class with inline implementation
class PolyllaGPU {
private:
    void* impl; // Will point to actual CUDA implementation

public:
    // Constructors
    PolyllaGPU() : impl(nullptr) {
#ifdef CUDA_AVAILABLE
        impl = polylla_gpu_create_default();
#else
        throw std::runtime_error("PolyllaGPU: This binary was compiled without CUDA support");
#endif
    }

    PolyllaGPU(int size) : impl(nullptr) {
#ifdef CUDA_AVAILABLE
        impl = polylla_gpu_create_int(size);
#else
        throw std::runtime_error("PolyllaGPU: This binary was compiled without CUDA support");
#endif
    }

    PolyllaGPU(std::string off_file, bool use_regions = false) : impl(nullptr) {
#ifdef CUDA_AVAILABLE
        impl = polylla_gpu_create_off(off_file.c_str(), use_regions);
#else
        throw std::runtime_error("PolyllaGPU: This binary was compiled without CUDA support");
#endif
    }

    PolyllaGPU(std::string node_file, std::string ele_file, std::string neigh_file, bool use_regions = false) : impl(nullptr) {
#ifdef CUDA_AVAILABLE
        impl = polylla_gpu_create_neigh(node_file.c_str(), ele_file.c_str(), neigh_file.c_str(), use_regions);
#else
        throw std::runtime_error("PolyllaGPU: This binary was compiled without CUDA support");
#endif
    }

    PolyllaGPU(std::string node_file, std::string ele_file, bool use_regions = false) : impl(nullptr) {
#ifdef CUDA_AVAILABLE
        impl = polylla_gpu_create_ele(node_file.c_str(), ele_file.c_str(), use_regions);
#else
        throw std::runtime_error("PolyllaGPU: This binary was compiled without CUDA support");
#endif
    }
    
    // Destructor
    ~PolyllaGPU() {
#ifdef CUDA_AVAILABLE
        if (impl) {
            polylla_gpu_destroy(impl);
        }
#endif
    }
    
    // Main methods
    void print_stats(std::string filename) {
#ifdef CUDA_AVAILABLE
        if (impl) {
            polylla_gpu_print_stats(impl, filename.c_str());
        }
#else
        throw std::runtime_error("PolyllaGPU: This binary was compiled without CUDA support");
#endif
    }

    void print_OFF(std::string filename) {
#ifdef CUDA_AVAILABLE
        if (impl) {
            polylla_gpu_print_off(impl, filename.c_str());
        }
#else
        throw std::runtime_error("PolyllaGPU: This binary was compiled without CUDA support");
#endif
    }
    
    // Region support methods
    void set_use_regions(bool use_regions) {
#ifdef CUDA_AVAILABLE
        if (impl) {
            polylla_gpu_set_use_regions(impl, use_regions);
        }
#else
        throw std::runtime_error("PolyllaGPU: This binary was compiled without CUDA support");
#endif
    }

    bool get_use_regions() const {
#ifdef CUDA_AVAILABLE
        if (impl) {
            return polylla_gpu_get_use_regions(impl);
        }
        return false;
#else
        throw std::runtime_error("PolyllaGPU: This binary was compiled without CUDA support");
#endif
    }
};

#endif // POLYLLA_GPU_HPP 