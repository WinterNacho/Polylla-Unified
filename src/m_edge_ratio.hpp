#ifndef MEASURE_EDGE_RATIO_HPP
#define MEASURE_EDGE_RATIO_HPP

#include <measure.hpp>
#include <triangulation.hpp>

class EdgeRatio : public Measure {
private:
public:
    using Measure::Measure;
    const double eval_face(const int face_index) const override {
        return 1.0;
    };

};

#endif // MEASURE_EDGE_RATIO_HPP
