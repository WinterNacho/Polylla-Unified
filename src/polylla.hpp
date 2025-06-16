/* Polygon mesh generator
//POSIBLE BUG: el algoritmo no viaja por todos los halfedges dentro de un poligono, 
    //por lo que pueden haber semillas que no se borren y tener poligonos repetidos de output
*/

#ifndef POLYLLA_HPP
#define POLYLLA_HPP


#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>
#include <iomanip>

#include <triangulation.hpp>
#include <m_edge_ratio.hpp>

#define print_e(eddddge) eddddge<<" ( "<<mesh_input->origin(eddddge)<<" - "<<mesh_input->target(eddddge)<<") "

// Structure for Polylla configuration options
struct PolyllaOptions {
    // Region options
    bool use_regions = false;
    
    // Smoothing options  
    std::string smooth_method = "";           // "", "laplacian", "laplacian-edge-ratio", "distmesh"
    int smooth_iterations = 50;               // default 50
    double target_length = -1;                // -1 = auto-calculate
};

class Polylla
{
private:
    typedef std::vector<int> _polygon; 
    typedef std::vector<char> bit_vector; 

    static constexpr double EPSILON = 1e-6;

    Triangulation *mesh_input; // Halfedge triangulation
    Triangulation *mesh_output;
    std::vector<int> output_seeds; //Seeds of the polygon

    //std::vector<int> triangles; //True if the edge generated a triangle CHANGE!!!!

    bit_vector max_edges; //True if the edge i is a max edge
    bit_vector frontier_edges; //True if the edge i is a frontier edge
    std::vector<int> seed_edges; //Seed edges that generate polygon simple and non-simple

    // Auxiliary array used during the barrier-edge elimination
    std::vector<int> triangle_list;
    bit_vector seed_bet_mark;

    // Configuration options
    PolyllaOptions options;

    //Statistics
    int m_polygons = 0; //Number of polygons
    int n_frontier_edges = 0; //Number of frontier edges
    int n_barrier_edge_tips = 0; //Number of barrier edge tips
    int n_polygons_to_repair = 0;
    int n_polygons_added_after_repair = 0;
    int n_smooth_iterations = 0;

    // Times
    double t_label_max_edges = 0;
    double t_label_frontier_edges = 0;
    double t_label_seed_edges = 0;
    double t_traversal_and_repair = 0;
    double t_traversal = 0;
    double t_repair = 0;
    double t_smooth = 0;
    
public:

    Polylla() {}; //Default constructor

    //Constructor with triangulation
    Polylla(Triangulation *input_mesh, const PolyllaOptions& options = PolyllaOptions()) 
        : mesh_input(input_mesh), options(options) {
        mesh_output = new Triangulation(*mesh_input);
        construct_Polylla();
    }

    //Constructor from a OFF file
    Polylla(const std::string& off_file, const PolyllaOptions& options = PolyllaOptions()) 
        : options(options) {
        this->mesh_input = new Triangulation(off_file, options.use_regions);
        mesh_output = new Triangulation(*mesh_input);
        construct_Polylla();
    }

    //Constructor from a node_file, ele_file and neigh_file
    Polylla(const std::string& node_file, const std::string& ele_file, const std::string& neigh_file, 
            const PolyllaOptions& options = PolyllaOptions()) 
        : options(options) {
        this->mesh_input = new Triangulation(node_file, ele_file, neigh_file, options.use_regions);
        mesh_output = new Triangulation(*mesh_input);
        construct_Polylla();
    }

    //Constructor from a node_file and ele_file only (without neigh_file)
    Polylla(const std::string& node_file, const std::string& ele_file, 
            const PolyllaOptions& options = PolyllaOptions()) 
        : options(options) {
        this->mesh_input = new Triangulation(node_file, ele_file, options.use_regions);
        mesh_output = new Triangulation(*mesh_input);
        construct_Polylla();
    }

    //Constructor random data construictor
    Polylla(int size){
        this->mesh_input = new Triangulation(size);
        mesh_output = new Triangulation(*mesh_input);
        construct_Polylla();
    }

    ~Polylla() {
        //triangles.clear(); 
        max_edges.clear(); 
        frontier_edges.clear();
        seed_edges.clear(); 
        seed_bet_mark.clear();
        triangle_list.clear();
        delete mesh_input;
        delete mesh_output;
    }

    // Configuration methods
    void set_use_regions(bool use_regions) {
        this->options.use_regions = use_regions;
    }

    bool get_use_regions() const {
        return options.use_regions;
    }

    void construct_Polylla(){

        max_edges = bit_vector(mesh_input->halfEdges(), false);
        frontier_edges = bit_vector(mesh_input->halfEdges(), false);
        //triangles = mesh_input->get_Triangles(); //Change by triangle list
        seed_bet_mark = bit_vector(this->mesh_input->halfEdges(), false);

        //terminal_edges = bit_vector(mesh_input->halfEdges(), false);
        //seed_edges = bit_vector(mesh_input->halfEdges(), false);
        
        std::cout<<"Creating Polylla..."<<std::endl;
        
        // Apply smoothing FIRST, before any polygon generation
        if (!options.smooth_method.empty()) {
            auto t_start = std::chrono::high_resolution_clock::now();

            if (options.use_regions) {
                std::cout << "Smoothing with region boundary preservation enabled" << std::endl;        
            }

            if (options.smooth_method == "laplacian") {
                optimize_mesh_laplacian(options.smooth_iterations);
            }
            else if (options.smooth_method == "laplacian-edge-ratio") {
                optimize_mesh_laplacian_constrained(options.smooth_iterations, "laplacian-edge-ratio"); 
            }
            else if (options.smooth_method == "distmesh") {
                optimize_mesh_distmesh(options.smooth_iterations, options.target_length);
            }

            auto t_end = std::chrono::high_resolution_clock::now();
            t_smooth = std::chrono::duration<double, std::milli>(t_end-t_start).count();
            std::string region_info = options.use_regions ? " (preserving region boundaries)" : "";     
            std::cout<<"Optimized mesh in "<<t_smooth<<" ms using "<<options.smooth_method<<" method"<<region_info<<std::endl;
        }
        //Label max edges of each triangle
        auto t_start = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < mesh_input->faces(); i++)
            max_edges[label_max_edge(mesh_input->incident_halfedge(i))] = true;
         
        auto t_end = std::chrono::high_resolution_clock::now();
        t_label_max_edges = std::chrono::duration<double, std::milli>(t_end-t_start).count();
        std::cout<<"Labeled max edges in "<<t_label_max_edges<<" ms"<<std::endl;

        t_start = std::chrono::high_resolution_clock::now();
        //Label frontier edges
        for (std::size_t e = 0; e < mesh_input->halfEdges(); e++){
            if(is_frontier_edge(e)){
                frontier_edges[e] = true;
                n_frontier_edges++;
            }
        }

        t_end = std::chrono::high_resolution_clock::now();
        t_label_frontier_edges = std::chrono::duration<double, std::milli>(t_end-t_start).count();
        std::cout<<"Labeled frontier edges in "<<t_label_frontier_edges<<" ms"<<std::endl;
        
        t_start = std::chrono::high_resolution_clock::now();
        //label seeds edges,
        for (std::size_t e = 0; e < mesh_input->halfEdges(); e++)
            if(mesh_input->is_interior_face(e) && is_seed_edge(e))
                seed_edges.push_back(e);

            
        t_end = std::chrono::high_resolution_clock::now();
        t_label_seed_edges = std::chrono::duration<double, std::milli>(t_end-t_start).count();
        std::cout<<"Labeled seed edges in "<<t_label_seed_edges<<" ms"<<std::endl;

        //Travel phase: Generate polygon mesh
        int polygon_seed;
        //Foreach seed edge generate polygon
        t_start = std::chrono::high_resolution_clock::now();
        for(auto &e : seed_edges){
            polygon_seed = travel_triangles(e);
            //output_seeds.push_back(polygon_seed);
            if(!has_BarrierEdgeTip(polygon_seed)){ //If the polygon is a simple polygon then is part of the mesh
                output_seeds.push_back(polygon_seed);
            }else{ //Else, the polygon is send to reparation phase
                auto t_start_repair = std::chrono::high_resolution_clock::now();
                barrieredge_tip_reparation(polygon_seed);
                auto t_end_repair = std::chrono::high_resolution_clock::now();
                t_repair += std::chrono::duration<double, std::milli>(t_end_repair-t_start_repair).count();
            }         
        }    
        t_end = std::chrono::high_resolution_clock::now();
        t_traversal_and_repair = std::chrono::duration<double, std::milli>(t_end-t_start).count();
        t_traversal = t_traversal_and_repair - t_repair;
        
        this->m_polygons = output_seeds.size();

        // std::cout << mesh_output->get_PointX(508) << ", " << mesh_output->get_PointY(508) << std::endl;

        // for(std::size_t v = 0; v < mesh_input->vertices(); v++) {

        // }
        
//         for(std::size_t v = 0; v < mesh_input->vertices(); v++){
//             mesh_input->set_PointX(v, mesh_input->get_PointX(v) + (rand() % 500 - 250));
//             mesh_input->set_PointY(v, mesh_input->get_PointY(v) + (rand() % 500 - 250));
//             std::cout<<mesh_input->get_PointX(v)<<" "<<mesh_input->get_PointY(v)<<" 0"<<std::endl; 
// }
        // std::size_t v = 156;
        // std::cout << "new_ver" << std::endl;
        // std::cout << mesh_input->degree(v);
        // auto v_init = v;
        // auto e_init = mesh_input->edge_of_vertex(v);
        // auto e_curr = e_init;
        // std::cout << "v_init" << v_init << std::endl;
        // std::cout << "e_init" << e_init << std::endl;
        // do {
        //     auto v_curr = mesh_input->target(e_curr);
        //     std::cout << "origen: " << mesh_input->origin(e_curr) << std::endl;
        //     std::cout << v_curr << std::endl;
        //     // std::cout << v_curr << std::endl;
        //     auto e_twin = mesh_input->twin(e_curr);
        //     e_curr = mesh_input->next(e_twin);
        //     std::cout << "v_curr" << v_curr << std::endl;
        //     std::cout << "e_curr" << e_curr << std::endl;
        //     // std::cout << mesh_input->origin(e_curr) << std::endl;
        // } while (e_curr != e_init);
        // e_init = mesh_output->edge_of_vertex(v);
        // auto e_next = mesh_output->CCW_edge_to_vertex(e_init);
        // std::vector<int> seen = {e_init};
        // std::cout << "v_init" << v_init << std::endl;
        // std::cout << "e_init" << e_init << std::endl;
        // while (std::find(seen.begin(), seen.end(), e_next) == seen.end()) {
        //     seen.push_back(e_next);
        //     auto v_curr = mesh_output->target(e_next);
        //     std::cout << "origen: " << mesh_output->origin(e_next) << std::endl;
        //     std::cout << v_curr << std::endl;
        //     // std::cout << v_curr << std::endl;
        //     auto e_twin = mesh_output->twin(e_next);
        //     e_next = mesh_output->next(e_twin);
        //     std::cout << "v_curr" << v_curr << std::endl;
        //     std::cout << "e_curr" << e_next << std::endl;
            // std::cout << mesh_input->origin(e_curr) << std::endl;
        // }
        
        std::cout<<"Mesh with "<<m_polygons<<" polygons "<<n_frontier_edges/2<<" edges and "<<n_barrier_edge_tips<<" barrier-edge tips."<<std::endl;
        //mesh_input->print_pg(std::to_string(mesh_input->vertices()) + ".pg");             
    }


    void print_stats(std::string filename){
        //Time
        std::cout<<"Time to generate Triangulation: "<<mesh_input->get_triangulation_generation_time()<<" ms"<<std::endl;
        std::cout<<"Time to label max edges "<<t_label_max_edges<<" ms"<<std::endl;
        std::cout<<"Time to label frontier edges "<<t_label_frontier_edges<<" ms"<<std::endl;
        std::cout<<"Time to label seed edges "<<t_label_seed_edges<<" ms"<<std::endl;
        std::cout<<"Time to label total "<<t_label_max_edges+t_label_frontier_edges+t_label_seed_edges<<" ms"<<std::endl;
        std::cout<<"Time to traversal and repair "<<t_traversal_and_repair<<" ms"<<std::endl;
        std::cout<<"Time to traversal "<<t_traversal<<" ms"<<std::endl;
        std::cout<<"Time to repair "<<t_repair<<" ms"<<std::endl;
        std::cout<<"Time to smooth "<<t_smooth<<" ms"<<std::endl;
        std::cout<<"Time to generate polygonal mesh "<<t_label_max_edges + t_label_frontier_edges + t_label_seed_edges + t_traversal_and_repair + t_smooth<<" ms"<<std::endl;

        //Memory
        long long m_max_edges =  sizeof(decltype(max_edges.back())) * max_edges.capacity();
        long long m_frontier_edge = sizeof(decltype(frontier_edges.back())) * frontier_edges.capacity();
        long long m_seed_edges = sizeof(decltype(seed_edges.back())) * seed_edges.capacity();
        long long m_seed_bet_mar = sizeof(decltype(seed_bet_mark.back())) * seed_bet_mark.capacity();
        long long m_triangle_list = sizeof(decltype(triangle_list.back())) * triangle_list.capacity();
        long long m_mesh_input = mesh_input->get_size_vertex_half_edge();
        long long m_mesh_output = mesh_output->get_size_vertex_half_edge();
        long long m_vertices_input = mesh_input->get_size_vertex_struct();
        long long m_vertices_output = mesh_output->get_size_vertex_struct();

        std::ofstream out(filename);
        std::cout<<"Printing JSON file as "<<filename<<std::endl;
        out<<"{"<<std::endl;
        out<<"\"n_polygons\": "<<m_polygons<<","<<std::endl;
        out<<"\"n_frontier_edges\": "<<n_frontier_edges/2<<","<<std::endl;
        out<<"\"n_barrier_edge_tips\": "<<n_barrier_edge_tips<<","<<std::endl;
        out<<"\"n_half_edges\": "<<mesh_input->halfEdges()<<","<<std::endl;
        out<<"\"n_faces\": "<<mesh_input->faces()<<","<<std::endl;
        out<<"\"n_vertices\": "<<mesh_input->vertices()<<","<<std::endl;
        out<<"\"n_polygons_to_repair\": "<<n_polygons_to_repair<<","<<std::endl;
        out<<"\"n_polygons_added_after_repair\": "<<n_polygons_added_after_repair<<","<<std::endl;
        out<<"\"n_smooth_iterations\": "<<n_smooth_iterations<<","<<std::endl;
        out<<"\"time_triangulation_generation\": "<<mesh_input->get_triangulation_generation_time()<<","<<std::endl;
        out<<"\"time_to_label_max_edges\": "<<t_label_max_edges<<","<<std::endl;
        out<<"\"time_to_label_frontier_edges\": "<<t_label_frontier_edges<<","<<std::endl;
        out<<"\"time_to_label_seed_edges\": "<<t_label_seed_edges<<","<<std::endl;
        out<<"\"time_to_label_total\": "<<t_label_max_edges+t_label_frontier_edges+t_label_seed_edges<<","<<std::endl;
        out<<"\"time_to_traversal_and_repair\": "<<t_traversal_and_repair<<","<<std::endl;
        out<<"\"time_to_traversal\": "<<t_traversal<<","<<std::endl;
        out<<"\"time_to_repair\": "<<t_repair<<","<<std::endl;
        out<<"\"time_to_smooth\": "<<t_smooth<<","<<std::endl;
        out<<"\"time_to_generate_polygonal_mesh\": "<<t_label_max_edges + t_label_frontier_edges + t_label_seed_edges + t_traversal_and_repair + t_smooth<<","<<std::endl;
        out<<"\t\"memory_max_edges\": "<<m_max_edges<<","<<std::endl;
        out<<"\t\"memory_frontier_edge\": "<<m_frontier_edge<<","<<std::endl;
        out<<"\t\"memory_seed_edges\": "<<m_seed_edges<<","<<std::endl;
        out<<"\t\"memory_seed_bet_mar\": "<<m_seed_bet_mar<<","<<std::endl;
        out<<"\t\"memory_triangle_list\": "<<m_triangle_list<<","<<std::endl;
        out<<"\t\"memory_mesh_input\": "<<m_mesh_input<<","<<std::endl;
        out<<"\t\"memory_mesh_output\": "<<m_mesh_output<<","<<std::endl;
        out<<"\t\"memory_vertices_input\": "<<m_vertices_input<<","<<std::endl;
        out<<"\t\"memory_vertices_output\": "<<m_vertices_output<<","<<std::endl;
        out<<"\t\"memory_total\": "<<m_max_edges + m_frontier_edge + m_seed_edges + m_seed_bet_mar + m_triangle_list + m_mesh_input + m_mesh_output + m_vertices_input + m_vertices_output<<std::endl;
        out<<"}"<<std::endl;
        out.close();
    }


    //Print ale file of the polylla mesh
    void print_ALE(std::string filename){
        std::ofstream out(filename);
        _polygon poly;
        out<<"# domain type\nCustom\n";
        out<<"# nodal coordinates: number of nodes followed by the coordinates \n";
        out<<mesh_input->vertices()<<std::endl;
        //print nodes
        for(std::size_t v = 0; v < mesh_input->vertices(); v++)
            out<<std::setprecision(15)<<mesh_input->get_PointX(v)<<" "<<mesh_input->get_PointY(v)<<std::endl; 
        out<<"# element connectivity: number of elements followed by the elements\n";
        out<<this->m_polygons<<std::endl;
        //print polygons
        int size_poly;
        int e_curr;
        for(auto &e_init : output_seeds){
            size_poly = 1;
            e_curr = mesh_output->next(e_init);
            while(e_init != e_curr){
                size_poly++;
                e_curr = mesh_output->next(e_curr);
            }
            out<<size_poly<<" ";            

            out<<mesh_output->origin(e_init)<<" ";
            e_curr = mesh_output->next(e_init);
            while(e_init != e_curr){
                out<<mesh_output->origin(e_curr)<<" ";
                e_curr = mesh_output->next(e_curr);
            }
            out<<std::endl; 
        }
        //Print borderedges
        out<<"# indices of nodes located on the Dirichlet boundary\n";
        ///Find borderedges
        int b_curr, b_init = 0;
        for(std::size_t i = mesh_input->halfEdges()-1; i != 0; i--){
            if(mesh_input->is_border_face(i)){
                b_init = i;
                break;
            }
        }
        out<<mesh_input->origin(b_init)<<" ";
        b_curr = mesh_input->prev(b_init);
        while(b_init != b_curr){
            out<<mesh_input->origin(b_curr)<<" ";
            b_curr = mesh_input->prev(b_curr);
        }
        out<<std::endl;
        out<<"# indices of nodes located on the Neumann boundary\n0\n";
        out<<"# xmin, xmax, ymin, ymax of the bounding box\n";
        double xmax = mesh_input->get_PointX(0);
        double xmin = mesh_input->get_PointX(0);
        double ymax = mesh_input->get_PointY(0);
        double ymin = mesh_input->get_PointY(0);
        //Search min and max coordinates
        for(std::size_t v = 0; v < mesh_input->vertices(); v++){
            //search range x
            if(mesh_input->get_PointX(v) > xmax )
                xmax = mesh_input->get_PointX(v);
            if(mesh_input->get_PointX(v) < xmin )
                xmin = mesh_input->get_PointX(v);
            //search range y
            if(mesh_input->get_PointY(v) > ymax )
                ymax = mesh_input->get_PointY(v);
            if(mesh_input->get_PointY(v) < ymin )
                ymin = mesh_input->get_PointY(v);
        }
        out<<xmin<<" "<<xmax<<" "<<ymin<<" "<<ymax<<std::endl;
        out.close();
    }

    //Print off file of the polylla mesh
    void print_OFF(std::string filename) {
        std::ofstream out(filename);

        out << "OFF" << std::endl;
        out << std::setprecision(15) << mesh_input->vertices() << " " << m_polygons << " " << n_frontier_edges / 2 << std::endl;

        // Print vertices
        for (int i = 0; i < mesh_input->vertices(); i++) {
            out << mesh_input->get_PointX(i) << " " << mesh_input->get_PointY(i) << " 0" << std::endl;
        }

        // Print polygons
        for (int i = 0; i < m_polygons; i++) {
            int e_init = output_seeds[i];
            int e_curr = e_init;
            std::vector<int> polygon_vertices;

            // Collect vertex indices of the polygon
            do {
                polygon_vertices.push_back(mesh_output->origin(e_curr));
                e_curr = mesh_output->next(e_curr);
            } while (e_curr != e_init);

            // Write polygon
            out << polygon_vertices.size();
            for (int v : polygon_vertices)
                out << " " << v;
                
            // Add colors only if using regions
            if (options.use_regions) {
                // Get region from the original mesh (via first halfedge)
                int region = mesh_input->region_face(mesh_input->index_face(e_init));

                // Generate different RGB colors using prime numbers based on the region
                float r = (region * 73 % 256) / 255.0f;
                float g = (region * 149 % 256) / 255.0f;
                float b = (region * 233 % 256) / 255.0f;

                out << " " << r << " " << g << " " << b << " 1.0";
            }
            out << std::endl;
        }

        out.close();
    }

private:

    //Return true if it is the edge is terminal-edge or terminal border edge, 
    //but it only selects one halfedge as terminal-edge, the halfedge with lowest index is selected
    bool is_seed_edge(int e){
        int twin = mesh_input->twin(e);

        bool is_terminal_edge = (mesh_input->is_interior_face(twin) && (max_edges[e] && max_edges[twin]));
        bool is_terminal_border_edge = (mesh_input->is_border_face(twin) && max_edges[e]);
        
        bool is_terminal_region_edge = false;
        if (options.use_regions) {
            bool is_region_boundary = false;
            int region1 = mesh_input->region_face(mesh_input->index_face(e));
            int region2 = mesh_input->region_face(mesh_input->index_face(twin));
            is_region_boundary = (region1 != region2);
            is_terminal_region_edge = (is_region_boundary && max_edges[e]);
        }

        if((is_terminal_edge && e < twin) || is_terminal_border_edge || is_terminal_region_edge){
            return true;
        }

        return false;
    }

    int Equality(double a, double b, double eps = EPSILON)
    {
    return fabs(a - b) < eps;
    }
    
    int GreaterEqualthan(double a, double b, double eps = EPSILON){
            return Equality(a,b,eps) || a > b;
    }

    //Label max edges of all triangles in the triangulation
    //input: edge e indicent to a triangle t
    //output: position of edge e in max_edges[e] is labeled as true
    int label_max_edge(const int e)
    {
        //Calculates the size of each edge of a triangle 
        double dist0 = mesh_input->distance(e);
        double dist1 = mesh_input->distance(mesh_input->next(e));
        double dist2 = mesh_input->distance(mesh_input->prev(e));
        //Find the longest edge of the triangle
        if(std::max({dist0, dist1, dist2}) == dist0)
            return e;
        else if(std::max({dist0, dist1, dist2}) == dist1)
            return mesh_input->next(e);
        else
            return mesh_input->prev(e);
        return -1;

    }

 
    //Return true if the edge e is the lowest edge both triangles incident to e
    //in case of border edges, they are always labeled as frontier-edge
    bool is_frontier_edge(const int e)
    {
        int twin = mesh_input->twin(e);
        bool is_border_edge = mesh_input->is_border_face(e) || mesh_input->is_border_face(twin);
        bool is_not_max_edge = !(max_edges[e] || max_edges[twin]);

        bool is_region_boundary = false;
        if (options.use_regions) {
            int region1 = mesh_input->region_face(mesh_input->index_face(e));
            int region2 = mesh_input->region_face(mesh_input->index_face(twin));
            is_region_boundary = (region1 != region2);
        }
        
        return is_border_edge || is_not_max_edge || is_region_boundary;

    }

    //Travel in CCW order around the edges of vertex v from the edge e looking for the next frontier edge
    int search_frontier_edge(const int e)
    {
        int nxt = e;
        while(!frontier_edges[nxt])
            nxt = mesh_input->CW_edge_to_vertex(nxt);
        return nxt;
    }

    //return true if the polygon is not simple
    bool has_BarrierEdgeTip(int e_init){

        int e_curr = mesh_output->next(e_init);
        //travel inside frontier-edges of polygon
        while(e_curr != e_init){   
            //if the twin of the next halfedge is the current halfedge, then the polygon is not simple
            if( mesh_output->twin(mesh_output->next(e_curr)) == e_curr)
                return true;
            //travel to next half-edge
            e_curr = mesh_output->next(e_curr);
        }
        return false;
    }   

    //generate a polygon from a seed edge
    //input: Seed-edge
    //Output: seed frontier-edge of new popygon
    int travel_triangles(const int e)
    {   
        //search next frontier-edge
        int e_init = search_frontier_edge(e);
        int e_curr = mesh_input->next(e_init);        
        int e_fe = e_init; 
        // std::cout << "new" <<std::endl;
        //travel inside frontier-edges of polygon
        do{   
            e_curr = search_frontier_edge(e_curr);
            //update next of previous frontier-edge
            mesh_output->set_next(e_fe, e_curr);  
            //update prev of current frontier-edge
            mesh_output->set_prev(e_curr, e_fe);

            int v_curr = mesh_input->target(e_fe);
            int e_incident = mesh_input->twin(e_fe);
            // std::cout << "travelling "<< v_curr << "-"<< std::endl;
            // if (v_curr == 8||v_curr == 5||v_curr == 9||v_curr == 12||v_curr == 10||v_curr == 38||v_curr == 14) {
            //     std::cout << "v " << v_curr << "e " <<e_incident << std::endl;
            // }
            mesh_output->set_incident_halfedge(v_curr, e_incident);

            //travel to next half-edge
            e_fe = e_curr;
            e_curr = mesh_input->next(e_curr);
        }while(e_fe != e_init);
        return e_init;
    }
    
    //Given a barrier-edge tip v, return the middle edge incident to v
    //The function first calculate the degree of v - 1 and then divide it by 2, after travel to until the middle-edge
    //input: vertex v
    //output: edge incident to v
    int calculate_middle_edge(const int v){
        int frontieredge_with_bet = this->search_frontier_edge(mesh_input->edge_of_vertex(v));
        int internal_edges =mesh_input->degree(v) - 1; //internal-edges incident to v
        int adv = (internal_edges%2 == 0) ? internal_edges/2 - 1 : internal_edges/2 ;
        int nxt = mesh_input->CW_edge_to_vertex(frontieredge_with_bet);
        //back to traversing the edges of v_bet until select the middle-edge
        while (adv != 0){
            nxt = mesh_input->CW_edge_to_vertex(nxt);
            adv--;
        }
        return nxt;
    }

    //Given a seed edge e that generated polygon, split the polygon until remove al barrier-edge tips
    //input: seed edge e, polygon poly
    //output: polygon without barrier-edge tips
    void barrieredge_tip_reparation(const int e)
    {
        this->n_polygons_to_repair++;
        int x, y, i;
        int t1, t2;
        int middle_edge, v_bet;

        int e_init = e;
        int e_curr = mesh_output->next(e_init);
        //search by barrier-edge tips
        while(e_curr != e_init){   
            //if the twin of the next halfedge is the current halfedge, then the polygon is not simple
            if( mesh_output->twin(mesh_output->next(e_curr)) == e_curr){
                //std::cout<<"e_curr "<<e_curr<<" e_next "<<mesh_output->next(e_curr)<<" next del next "<<mesh_output->next(mesh_output->next(e_curr))<<" twin curr "<<mesh_output->twin(e_curr)<<" twin next "<<mesh_output->twin(mesh_output->next(e_curr))<<std::endl;

                n_barrier_edge_tips++;
                n_frontier_edges+=2;

                //select edge with bet
                v_bet = mesh_output->target(e_curr);
                middle_edge = calculate_middle_edge(v_bet);

                //middle edge that contains v_bet
                t1 = middle_edge;
                t2 = mesh_output->twin(middle_edge);
                
                //edges of middle-edge are labeled as frontier-edge
                this->frontier_edges[t1] = true;
                this->frontier_edges[t2] = true;

                //edges are use as seed edges and saves in a list
                triangle_list.push_back(t1);
                triangle_list.push_back(t2);

                seed_bet_mark[t1] = true;
                seed_bet_mark[t2] = true;
            }
                
            //travel to next half-edge
            e_curr = mesh_output->next(e_curr);
        }

        int t_curr;
        //generate polygons from seeds,
        //two seeds can generate the same polygon
        //so the bit_vector seed_bet_mark is used to label as false the edges that are already used
        int new_polygon_seed;
        while (!triangle_list.empty()){
            t_curr = triangle_list.back();
            triangle_list.pop_back();
            if(seed_bet_mark[t_curr]){
                this->n_polygons_added_after_repair++;
                seed_bet_mark[t_curr] = false;
                new_polygon_seed = generate_repaired_polygon(t_curr, seed_bet_mark);
                //Store the polygon in the as part of the mesh
                output_seeds.push_back(new_polygon_seed);
            }
        }

    }

/*
    //Generate a polygon from a seed-edge and remove repeated seed from seed_list
    //POSIBLE BUG: el algoritmo no viaja por todos los halfedges dentro de un poligono, 
    //por lo que pueden haber semillas que no se borren y tener poligonos repetidos de output
    int generate_repaired_polygon(const int e, bit_vector &seed_list)
    {   
        int e_init = e;
        //search next frontier-edge
        while(!frontier_edges[e_init]){
            e_init = mesh_input->CW_edge_to_vertex(e_init);
            seed_list[e_init] = false; 
            //seed_list[mesh_input->twin(e_init)] = false;
        }        
        //first frontier-edge is store to calculate the prev of next frontier-edfge
        int e_prev = e_init; 
        int v_init = mesh_input->origin(e_init);

        int e_curr = mesh_input->next(e_init);
        int v_curr = mesh_input->origin(e_curr);
        seed_list[e_curr] = false;

        //travel inside frontier-edges of polygon
        while(e_curr != e_init && v_curr != v_init){   
            while(!frontier_edges[e_curr])
            {
                e_curr = mesh_input->CW_edge_to_vertex(e_curr);
                seed_list[e_curr] = false;
          //      seed_list[mesh_input->twin(e_curr)] = false;
            } 

            //update next of previous frontier-edge
            mesh_output->set_next(e_prev, e_curr);  
            //update prev of current frontier-edge
            mesh_output->set_prev(e_curr, e_prev);

            //travel to next half-edge
            e_prev = e_curr;        
            e_curr = mesh_input->next(e_curr);
            v_curr = mesh_input->origin(e_curr);
            seed_list[e_curr] = false;
            //seed_list[mesh_input->twin(e_curr)] = false;
        }
        mesh_output->set_next(e_prev, e_init);
        mesh_output->set_prev(e_init, e_prev);
        return e_init;
    }
*/

    //Generate a polygon from a seed-edge and remove repeated seed from seed_list
    //POSIBLE BUG: el algoritmo no viaja por todos los halfedges dentro de un poligono, 
    //por lo que pueden haber semillas que no se borren y tener poligonos repetidos de output
    int generate_repaired_polygon(const int e, bit_vector &seed_list)
    {   
        int e_init = e;

        //search next frontier-edge
        while(!frontier_edges[e_init]){
            e_init = mesh_input->CW_edge_to_vertex(e_init);
            seed_list[e_init] = false; 
            //seed_list[mesh_input->twin(e_init)] = false;
        }   
        int e_curr = mesh_input->next(e_init);    
        seed_list[e_curr] = false;
    
        int e_fe = e_init; 

        //travel inside frontier-edges of polygon
        do{   
            while(!frontier_edges[e_curr])
            {
                e_curr = mesh_input->CW_edge_to_vertex(e_curr);
                seed_list[e_curr] = false;
          //      seed_list[mesh_input->twin(e_curr)] = false;
            } 
            //update next of previous frontier-edge
            mesh_output->set_next(e_fe, e_curr);  
            //update prev of current frontier-edge
            mesh_output->set_prev(e_curr, e_fe);

            // int v_curr = mesh_input->target(e_fe);
            // int e_incident = mesh_input->twin(e_fe);
            // std::cout << "repairing "<< v_curr << std::endl;
            // if (v_curr == 8) {
            //     std::cout << "v " << v_curr << "e " <<e_incident << std::endl;
            // }
            // mesh_output->set_incident_halfedge(v_curr, e_incident);

            //travel to next half-edge
            e_fe = e_curr;
            e_curr = mesh_input->next(e_curr);
            seed_list[e_curr] = false;

        }while(e_fe != e_init);
        return e_init;
    }

    double area(int v0, int v1, int v2) {
        double area =   (mesh_output->get_PointX(v1) - mesh_output->get_PointX(v0)) * 
                        (mesh_output->get_PointY(v2) - mesh_output->get_PointY(v0)) - 
                        (mesh_output->get_PointY(v1) - mesh_output->get_PointY(v0)) * 
                        (mesh_output->get_PointX(v2) - mesh_output->get_PointX(v0));
        return area;
    }

    bool is_left(int v0, int v1, int p) {
        return area(v0, v1, p) > 0;
    }

    bool parallel(int e1, int e2) {
        auto v0 = mesh_output->origin(e1);
        auto v1 = mesh_output->target(e1);
        auto v2 = mesh_output->origin(e2);
        auto v3 = mesh_output->target(e2);
        auto v0_x = mesh_output->get_PointX(v0);
        auto v0_y = mesh_output->get_PointY(v0);
        auto v1_x = mesh_output->get_PointX(v1);
        auto v1_y = mesh_output->get_PointY(v1);
        auto v2_x = mesh_output->get_PointX(v2);
        auto v2_y = mesh_output->get_PointY(v2);
        auto v3_x = mesh_output->get_PointX(v3);
        auto v3_y = mesh_output->get_PointY(v3);
        auto den = (v0_x - v1_x)*(v2_y - v3_y) - (v0_y - v1_y)*(v2_x - v3_x);
        return std::abs(den) < EPSILON;
    }

    bool is_collinear(int v0, int v1, int v2) {
        double this_area = area(v0, v1, v2);
        return std::abs(this_area) < EPSILON;
    }

    bool in_range(int p, int v0, int v1) {
        auto p_x = mesh_output->get_PointX(p);
        auto p_y = mesh_output->get_PointY(p);
        auto v0_x = mesh_output->get_PointX(v0);
        auto v0_y = mesh_output->get_PointY(v0);
        auto v1_x = mesh_output->get_PointX(v1);
        auto v1_y = mesh_output->get_PointY(v1);
        return  std::min(v0_x, v1_x) < p_x && p_x < std::max(v0_x, v1_x) && 
                std::min(v0_y, v1_y) < p_y && p_y < std::max(v0_y, v1_y);
    }

    bool intersection(int e1, int e2) {
        auto v0 = mesh_output->origin(e1);
        auto v1 = mesh_output->target(e1);
        auto v2 = mesh_output->origin(e2);
        auto v3 = mesh_output->target(e2);
        return is_left(v0, v1, v2) != is_left(v0, v1, v3) && is_left(v2, v3, v0) != is_left(v2, v3, v1);
    }

    // Check if a vertex is on a region boundary (should not be moved during smoothing)
    // A vertex is on region boundary if any adjacent edge connects faces with different region IDs
    // or if vertex is on external mesh boundary
    // Returns false if regions are disabled or vertex has no valid incident edge
    bool is_region_boundary_vertex(int v) {
        if (!options.use_regions) return false;
        
        auto e_init = mesh_output->edge_of_vertex(v);
        if (e_init < 0) return false;
        
        // Border vertices are always region boundaries
        if (mesh_output->is_border_vertex(v)) return true;
        
        auto e_next = e_init;
        do {
            auto twin = mesh_output->twin(e_next);
            if (twin >= 0) {
                auto face1 = mesh_output->index_face(e_next);
                auto face2 = mesh_output->index_face(twin);

                // Check if faces have different regions
                if (face1 >= 0 && face2 >= 0) {
                    if (mesh_output->region_face(face1) != mesh_output->region_face(face2)) {
                        return true; // Different regions - early exit
                    }
                }
            }
            e_next = mesh_output->CCW_edge_to_vertex(e_next);
        } while (e_next != e_init);
        
        return false;
    }

    // Pre-compute region boundary vertices for optimization during smoothing
    // Only call this when use_regions is true and before starting smoothing iterations
    void compute_region_boundary_cache(std::vector<bool>& cache) {
        if (!options.use_regions) return;
        
        cache.resize(mesh_output->vertices(), false);
        for(std::size_t v = 0; v < mesh_output->vertices(); v++) {
            cache[v] = is_region_boundary_vertex(v);
        }
    }

    bool is_valid_move(int v) {
        auto e_init = mesh_output->edge_of_vertex(v);
        auto e_next = e_init;
        do {
            auto first_edge = e_next;
            auto last_edge = mesh_output->prev(first_edge);
            auto curr_edge = last_edge;
            do {
                auto e_init_2 = mesh_output->next(curr_edge);
                auto e_next_2 = e_init_2;
                do {
                    // std::cout <<"e1:" << e_next_1 << ", e2:" << e_next_2<<std::endl;
                    auto v0 = mesh_output->origin(curr_edge);
                    auto v1 = mesh_output->target(curr_edge);
                    auto v2 = mesh_output->origin(e_next_2);
                    auto v3 = mesh_output->target(e_next_2);
                    auto v0_x = mesh_output->get_PointX(v0);
                    auto v0_y = mesh_output->get_PointY(v0);
                    auto v1_x = mesh_output->get_PointX(v1);
                    auto v1_y = mesh_output->get_PointY(v1);
                    auto v2_x = mesh_output->get_PointX(v2);
                    auto v2_y = mesh_output->get_PointY(v2);
                    auto v3_x = mesh_output->get_PointX(v3);
                    auto v3_y = mesh_output->get_PointY(v3);
                    // if (v0 == 166 && v1 == 183 && v2 ==182 && v3 == 165) {
                    //     std::cout << "HERE" << std::endl;
                    // }
                    if (curr_edge == e_next_2 || v3 == v0) {
                        e_next_2 = mesh_output->next(e_next_2);
                        continue;
                    }
                    if (parallel(curr_edge, e_next_2)) {
                        if (is_collinear(v0, v1, v3)) {
                            if (v1 == v2) { // adjacent
                                if (in_range(v3, v0, v1) || in_range(v0, v2, v3)) {
                                    // std::cout << "here0"<<std::endl;
                                    return false;
                                }
                            } else {
                                if (in_range(v2, v0, v1) || in_range(v3, v0, v1) ||
                                    in_range(v0, v2, v3) || in_range(v1, v2, v3)) {
                                    return false;
                                }
                            }
                        }
                    } else { // not parallel
                        if (v1 != v2 &&  // not adjacent
                            intersection(curr_edge, e_next_2)) {
                                // if (v0 == 166 && v1 == 183 && v2 ==182 && v3 == 165) {
                                // }
                                return false;
                            }
                    }
                    e_next_2 = mesh_output->next(e_next_2);
                } while (e_init_2 != e_next_2);
                curr_edge = mesh_output->next(curr_edge);
            } while (curr_edge != mesh_output->next(first_edge));
        e_next = mesh_output->CCW_edge_to_vertex(e_next);
        } while (e_init != e_next);
        return true;
    }

    void optimize_mesh_laplacian(int max_iterations) {
        double first_movement = -1;
        
        // Pre-compute region boundary vertices for optimization
        std::vector<bool> region_boundary_cache;
        compute_region_boundary_cache(region_boundary_cache);
        
        for (int i = 0; i < max_iterations; i++) {
            n_smooth_iterations++;
            double movement = 0;
            for(std::size_t v = 0; v < mesh_output->vertices(); v++){
                // std::cout << "curr: "<< v << ", "<< mesh_output->get_PointX(v) << std::endl;
                if (mesh_output->is_border_vertex(v) || mesh_output->edge_of_vertex(v) < 0) continue;   

                // If using regions, skip vertices on region boundaries to preserve topology
                if (options.use_regions && region_boundary_cache[v]) continue;
                auto e_init = mesh_output->edge_of_vertex(v);
                // std::cout << "v"<<v <<std::endl;
                // std::cout << e_init <<std::endl;
                auto e_next = e_init;
                int n = 0;
                double x = 0;
                double y = 0;
                // bool border_present = false;
                do {
                    auto v_next = mesh_output->target(e_next);
                    // if (mesh_output->is_border_vertex(v_next)) border_present = true;
                    x += mesh_output->get_PointX(v_next) - mesh_output->get_PointX(v);
                    y += mesh_output->get_PointY(v_next) - mesh_output->get_PointY(v);
                    n++;
                    e_next = mesh_output->CCW_edge_to_vertex(e_next);
                } while (e_next != e_init);
                auto original_x = mesh_output->get_PointX(v);
                auto original_y = mesh_output->get_PointY(v);
                mesh_output->set_PointX(v, mesh_output->get_PointX(v) + x/n);
                mesh_output->set_PointY(v, mesh_output->get_PointY(v) + y/n);
                // if (!is_valid_move(v)) {
                //     mesh_output->set_PointX(v, original_x);
                //     mesh_output->set_PointY(v, original_y);
                // }
                if (first_movement == -1) first_movement = std::abs(x/n) + std::abs(y/n);
                movement = movement + std::abs(x/n) + std::abs(y/n);
            }
            if (std::abs(movement) < first_movement * 0.0001) {
                break;
            }
        }
    }

    void optimize_mesh_laplacian_constrained(int iterations, std::string measure_type) {
        Measure* measure = nullptr;
        if (measure_type == "laplacian-edge-ratio") {
            measure = new EdgeRatio(mesh_output, output_seeds);
        } else {
            std::cerr << "Warning: Unknown measure type '" << measure_type << "'. Skipping optimization." << std::endl;
            return;
        }
        
        // Pre-compute region boundary vertices for optimization
        std::vector<bool> region_boundary_cache;
        compute_region_boundary_cache(region_boundary_cache);
        
        for (int i = 0; i<iterations; i++) {
            n_smooth_iterations++;
            for(std::size_t v = 0; v < mesh_output->vertices(); v++){
                if (mesh_output->is_border_vertex(v) || mesh_output->edge_of_vertex(v) < 0) continue;   

                // If using regions, skip vertices on region boundaries to preserve topology
                if (options.use_regions && region_boundary_cache[v]) continue;
                auto e_init = mesh_output->edge_of_vertex(v);
                auto e_next = e_init;
                int n = 0;
                double x = 0;
                double y = 0;
                do {
                    auto v_next = mesh_output->target(e_next);
                    x += mesh_output->get_PointX(v_next) - mesh_output->get_PointX(v);
                    y += mesh_output->get_PointY(v_next) - mesh_output->get_PointY(v);
                    n++;
                    e_next = mesh_output->CCW_edge_to_vertex(e_next);
                } while (e_next != e_init);

                // original measures
                double original_x = mesh_output->get_PointX(v);
                double original_y = mesh_output->get_PointY(v);
                double original_sum = 0;
                int adjacent_faces = 0;
                e_init = mesh_output->edge_of_vertex(v);
                e_next = e_init;
                do {
                    // double face_res = measure(mesh_output, e_next);
                    double face_res = measure->eval_face(e_next);
                    adjacent_faces++;
                    original_sum += face_res;
                    e_next = mesh_output->CCW_edge_to_vertex(e_next);
                } while (e_next != e_init);
                double original_avg = original_sum / adjacent_faces;

                // move vertex
                mesh_output->set_PointX(v, mesh_output->get_PointX(v) + x/n);
                mesh_output->set_PointY(v, mesh_output->get_PointY(v) + y/n);

                // new measures
                double new_sum = 0;
                e_next = e_init;
                do {
                    // double face_res = measure(mesh_output, e_next);
                    double face_res = measure->eval_face(e_next);
                    new_sum += face_res;
                    e_next = mesh_output->CCW_edge_to_vertex(e_next);
                } while (e_next != e_init);
                double new_avg = new_sum / adjacent_faces;

                // if worse measure undo move
                if (measure->is_better(original_avg, new_avg) || !is_valid_move(v)) {
                    mesh_output->set_PointX(v, original_x);
                    mesh_output->set_PointY(v, original_y);
                }
            }
        }
        delete measure;
    }
    
    void optimize_mesh_distmesh(int max_iterations, double target_length) {
        double first_movement = -1;
        if (target_length == -1) {
            double sum = 0;
            for(std::size_t e = 0; e < mesh_output->halfEdges(); e++) {
                auto target = mesh_output->target(e);
                auto origin = mesh_output->origin(e);
                auto x = mesh_output->get_PointX(target) - mesh_output->get_PointX(origin);
                auto y = mesh_output->get_PointY(target) - mesh_output->get_PointY(origin);
                auto length = std::sqrt(x*x + y*y);
                sum += length;
            }
            target_length = sum/mesh_output->halfEdges();
        }
        
        // Pre-compute region boundary vertices for optimization
        std::vector<bool> region_boundary_cache;
        compute_region_boundary_cache(region_boundary_cache);
        
        // std::cout << target_length << std::endl;
        for (int i = 0; i < max_iterations; i++) {
            n_smooth_iterations++;
            double movement = 0;

            for(std::size_t v = 0; v < mesh_output->vertices(); v++){
                if (mesh_output->is_border_vertex(v) || mesh_output->edge_of_vertex(v) < 0) continue;

                // If using regions, skip vertices on region boundaries to preserve topology   
                if (options.use_regions && region_boundary_cache[v]) continue;
                auto e_init = mesh_output->edge_of_vertex(v);
                auto e_next = e_init;
                double origin_x = mesh_output->get_PointX(v);
                double origin_y = mesh_output->get_PointY(v);
                double x = 0;
                double y = 0;
                // std::cout << "v: " << v << std::endl;
                do {
                    auto v_next = mesh_output->target(e_next);
                    auto d_x = mesh_output->get_PointX(v_next) - mesh_output->get_PointX(v);
                    auto d_y = mesh_output->get_PointY(v_next) - mesh_output->get_PointY(v);
                    double length = std::sqrt(d_x*d_x + d_y*d_y);
                    if (target_length > length) {
                        e_next = mesh_output->CCW_edge_to_vertex(e_next);
                        continue;
                    }
                    double force = target_length - length;
                    double target_x = mesh_output->get_PointX(v_next);
                    double target_y = mesh_output->get_PointY(v_next);
                    double direction_x = (target_x - origin_x)/length;
                    double direction_y = (target_y - origin_y)/length;

                    x += direction_x * -force;
                    y += direction_y * -force;
                    // std::cout << target_length << ", " << length << ", " << force << "; " << direction_x * force << ", " << direction_y * force << std::endl;

                    e_next = mesh_output->CCW_edge_to_vertex(e_next);
                } while (e_next != e_init);

                // std::cout << "e: " << e_next << std::endl;
                // is_valid_move(e_next);
                // std::cout << mesh_output->get_PointX(v) <<std::endl;
                // std::cout << mesh_output->get_PointY(v) <<std::endl;
                mesh_output->set_PointX(v, mesh_output->get_PointX(v) + x * 0.5);
                mesh_output->set_PointY(v, mesh_output->get_PointY(v) + y * 0.5);
                if (!is_valid_move(v)) {
                    // std::cout << "reversing"<<std::endl;
                    // std::cout << mesh_output->get_PointX(v) <<std::endl;
                    // std::cout << mesh_output->get_PointY(v) <<std::endl;
                    mesh_output->set_PointX(v, origin_x);
                    mesh_output->set_PointY(v, origin_y);
                    // std::cout << "reversed"<<std::endl;
                    // std::cout << mesh_output->get_PointX(v) <<std::endl;
                    // std::cout << mesh_output->get_PointY(v) <<std::endl;
                }
                
                if (first_movement == -1) first_movement = std::abs(x) + std::abs(y);
                movement = movement + std::abs(x) + std::abs(y);
            }

            if (std::abs(movement) < first_movement * 0.0001) {
                break;
            }
        }
    }
};

#endif
