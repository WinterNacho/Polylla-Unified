#ifndef MEASURE_HPP
#define MEASURE_HPP

#include <triangulation.hpp>
#include <vector>

class Measure {
private:
    double average = 0;
    double sum = 0;
    double max = -1;
    double min = -1;
protected:
    Triangulation *mesh;
    std::vector<int> seeds;
public:
    Measure() {}
    explicit Measure(Triangulation *mesh, std::vector<int>& seeds) {
        this->seeds = seeds;
        this->mesh = mesh;
    }
    virtual const double eval_face(const int face_index) const {
        return 0;
    };
    virtual const bool is_better(const double val1, const double val2) const {
        return false;
    }
    void eval_mesh() {
        for(auto &e_init : seeds){
            double face_res = eval_face(e_init);
            this->sum += face_res;
            if (this->max == -1) this->max = face_res;
            if (this->min == -1) this->min = face_res;
            this->max = std::max(this->max, face_res);
            this->min = std::min(this->min, face_res);
        }
        this->average = this->sum / this->seeds.size();
    }
    const double getAverage() const {
        return average;
    }
};

#endif // MEASURE_HPP
