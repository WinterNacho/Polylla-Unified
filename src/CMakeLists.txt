file(GLOB HEADER_FILES *.hpp)

add_library(meshfiles STATIC polylla.hpp triangulation.hpp measure.hpp m_edge_ratio.hpp)
