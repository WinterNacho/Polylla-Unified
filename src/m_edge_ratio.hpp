#ifndef MEASURE_EDGE_RATIO_HPP
#define MEASURE_EDGE_RATIO_HPP

#include <measure.hpp>
#include <triangulation.hpp>

class EdgeRatio : public Measure {
private:
public:
    using Measure::Measure;
    const double eval_face(const int face_index) const {
        int e_curr = face_index;
        double max_edge = -1;
        double min_edge = -1;
        do {
            double e_length = mesh->distance(e_curr);
            if (max_edge < 0) max_edge = e_length;
            if (min_edge < 0) min_edge = e_length;
            max_edge = std::max(max_edge, e_length);
            min_edge = std::min(min_edge, e_length);
            e_curr = mesh->next(e_curr);
        } while (face_index != e_curr);
        return min_edge / max_edge;
    };
    virtual const bool is_better(const double val1, const double val2) const {
        return val1 > val2;
    }

};

#endif // MEASURE_EDGE_RATIO_HPP
