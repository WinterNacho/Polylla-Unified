#ifndef GPOLYLLA_WRAPPER_HPP
#define GPOLYLLA_WRAPPER_HPP

#include <string>
#include <stdexcept>

#ifdef CUDA_AVAILABLE
// Forward declarations for the C wrapper functions from polylla.cu
extern "C" {
    void* gpolylla_create_default();
    void* gpolylla_create_int(int size);
    void* gpolylla_create_off(const char* off_file, bool use_regions);
    void* gpolylla_create_neigh(const char* node_file, const char* ele_file, const char* neigh_file, bool use_regions);
    void gpolylla_destroy(void* instance);
    void gpolylla_print_stats(void* instance, const char* filename);
    void gpolylla_print_off(void* instance, const char* filename);
}
#endif

// GPU version class wrapper
class GPolylla {
private:
    void* impl; // Will point to actual CUDA implementation

public:
    // Constructors
    GPolylla() : impl(nullptr) {
#ifdef CUDA_AVAILABLE
        impl = gpolylla_create_default();
#else
        throw std::runtime_error("GPolylla: This binary was compiled without CUDA support");
#endif
    }

    GPolylla(int size) : impl(nullptr) {
#ifdef CUDA_AVAILABLE
        impl = gpolylla_create_int(size);
#else
        throw std::runtime_error("GPolylla: This binary was compiled without CUDA support");
#endif
    }

    GPolylla(std::string off_file, bool use_regions = false) : impl(nullptr) {
#ifdef CUDA_AVAILABLE
        impl = gpolylla_create_off(off_file.c_str(), use_regions);
#else
        throw std::runtime_error("GPolylla: This binary was compiled without CUDA support");
#endif
    }

    GPolylla(std::string node_file, std::string ele_file, std::string neigh_file, bool use_regions = false) : impl(nullptr) {
#ifdef CUDA_AVAILABLE
        impl = gpolylla_create_neigh(node_file.c_str(), ele_file.c_str(), neigh_file.c_str(), use_regions);
#else
        throw std::runtime_error("GPolylla: This binary was compiled without CUDA support");
#endif
    }
    
    // Destructor
    ~GPolylla() {
#ifdef CUDA_AVAILABLE
        if (impl) {
            gpolylla_destroy(impl);
        }
#endif
    }
    
    // Main methods
    void print_stats(std::string filename) {
#ifdef CUDA_AVAILABLE
        if (impl) {
            gpolylla_print_stats(impl, filename.c_str());
        }
#else
        throw std::runtime_error("GPolylla: This binary was compiled without CUDA support");
#endif
    }

    void print_OFF(std::string filename) {
#ifdef CUDA_AVAILABLE
        if (impl) {
            gpolylla_print_off(impl, filename.c_str());
        }
#else
        throw std::runtime_error("GPolylla: This binary was compiled without CUDA support");
#endif
    }
};

#endif // GPOLYLLA_WRAPPER_HPP 