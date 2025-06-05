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
    
    // Additional flags
    bool use_gpu = false;
    bool use_regions = false;
    bool help = false;
};

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS] [FILES...]\n\n";
    std::cout << "Input modes:\n";
    std::cout << "  -o, --off            Use OFF file as input\n";
    std::cout << "  -n, --neigh          Use .node, .ele, and .neigh files as input\n";
    std::cout << "  -e, --ele            Use .node and .ele files as input (without .neigh)\n\n";
    std::cout << "Options:\n";
    std::cout << "  -g, --gpu            Enable GPU(CUDA)\n";
    std::cout << "  -r, --region         Read and process triangulation considering regions\n";
    std::cout << "  -O, --output FORMAT  Specify output format: off (default)\n";
    std::cout << "  -h, --help           Show this help message\n\n";
}

bool parse_arguments(int argc, char** argv, ProgramOptions& options) {
    static struct option long_options[] = {
        {"off",     no_argument,       0, 'o'},
        {"neigh",   no_argument,       0, 'n'},
        {"ele",     no_argument,       0, 'e'},
        {"gpu",     no_argument,       0, 'g'},
        {"region",  no_argument,       0, 'r'},
        {"output",  required_argument, 0, 'O'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    while ((c = getopt_long(argc, argv, "onegrO:h", long_options, &option_index)) != -1) {
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
                
            case 'g':
                options.use_gpu = true;
                break;
                
            case 'r':
                options.use_regions = true;
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
    
    // Print configuration if GPU or regions are enabled
    if (options.use_gpu) {
        std::cout << "GPU acceleration enabled (CUDA)" << std::endl;
    }
    if (options.use_regions) {
        std::cout << "Region reading and verification enabled" << std::endl;
    }
    
    try {
        // Create mesh based on input type using direct initialization
        switch (options.input_type) {
            case ProgramOptions::OFF: {
                Polylla mesh(options.off_file, options.use_regions);
                
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
                Polylla mesh(options.node_file, options.ele_file, options.neigh_file, options.use_regions);
                
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
                Polylla mesh(options.node_file, options.ele_file, options.use_regions);
                
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
