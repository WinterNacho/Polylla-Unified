#ifndef MEASURE_HPP
#define MEASURE_HPP

#include <triangulation.hpp>
#include <vector>

class Measure {
private:
    Triangulation *mesh;
    std::vector<int> seeds;
    double average;
public:
    explicit Measure(Triangulation *mesh, std::vector<int>& seeds) {
        this->seeds = seeds;
        this->mesh = mesh;
    }
    virtual const double eval_face(const int face_index) const = 0;
    void eval_mesh() {
        this->average = 0;
        int e_curr;
        for(auto &e_init : seeds){
            std::cout << "new pol" << std::endl;
            e_curr = mesh->next(e_init);
            std::cout << e_curr << std::endl;
            std::cout << e_init << std::endl;
            while(e_init != e_curr){
                std::cout << e_curr << std::endl;
                e_curr = mesh->next(e_curr);
            }

        //     out<<mesh->origin(e_init)<<" ";
        //     e_curr = mesh->next(e_init);
        //     while(e_init != e_curr){
        //         out<<mesh->origin(e_curr)<<" ";
        //         e_curr = mesh_output->next(e_curr);
        //     }
        //     out<<std::endl; 
        }
        this->average /= mesh->faces();
    }
    const double getAverage() const {
        return average;
    }
};

#endif // MEASURE_HPP
