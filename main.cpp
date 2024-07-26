#include <algorithm>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <polylla.hpp>
#include <triangulation.hpp>

bool is_positive_num(std::string s) {
    for (auto c : s) {
        if (!isdigit(c)) return false;
    }
    return true;
}


int main(int argc, char **argv) {
    int opt_args = 0;

    int arg_index = std::find_if(argv + 1, argv + argc, [&](char const * const arg) { return strcmp(arg, "--smooth") == 0; }) - argv;
    std::vector<std::string> smooth_methods = {"laplacian", "laplacian-edge-ratio", "distmesh"};
    if (arg_index < argc) {
        std::string arg_value = std::string(argv[arg_index + 1]);
        if (std::find(smooth_methods.begin(), smooth_methods.end(), arg_value) != smooth_methods.end()) {
            opt_args += 2;
            smooth_method = arg_value;
        } else {
            std::cout<< "Invalid value " << arg_value << " for optional argument --smooth"<<std::endl;
            std::cout<< "Valid values:" <<std::endl;
            for (auto s : smooth_methods) {
                std::cout << s << std::endl;
            }
            return 0;
        }
    }

    arg_index = std::find_if(argv + 1, argv + argc, [&](char const * const arg) { return strcmp(arg, "--smooth-iter") == 0; }) - argv;
    if (arg_index < argc) {
        std::string arg_value = std::string(argv[arg_index + 1]);
        if (is_positive_num(arg_value)) {
            opt_args += 2;
            max_smooth_iterations = stoi(arg_value);
        } else {
            std::cout<< "Invalid value " << arg_value << " for optional argument --smooth-iter, value should be a positive number"<<std::endl;
            return 0;
        }
    }

    arg_index = std::find_if(argv + 1, argv + argc, [&](char const * const arg) { return strcmp(arg, "--target-length") == 0; }) - argv;
    if (arg_index < argc) {
        std::string arg_value = std::string(argv[arg_index + 1]);
        if (is_positive_num(arg_value) || stoi(arg_value) == 0) {
            opt_args += 2;
            distmesh_target_length = stoi(arg_value);
        } else {
            std::cout<< "Invalid value " << arg_value << " for optional argument --target-length, value should be a positive number"<<std::endl;
            return 0;
        }
    }

    if(argc == opt_args + 5)
    {
        std::string node_file = std::string(argv[1]);
        std::string ele_file = std::string(argv[2]);
        std::string neigh_file = std::string(argv[3]);
        std::string output = std::string(argv[4]);

        if(node_file.substr(node_file.find_last_of(".") + 1) != "node"){
            std::cout<<"Error: node file must be .node"<<std::endl;
            return 0;
        }
        if(ele_file.substr(ele_file.find_last_of(".") + 1) != "ele"){
            std::cout<<"Error: ele file must be .ele"<<std::endl;
            return 0;
        }
        if(neigh_file.substr(neigh_file.find_last_of(".") + 1) != "neigh"){
            std::cout<<"Error: neigh file must be .neigh"<<std::endl;
            return 0;
        }

        Polylla mesh(node_file, ele_file, neigh_file);
        
        mesh.print_stats(output + ".json");
        std::cout<<"output json in "<<output<<".json"<<std::endl;
        mesh.print_OFF(output+".off");
        std::cout<<"output off in "<<output<<".off"<<std::endl;
        //mesh.print_ALE(output+".ale");
        //std::cout<<"output ale in "<<output<<".ale"<<std::endl;
    }else if (argc == opt_args + 3){
        std::string off_file = std::string(argv[1]);
        std::string output = std::string(argv[2]);
	    Polylla mesh(off_file);

        mesh.print_stats(output + ".json");
        std::cout<<"output json in "<<output<<".json"<<std::endl;
        mesh.print_OFF(output+".off");
        std::cout<<"output off in "<<output<<".off"<<std::endl;
        //mesh.print_ALE(output+".ale");
        //std::cout<<"output ale in "<<output<<".ale"<<std::endl;
    }else if (argc == opt_args + 2){
        int size = atoi(argv[1]);
        std::string output = "uniform_" + std::string(argv[1]) + "_CPU";

        Polylla mesh(size);
        std::cout<<"Done!"<<std::endl;

        mesh.print_stats(output + ".json");
        std::cout<<"output json in "<<output<<".json"<<std::endl;

        mesh.print_OFF(output +".off");
        std::cout<<"output off in "<<output<<".off"<<std::endl;
    }else{
        std::cout<<"Usage: "<<argv[0]<<" <off file .off> <output name>"<<std::endl;
        std::cout<<"Usage: "<<argv[0]<<" <node_file .node> <ele_file .ele> <neigh_file .neigh> <output name>"<<std::endl;
        return 0;
    }
    

    
    
	return 0;
}
