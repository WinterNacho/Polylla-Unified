# Polylla: Polygonal meshing algorithm based on terminal-edge regions

<p align="center">
 <img src="https://github.com/ssalinasfe/Polylla-Mesh-DCEL/blob/main/images/polyllalogo2.png" width="80%">
</p>
New algorithm to generate polygonal meshes of arbitrary shape, using any kind of triangulation as input, adaptable to any kind of complex geometry, no addition of extra points and uses the classic Doubly connected edge list (Half edge data struct) easy to implement wih another programming language.

<p align="center">
<img src="https://github.com/ssalinasfe/Polylla-Mesh-DCEL/blob/main/images/pikachu_A500000_T531_V352.png" width="50%">
</p>

The algorithm needs a initial triangulation as input, any triangulations will work, in the following Figure the example of a Planar Straigh Line Graph (PSLG) with holes (left image), triangulizated (middle image) to generate a Polylla mesh (right image).

<p align="center">
 <img src="https://github.com/ssalinasfe/Polylla-Mesh-DCEL/blob/main/images/faceoriginalPSLG.png" width="30%">
 <img src="https://github.com/ssalinasfe/Polylla-Mesh-DCEL/blob/main/images/facewithtrianglesblack.png" width="30%">
 <img src="https://github.com/ssalinasfe/Polylla-Mesh-DCEL/blob/main/images/final.png" width="30%">
</p>

<p align="center">
 <img src="https://github.com/ssalinasfe/Polylla-Mesh-DCEL/blob/main/images/pikachu PLSG.png" width="30%">
 <img src="https://github.com/ssalinasfe/Polylla-Mesh-DCEL/blob/main/images/pikachutriangulization.png" width="30%">
 <img src="https://github.com/ssalinasfe/Polylla-Mesh-DCEL/blob/main/images/pikachuPolylla.png" width="30%">
</p>

## Usage

The algorithm uses a command-line interface with getopt standard options:

```
Usage: ./Polylla [OPTIONS] [FILES...]

Input modes:
  -o, --off            Use OFF file as input
  -n, --neigh          Use .node, .ele, and .neigh files as input
  -e, --ele            Use .node and .ele files as input (without .neigh)

Options:
  -g, --gpu            Enable GPU acceleration (requires CUDA)
  -r, --region         Read and process triangulation considering regions
  -s, --smooth METHOD  Use smoothing method: laplacian, laplacian-edge-ratio, distmesh
  -i, --iterations N   Number of smoothing iterations (default: 50)
  -t, --target-length N Target edge length for distmesh method
  -O, --output FORMAT  Specify output format: off (default)
  -h, --help           Show this help message
```

## Input/Output formats

The algorithm supports multiple input formats and generates `.off` files and `.json` statistics.

### Input modes

#### 1. OFF file input

Use a triangulated mesh in [OFF format](<https://en.wikipedia.org/wiki/OFF_(file_format)>):

```bash
./Polylla --off input.off
```

#### 2. Triangle files input (with .neigh)

Use [Triangle](https://www.cs.cmu.edu/~quake/triangle.html) generated files including adjacency information:

```bash
./Polylla --neigh mesh.node mesh.ele mesh.neigh
```

#### 3. Triangle files input (without .neigh)

Use Triangle files without adjacency information (adjacencies computed internally):

```bash
./Polylla --ele mesh.node mesh.ele
```

### Examples

Generate pikachu mesh from Triangle files:

```bash
./Polylla --neigh pikachu.1.node pikachu.1.ele pikachu.1.neigh
```

Enable GPU acceleration:

```bash
./Polylla --off --gpu input.off
```

Apply mesh smoothing:

```bash
# Laplacian smoothing with 100 iterations
./Polylla --off --smooth laplacian --iterations 100 input.off

# Edge-ratio constrained smoothing
./Polylla --off --smooth laplacian-edge-ratio --iterations 50 input.off

# DistMesh-style smoothing with target edge length
./Polylla --off --smooth distmesh --target-length 0.1 --iterations 75 input.off
```

Combine options:

```bash
# Use regions with GPU and smoothing
./Polylla --neigh --region --gpu --smooth laplacian --iterations 50 mesh.node mesh.ele mesh.neigh
```

### Smoothing Methods

The algorithm supports three mesh smoothing methods that can be applied before polygon generation:

- **laplacian**: Classic Laplacian smoothing for vertex positions
- **laplacian-edge-ratio**: Laplacian smoothing with edge ratio constraints to preserve mesh quality
- **distmesh**: DistMesh-style smoothing with target edge length control

**Notes:**

- Smoothing is applied **before** polygon generation to improve the quality of the input triangulation
- When `--region` is enabled, smoothing preserves region boundaries
- For `distmesh` method, use `--target-length` to specify desired edge length (auto-calculated if not provided)

### Output files

The algorithm automatically generates:

- **mesh_name.off**: Polygonal mesh in OFF format
- **mesh_name.json**: Statistics and timing information

## Shape of polygons

Note shape of the polygon depend on the initital triangulation, in the folowing Figure there is a example of a disk generate with a Delaunay Triangulation with random points (left image) vs a refined Delaunay triangulation with semi uniform points (right image).

<p align="center">
 <img src="https://github.com/ssalinasfe/Polylla-Mesh-DCEL/blob/main/images/2x2RPDisk_3000_poly_1000.png" width="40%" hspace="10px">
 <img src="https://github.com/ssalinasfe/Polylla-Mesh-DCEL/blob/main/images/disk2x2_1574_poly1012.png" width="40%">
</p>

## Scripts

Scripts made to facilizate the process of test the algorithm:

- (in build folder) To generate random points, an initital triangulation and a poylla mesh

  ```
  ./generatemesh.sh <number of vertices of triangulation>
  ```

- (in build folder) To generate mesh from files .node, .ele, .neigh with the same name

```
./generatefromfile.sh <filename> <output name>
```

```
 ./generatefromfile.sh pikachu.1 out
```

Triangulazitation are generated with [triangle](https://www.cs.cmu.edu/~quake/triangle.html) with the [command -zn](https://www.cs.cmu.edu/~quake/triangle.switch.html).

## TODO

### TODO scripts

- [ ] Line 45 of plotting depends on a transpose, store edges directly as the transpose of edge vectors and remove it.
- [ ] Define an input and output folder scripts
- [ ] Define -n in plot_triangulation.py to avoid label edges and vertices
- [ ] Change name plot_triangulation.py to plot_mesh.py

### TODO Poylla

- [ ] Travel phase does not work with over big meshes (10^7)
- [ ] Add high float point precision edge lenght comparision
- [ ] POSIBLE BUG: el algoritmo no viaja por todos los halfedges dentro de un poligono en la travel phase, por lo que pueden haber semillas que no se borren y tener poligonos repetidos de output
- [ ] Add arbitrary precision arithmetic in the label phase
- [ ] Add frontier-edge addition to constrained segmend and refinement (agregar método que dividida un polygono dado una arista especifica)
- [x] hacer la función distance parte de cada halfedge y cambiar el ciclo por 3 comparaciones.
- [x] Add way to store polygons.
- [ ] iterador de polygono
- [x] Vector con los poligonos de malla
- [ ] Método para imprimir SVG
- [ ] Copy constructor
- [ ] half-edge constructor
- [x] Change by triangle bitvector by triangle list
- [x] Remove distance edge

### TODO Halfedges

- [ ] edge_iterator;
- [ ] face_iterator;
- [ ] vertex_iterator;
- [ ] copy constructor;
- [x] constructor indepent of triangle (any off file now works)
- [x] default constructor
- [ ] definir mejor cuáles variables son unsigned int y cuáles no
- [x] Change by triangle bitvector by triangle list
- [ ] Calculate distante edge
- [ ] Read node files with commentaries

### TODO C++

- [x] change to std::size_t to int
- [x] change operator [] by .at()
- [x] add #ifndef ALL_H_FILES #define ALL_H_FILES #endif to being and end header
- [ ] add google tests
- [ ] Add google benchmark

### TODO github

- [ ] Add how generate mesh from OFF file
- [x] Add images that show how the initial trangulization changes the output
- [x] Add the triangulation of the disks
- [x] Hacer el readme más explicativo
- [ ] Add example meshes
- [x] Add .gitignore
- [ ] Poner en inglés uwu
