#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <polylla.hpp>
#include <triangulation.hpp>

// Conditional CUDA includes
#ifdef CUDA_AVAILABLE
#include <GPolylla/gpolylla_wrapper.hpp>
#endif

struct ProgramOptions {
    enum InputType { NONE, OFF, NEIGH, ELE, POLY };
    enum OutputFormat { OFF_FORMAT };
    
    InputType input_type = NONE;
    OutputFormat output_format = OFF_FORMAT;  // Default format
    std::string node_file;
    std::string ele_file;
    std::string neigh_file;
    std::string off_file;
    std::string poly_file;
    std::string triangle_args = "pnz";  // Default triangle arguments
    std::string output_name;
    
    // Polylla options
    PolyllaOptions polylla_options;
    
    // Additional flags
    bool help = false;
    bool use_gpu = false;
};

bool is_positive_num(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!isdigit(c)) return false;
    }
    return true;
}

// Helper function to validate file existence
bool file_exists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

bool validate_polylla_options(const PolyllaOptions& options) {
    // Business logic validations (not covered by parse_arguments)
    
    // Validate target length for distmesh method
    if (options.smooth_method == "distmesh" && options.target_length == 0) {
        std::cerr << "Error: Target length cannot be zero for distmesh method" << std::endl;
        return false;
    }
    
    // Could add more business logic validations here in the future
    // e.g., combination of options that don't make sense together
    
    return true;
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS] [FILES...]\n\n";
    std::cout << "Input modes:\n";
    std::cout << "  -o, --off            Use OFF file as input\n";
    std::cout << "  -n, --neigh          Use .node, .ele, and .neigh files as input\n";
    std::cout << "  -e, --ele            Use .node and .ele files as input (without .neigh)\n";
    std::cout << "  -p, --poly           Use .poly file as input (requires Triangle)\n";
    std::cout << "  -p:ARGS              Use .poly file with custom Triangle arguments\n\n";
    
    std::cout << "File specification:\n";
    std::cout << "  You can specify files in two ways:\n";
    std::cout << "  1. Individual files: program -n file.node file.ele file.neigh\n";
    std::cout << "  2. Base name: program -n basename (automatically uses basename.node, basename.ele, basename.neigh)\n\n";
    
    std::cout << "Triangle integration (.poly files):\n";
    std::cout << "  Basic usage:\n";
    std::cout << "    " << program_name << " -p input.poly                    # Uses 'triangle -pnz'\n";
    std::cout << "    " << program_name << " -p input.poly --region           # Uses 'triangle -pnzAa'\n";
    std::cout << "  Custom Triangle arguments:\n";
    std::cout << "    " << program_name << " -p:pqnz input.poly               # Uses 'triangle -pqnz'\n";
    std::cout << "    " << program_name << " -p:pq30a0.1nzAa input.poly       # Quality 30°, max area 0.1\n";
    std::cout << "    " << program_name << " -p:pq25nzAa input.poly --region  # Quality 25° with regions\n\n";
    
    std::cout << "Options:\n";
    std::cout << "  -g, --gpu            Enable GPU acceleration (requires CUDA)\n";
    std::cout << "  -r, --region         Read and process triangulation considering regions\n";
    std::cout << "  -s, --smooth METHOD  Use smoothing method: laplacian, laplacian-edge-ratio, distmesh\n";
    std::cout << "  -i, --iterations N   Number of smoothing iterations (default: 50)\n";
    std::cout << "  -t, --target-length N Target edge length for distmesh method\n";
    std::cout << "  -O, --output FORMAT  Specify output format: off (default)\n";
    std::cout << "  -h, --help           Show this help message\n\n";
    
    // Show CUDA availability status
#ifdef CUDA_AVAILABLE
    std::cout << "CUDA support: Available\n";
#else
    std::cout << "CUDA support: Not available (compiled without CUDA)\n";
#endif
}

bool parse_arguments(int argc, char** argv, ProgramOptions& options) {
    // First pass: check for -p:args format before using getopt
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.substr(0, 3) == "-p:") {
            // Found -p:args format
            if (options.input_type != ProgramOptions::NONE) {
                std::cerr << "Error: Multiple input types specified\n";
                return false;
            }
            options.input_type = ProgramOptions::POLY;
            options.triangle_args = arg.substr(3);  // Extract arguments after -p:
            
            // Remove this argument from argv for getopt processing
            for (int j = i; j < argc - 1; j++) {
                argv[j] = argv[j + 1];
            }
            argc--;
            i--; // Adjust index since we removed an element
        }
    }
    
    static struct option long_options[] = {
        {"off",           no_argument,       0, 'o'},
        {"neigh",         no_argument,       0, 'n'},
        {"ele",           no_argument,       0, 'e'},
        {"poly",          no_argument,       0, 'p'},
        {"gpu",           no_argument,       0, 'g'},
        {"region",        no_argument,       0, 'r'},
        {"smooth",        required_argument, 0, 's'},
        {"iterations",    required_argument, 0, 'i'},
        {"target-length", required_argument, 0, 't'},
        {"output",        required_argument, 0, 'O'},
        {"help",          no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    while ((c = getopt_long(argc, argv, "onegprs:i:t:O:h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'o':
                if (options.input_type != ProgramOptions::NONE) {
                    std::cerr << "Error: Multiple input types specified\n";
                    return false;
                }
                options.input_type = ProgramOptions::OFF;
                break;
                
            case 'n':
                if (options.input_type != ProgramOptions::NONE) {
                    std::cerr << "Error: Multiple input types specified\n";
                    return false;
                }
                options.input_type = ProgramOptions::NEIGH;
                break;
                
            case 'e':
                if (options.input_type != ProgramOptions::NONE) {
                    std::cerr << "Error: Multiple input types specified\n";
                    return false;
                }
                options.input_type = ProgramOptions::ELE;
                break;
                
            case 'p':
                if (options.input_type != ProgramOptions::NONE) {
                    std::cerr << "Error: Multiple input types specified\n";
                    return false;
                }
                options.input_type = ProgramOptions::POLY;
                break;
                
            case 'g':
                options.use_gpu = true;
                break;
                
            case 'r':
                options.polylla_options.use_regions = true;
                break;
                
            case 's':
                {
                    std::string method = optarg;
                    std::vector<std::string> valid_methods = {"laplacian", "laplacian-edge-ratio", "distmesh"};
                    if (std::find(valid_methods.begin(), valid_methods.end(), method) != valid_methods.end()) {
                        options.polylla_options.smooth_method = method;
                    } else {
                        std::cerr << "Error: Invalid smoothing method '" << method << "'\n";
                        std::cerr << "Valid methods: laplacian, laplacian-edge-ratio, distmesh\n";
                        return false;
                    }
                }
                break;
                
            case 'i':
                {
                    std::string iter_str = optarg;
                    if (is_positive_num(iter_str)) {
                        options.polylla_options.smooth_iterations = std::stoi(iter_str);
                    } else {
                        std::cerr << "Error: Invalid value '" << iter_str << "' for iterations. Must be a positive number.\n";
                        return false;
                    }
                }
                break;
                
            case 't':
                {
                    std::string length_str = optarg;
                    if (is_positive_num(length_str) || std::stoi(length_str) == 0) {
                        options.polylla_options.target_length = std::stod(length_str);
                    } else {
                        std::cerr << "Error: Invalid value '" << length_str << "' for target-length. Must be a positive number.\n";
                        return false;
                    }
                }
                break;
                
            case 'O':
                {
                    std::string format = optarg;
                    if (format == "off") {
                        options.output_format = ProgramOptions::OFF_FORMAT;
                    } else {
                        std::cerr << "Error: Unknown output format '" << format << "'. Supported: off" << std::endl;
                        return false;
                    }
                }
                break;
                
            case 'h':
                options.help = true;
                return true;
                
            case '?':
                // getopt_long already printed an error message
                return false;
                
            default:
                return false;
        }
    }
    
    // Process remaining arguments (files)
    std::vector<std::string> remaining_args;
    for (int i = optind; i < argc; i++) {
        remaining_args.push_back(argv[i]);
    }
    
    // If no input type specified, we need one
    if (options.input_type == ProgramOptions::NONE) {
        std::cerr << "Error: No input type specified. Use --help for usage information.\n";
        return false;
    }
    
    // Process input files based on type
    if (options.input_type == ProgramOptions::OFF) {
        // Look for .off file first (existing behavior)
        auto off_file = std::find_if(remaining_args.begin(), remaining_args.end(), 
            [](const std::string& file) { 
                return file.substr(file.find_last_of(".") + 1) == "off"; 
            });
        
        if (off_file != remaining_args.end()) {
            // Found explicit .off file
            options.off_file = *off_file;
        } else if (remaining_args.size() == 1) {
            // Single argument - treat as base name
            std::string base = remaining_args[0];
            // Only remove known extensions (.off) to get clean base name
            if (base.length() > 4 && base.substr(base.length() - 4) == ".off") {
                base = base.substr(0, base.length() - 4);
            }
            options.off_file = base + ".off";
        } else {
            std::cerr << "Error: No .off file found in arguments\n";
            return false;
        }
        
        // Auto-generate output name if not provided
        if (options.output_name.empty()) {
            options.output_name = options.off_file.substr(0, options.off_file.find_last_of("."));
        }
        
    } else if (options.input_type == ProgramOptions::NEIGH) {
        // Look for explicit files first (existing behavior)
        auto node_file = std::find_if(remaining_args.begin(), remaining_args.end(), 
            [](const std::string& file) { 
                return file.substr(file.find_last_of(".") + 1) == "node"; 
            });
        auto ele_file = std::find_if(remaining_args.begin(), remaining_args.end(), 
            [](const std::string& file) { 
                return file.substr(file.find_last_of(".") + 1) == "ele"; 
            });
        auto neigh_file = std::find_if(remaining_args.begin(), remaining_args.end(), 
            [](const std::string& file) { 
                return file.substr(file.find_last_of(".") + 1) == "neigh"; 
            });
            
        if (node_file != remaining_args.end() && ele_file != remaining_args.end() && neigh_file != remaining_args.end()) {
            // Found all explicit files
            options.node_file = *node_file;
            options.ele_file = *ele_file;
            options.neigh_file = *neigh_file;
        } else if (remaining_args.size() == 1) {
            // Single argument - treat as base name
            std::string base = remaining_args[0];
            // Only remove known extensions (.node, .ele, .neigh) to get clean base name
            if (base.length() > 5 && base.substr(base.length() - 5) == ".node") {
                base = base.substr(0, base.length() - 5);
            } else if (base.length() > 4 && base.substr(base.length() - 4) == ".ele") {
                base = base.substr(0, base.length() - 4);
            } else if (base.length() > 6 && base.substr(base.length() - 6) == ".neigh") {
                base = base.substr(0, base.length() - 6);
            }
            options.node_file = base + ".node";
            options.ele_file = base + ".ele";
            options.neigh_file = base + ".neigh";
        } else {
            std::cerr << "Error: Missing required files (.node, .ele, .neigh) or provide single base name\n";
            return false;
        }
        
        // Auto-generate output name if not provided
        if (options.output_name.empty()) {
            std::string base = options.node_file.substr(0, options.node_file.find_last_of("."));
            options.output_name = base;
        }
        
    } else if (options.input_type == ProgramOptions::ELE) {
        // Look for explicit files first (existing behavior)
        auto node_file = std::find_if(remaining_args.begin(), remaining_args.end(), 
            [](const std::string& file) { 
                return file.substr(file.find_last_of(".") + 1) == "node"; 
            });
        auto ele_file = std::find_if(remaining_args.begin(), remaining_args.end(), 
            [](const std::string& file) { 
                return file.substr(file.find_last_of(".") + 1) == "ele"; 
            });
            
        if (node_file != remaining_args.end() && ele_file != remaining_args.end()) {
            // Found explicit files
            options.node_file = *node_file;
            options.ele_file = *ele_file;
        } else if (remaining_args.size() == 1) {
            // Single argument - treat as base name
            std::string base = remaining_args[0];
            // Only remove known extensions (.node, .ele) to get clean base name
            if (base.length() > 5 && base.substr(base.length() - 5) == ".node") {
                base = base.substr(0, base.length() - 5);
            } else if (base.length() > 4 && base.substr(base.length() - 4) == ".ele") {
                base = base.substr(0, base.length() - 4);
            }
            options.node_file = base + ".node";
            options.ele_file = base + ".ele";
        } else {
            std::cerr << "Error: Missing required files (.node, .ele) or provide single base name\n";
            return false;
        }
        
        // Auto-generate output name if not provided
        if (options.output_name.empty()) {
            std::string base = options.node_file.substr(0, options.node_file.find_last_of("."));
            options.output_name = base;
        }
    } else if (options.input_type == ProgramOptions::POLY) {
        if (remaining_args.size() != 1) {
            std::cerr << "Error: Exactly one .poly file must be specified\n";
            return false;
        }
        options.poly_file = remaining_args[0];
    }
    
    return true;
}

bool validate_file_extensions(const ProgramOptions& options) {
    // File existence and extension validation
    
    if (options.input_type == ProgramOptions::OFF) {
        if (!file_exists(options.off_file)) {
            std::cerr << "Error: File '" << options.off_file << "' does not exist" << std::endl;
            return false;
        }
        if (options.off_file.substr(options.off_file.find_last_of(".") + 1) != "off") {
            std::cerr << "Error: OFF file must have .off extension" << std::endl;
            return false;
        }
    } else if (options.input_type == ProgramOptions::NEIGH || options.input_type == ProgramOptions::ELE) {
        if (!file_exists(options.node_file)) {
            std::cerr << "Error: File '" << options.node_file << "' does not exist" << std::endl;
            return false;
        }
        if (!file_exists(options.ele_file)) {
            std::cerr << "Error: File '" << options.ele_file << "' does not exist" << std::endl;
            return false;
        }
        if (options.node_file.substr(options.node_file.find_last_of(".") + 1) != "node") {
            std::cerr << "Error: node file must have .node extension" << std::endl;
            return false;
        }
        if (options.ele_file.substr(options.ele_file.find_last_of(".") + 1) != "ele") {
            std::cerr << "Error: ele file must have .ele extension" << std::endl;
            return false;
        }
        if (options.input_type == ProgramOptions::NEIGH) {
            if (!file_exists(options.neigh_file)) {
                std::cerr << "Error: File '" << options.neigh_file << "' does not exist" << std::endl;
                return false;
            }
            if (options.neigh_file.substr(options.neigh_file.find_last_of(".") + 1) != "neigh") {
                std::cerr << "Error: neigh file must have .neigh extension" << std::endl;
                return false;
            }
        }
    } else if (options.input_type == ProgramOptions::POLY) {
        if (!file_exists(options.poly_file)) {
            std::cerr << "Error: File '" << options.poly_file << "' does not exist" << std::endl;
            return false;
        }
    }
    return true;
}

// Helper function template to execute common mesh operations
template<typename MeshType>
void execute_mesh_operations(MeshType& mesh, const ProgramOptions& options) {
    // Always generate stats
    mesh.print_stats(options.output_name + ".json");
    std::cout << "output json in " << options.output_name << ".json" << std::endl;
    
    // Generate format-specific output
    switch (options.output_format) {
        case ProgramOptions::OFF_FORMAT:
            mesh.print_OFF(options.output_name + ".off");
            std::cout << "output off in " << options.output_name << ".off" << std::endl;
            break;
    }
}

// Helper function for OFF file processing
void process_off_file(const ProgramOptions& options) {
#ifdef CUDA_AVAILABLE
    if (options.use_gpu) {
        // Use GPU version (temporarily with bool parameter, will be updated in Phase 3)
        GPolylla mesh(options.off_file, options.polylla_options.use_regions);
        execute_mesh_operations(mesh, options);
    } else {
#endif
        // Use CPU version
        Polylla mesh(options.off_file, options.polylla_options);
        execute_mesh_operations(mesh, options);
#ifdef CUDA_AVAILABLE
    }
#endif
}

// Helper function for NEIGH file processing
void process_neigh_files(const ProgramOptions& options) {
#ifdef CUDA_AVAILABLE
    if (options.use_gpu) {
        // Use GPU version (temporarily with bool parameter, will be updated in Phase 3)
        GPolylla mesh(options.node_file, options.ele_file, options.neigh_file, options.polylla_options.use_regions);
        execute_mesh_operations(mesh, options);
    } else {
#endif
        // Use CPU version
        Polylla mesh(options.node_file, options.ele_file, options.neigh_file, options.polylla_options);
        execute_mesh_operations(mesh, options);
#ifdef CUDA_AVAILABLE
    }
#endif
}

// Helper function for ELE file processing (CPU only)
void process_ele_files(const ProgramOptions& options) {
#ifdef CUDA_AVAILABLE
    if (options.use_gpu) {
        std::cerr << "Error: GPU version does not support --ele mode (only --off and --neigh)" << std::endl;
        std::cerr << "Use CPU version (remove --gpu flag) or use --neigh mode instead" << std::endl;
        throw std::runtime_error("GPU mode not supported for --ele");
    }
#endif
    // Use CPU version (ELE mode only supported in CPU)
    Polylla mesh(options.node_file, options.ele_file, options.polylla_options);
    execute_mesh_operations(mesh, options);
}

// Helper function for POLY file processing
void process_poly_file(const ProgramOptions& options) {
    // Determine triangle arguments based on region flag
    std::string triangle_args = options.triangle_args;
    if (options.polylla_options.use_regions && triangle_args == "pnz") {
        // Auto-upgrade to include region attributes when --region is used
        triangle_args = "pnzAa";
        std::cout << "Region mode enabled: using triangle arguments 'pnzAa'" << std::endl;
    }
    
    // Get base name for output files
    std::string base = options.poly_file;
    if (base.length() > 5 && base.substr(base.length() - 5) == ".poly") {
        base = base.substr(0, base.length() - 5);
    }
    
    // Construct triangle command (Unix)
    std::string triangle_path = "./bin/triangle";
    std::string triangle_cmd = triangle_path + " -" + triangle_args + " " + options.poly_file;
    
    // Visual separation before Triangle execution
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "                          TRIANGLE EXECUTION" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "Executing: " << triangle_cmd << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    // Execute triangle
    int result = system(triangle_cmd.c_str());
    
    // Visual separation after Triangle execution
    std::cout << std::string(80, '-') << std::endl;
    if (result != 0) {
        std::cout << "Triangle execution FAILED with code " << result << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        throw std::runtime_error("Triangle execution failed with code " + std::to_string(result));
    } else {
        std::cout << "Triangle execution completed successfully!" << std::endl;
    }
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "                          POLYLLA PROCESSING" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    // Set up file paths for generated files (triangle outputs with .1 suffix)
    ProgramOptions modified_options = options;
    modified_options.input_type = ProgramOptions::NEIGH;
    modified_options.node_file = base + ".1.node";
    modified_options.ele_file = base + ".1.ele";
    modified_options.neigh_file = base + ".1.neigh";
    
    // Auto-generate output name if not provided
    if (modified_options.output_name.empty()) {
        modified_options.output_name = base;
    }
    
    // Verify triangle generated the expected files
    if (!file_exists(modified_options.node_file)) {
        throw std::runtime_error("Triangle did not generate expected .node file: " + modified_options.node_file);
    }
    if (!file_exists(modified_options.ele_file)) {
        throw std::runtime_error("Triangle did not generate expected .ele file: " + modified_options.ele_file);
    }
    if (!file_exists(modified_options.neigh_file)) {
        throw std::runtime_error("Triangle did not generate expected .neigh file: " + modified_options.neigh_file);
    }
    
    std::cout << "Triangle completed successfully. Processing with Polylla..." << std::endl;
    
    // Process with Polylla using the generated files
    process_neigh_files(modified_options);
}

int main(int argc, char **argv) {
    ProgramOptions options;
    
    if (!parse_arguments(argc, argv, options)) {
        print_usage(argv[0]);
        return 1;
    }
    
    if (options.help) {
        print_usage(argv[0]);
        return 0;
    }
    
    if (!validate_file_extensions(options)) {
        return 1;
    }
    
    // Validate GPU flag
    if (options.use_gpu) {
#ifndef CUDA_AVAILABLE
        std::cerr << "Error: GPU acceleration requested but CUDA support not available (compiled without CUDA)" << std::endl;
        std::cerr << "Please rebuild with CUDA or remove --gpu flag" << std::endl;
        return 1;
#else
        std::cout << "GPU acceleration enabled" << std::endl;
#endif
    }
    
    // Validate Polylla options
    if (!validate_polylla_options(options.polylla_options)) {
        return 1;
    }
    
    // Print configuration if regions or smoothing are enabled
    if (options.polylla_options.use_regions) {
        std::cout << "Region reading and verification enabled" << std::endl;
    }
    if (!options.polylla_options.smooth_method.empty()) {
        std::cout << "Smoothing enabled: " << options.polylla_options.smooth_method 
                  << " (iterations: " << options.polylla_options.smooth_iterations << ")" << std::endl;
        if (options.polylla_options.smooth_method == "distmesh" && options.polylla_options.target_length > 0) {
            std::cout << "Target length: " << options.polylla_options.target_length << std::endl;
        }
    }
    
    try {
        // Process mesh based on input type and GPU selection
        switch (options.input_type) {
            case ProgramOptions::OFF:
                process_off_file(options);
                break;
                
            case ProgramOptions::NEIGH:
                process_neigh_files(options);
                break;
                
            case ProgramOptions::ELE:
                process_ele_files(options);
                break;
                
            case ProgramOptions::POLY:
                process_poly_file(options);
                break;
                
            default:
                std::cerr << "Error: No valid input type specified" << std::endl;
                return 1;
        }
        
        // Uncomment when ALE output is needed
        // mesh.print_ALE(options.output_name + ".ale");
        // std::cout << "output ale in " << options.output_name << ".ale" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
