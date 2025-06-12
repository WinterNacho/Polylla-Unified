#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <polylla.hpp>
#include <triangulation.hpp>

struct ProgramOptions {
    enum InputType { NONE, OFF, NEIGH, ELE };
    enum OutputFormat { OFF_FORMAT };
    
    InputType input_type = NONE;
    OutputFormat output_format = OFF_FORMAT;  // Default format
    std::string node_file;
    std::string ele_file;
    std::string neigh_file;
    std::string off_file;
    std::string output_name;
    
    // Polylla options
    PolyllaOptions polylla_options;
    
    // Additional flags
    bool help = false;
};

bool is_positive_num(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!isdigit(c)) return false;
    }
    return true;
}

bool validate_polylla_options(const PolyllaOptions& options) {
    // Business logic validations (not covered by parse_arguments)
    
    // Validate target length for distmesh method
    if (options.smooth_method == "distmesh" && options.target_length == 0) {
        std::cout << "Error: Target length cannot be zero for distmesh method" << std::endl;
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
    std::cout << "  -e, --ele            Use .node and .ele files as input (without .neigh)\n\n";
    std::cout << "Options:\n";
    std::cout << "  -g, --gpu            Enable GPU(CUDA)\n";
    std::cout << "  -r, --region         Read and process triangulation considering regions\n";
    std::cout << "  -s, --smooth METHOD  Use smoothing method: laplacian, laplacian-edge-ratio, distmesh\n";
    std::cout << "  -i, --iterations N   Number of smoothing iterations (default: 50)\n";
    std::cout << "  -t, --target-length N Target edge length for distmesh method\n";
    std::cout << "  -O, --output FORMAT  Specify output format: off (default)\n";
    std::cout << "  -h, --help           Show this help message\n\n";
}

bool parse_arguments(int argc, char** argv, ProgramOptions& options) {
    static struct option long_options[] = {
        {"off",           no_argument,       0, 'o'},
        {"neigh",         no_argument,       0, 'n'},
        {"ele",           no_argument,       0, 'e'},
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
    
    while ((c = getopt_long(argc, argv, "onres:i:t:O:h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'o':
                if (options.input_type != ProgramOptions::NONE) {
                    std::cout << "Error: Multiple input types specified\n";
                    return false;
                }
                options.input_type = ProgramOptions::OFF;
                break;
                
            case 'n':
                if (options.input_type != ProgramOptions::NONE) {
                    std::cout << "Error: Multiple input types specified\n";
                    return false;
                }
                options.input_type = ProgramOptions::NEIGH;
                break;
                
            case 'e':
                if (options.input_type != ProgramOptions::NONE) {
                    std::cout << "Error: Multiple input types specified\n";
                    return false;
                }
                options.input_type = ProgramOptions::ELE;
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
                        std::cout << "Error: Invalid smoothing method '" << method << "'\n";
                        std::cout << "Valid methods: laplacian, laplacian-edge-ratio, distmesh\n";
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
                        std::cout << "Error: Invalid value '" << iter_str << "' for iterations. Must be a positive number.\n";
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
                        std::cout << "Error: Invalid value '" << length_str << "' for target-length. Must be a positive number.\n";
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
                        std::cout << "Error: Unknown output format '" << format << "'. Supported: off" << std::endl;
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
        std::cout << "Error: No input type specified. Use --help for usage information.\n";
        return false;
    }
    
    // Process input files based on type
    if (options.input_type == ProgramOptions::OFF) {
        auto off_file = std::find_if(remaining_args.begin(), remaining_args.end(), 
            [](const std::string& file) { 
                return file.substr(file.find_last_of(".") + 1) == "off"; 
            });
        if (off_file == remaining_args.end()) {
            std::cout << "Error: No .off file found in arguments\n";
            return false;
        }
        options.off_file = *off_file;
        
        // Auto-generate output name if not provided
        if (options.output_name.empty()) {
            options.output_name = options.off_file.substr(0, options.off_file.find_last_of("."));
        }
        
    } else if (options.input_type == ProgramOptions::NEIGH) {
        // Find required files
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
            
        if (node_file == remaining_args.end() || ele_file == remaining_args.end() || neigh_file == remaining_args.end()) {
            std::cout << "Error: Missing required files (.node, .ele, .neigh)\n";
            return false;
        }
        
        options.node_file = *node_file;
        options.ele_file = *ele_file;
        options.neigh_file = *neigh_file;
        
        // Auto-generate output name if not provided
        if (options.output_name.empty()) {
            std::string base = options.node_file.substr(0, options.node_file.find_last_of("."));
            options.output_name = base;
        }
        
    } else if (options.input_type == ProgramOptions::ELE) {
        // Similar logic for ELE type
        auto node_file = std::find_if(remaining_args.begin(), remaining_args.end(), 
            [](const std::string& file) { 
                return file.substr(file.find_last_of(".") + 1) == "node"; 
            });
        auto ele_file = std::find_if(remaining_args.begin(), remaining_args.end(), 
            [](const std::string& file) { 
                return file.substr(file.find_last_of(".") + 1) == "ele"; 
            });
            
        if (node_file == remaining_args.end() || ele_file == remaining_args.end()) {
            std::cout << "Error: Missing required files (.node, .ele)\n";
            return false;
        }
        
        options.node_file = *node_file;
        options.ele_file = *ele_file;
        
        // Auto-generate output name if not provided
        if (options.output_name.empty()) {
            std::string base = options.node_file.substr(0, options.node_file.find_last_of("."));
            options.output_name = base;
        }
    }
    
    return true;
}

bool validate_file_extensions(const ProgramOptions& options) {
    if (options.input_type == ProgramOptions::OFF) {
        if (options.off_file.substr(options.off_file.find_last_of(".") + 1) != "off") {
            std::cout << "Error: OFF file must have .off extension" << std::endl;
            return false;
        }
    } else if (options.input_type == ProgramOptions::NEIGH || options.input_type == ProgramOptions::ELE) {
        if (options.node_file.substr(options.node_file.find_last_of(".") + 1) != "node") {
            std::cout << "Error: node file must have .node extension" << std::endl;
            return false;
        }
        if (options.ele_file.substr(options.ele_file.find_last_of(".") + 1) != "ele") {
            std::cout << "Error: ele file must have .ele extension" << std::endl;
            return false;
        }
        if (options.input_type == ProgramOptions::NEIGH) {
            if (options.neigh_file.substr(options.neigh_file.find_last_of(".") + 1) != "neigh") {
                std::cout << "Error: neigh file must have .neigh extension" << std::endl;
                return false;
            }
        }
    }
    return true;
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
        // Create mesh based on input type using new PolyllaOptions
        switch (options.input_type) {
            case ProgramOptions::OFF: {
                Polylla mesh(options.off_file, options.polylla_options);
                
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
                break;
            }
                
            case ProgramOptions::NEIGH: {
                Polylla mesh(options.node_file, options.ele_file, options.neigh_file, options.polylla_options);
                
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
                break;
            }
                
            case ProgramOptions::ELE: {
                Polylla mesh(options.node_file, options.ele_file, options.polylla_options);
                
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
                break;
            }
                
            default:
                std::cout << "Error: No valid input type specified" << std::endl;
                return 1;
        }
        
        // Uncomment when ALE output is needed
        // mesh.print_ALE(options.output_name + ".ale");
        // std::cout << "output ale in " << options.output_name << ".ale" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
