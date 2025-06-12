import sys
import os
from math import atan2, degrees, dist, isclose, pi
from utils import kernel_poly, angle_between

def read_OFF(filename):
    vertices = []
    regions = []
    boundary = []
    with open(filename) as fp:
        # Read number of vertices
        for line in fp:
            if line.startswith('#'):
                continue
            l = line.split()
            if l[0] == 'OFF':
                break
            else:
                sys.exit("Error: File is not an OFF file")
        for line in fp:
            if line.startswith('#'):
                continue
            l = line.split()
            if l[0].isdigit():
                n_vertices = int(l[0])
                n_regions = int(l[1])
                break
        # read vertices
        for i in range(0, n_vertices):
            line = fp.readline()
            l = line.split()
            vertices.append((float(l[0]), float(l[1])))
        # read regions
        for i in range(0, n_regions):
            line = fp.readline()
            l = line.split()
            l.pop(0)
            regions.append([int(x) for x in l])

    return vertices, regions

def calc_edges_per_polygon(regions):
    edges = 0
    for i in regions:
        edges += len(i)
    return edges / len(regions)

# #https://stackoverflow.com/questions/58953047/issue-with-finding-angle-between-3-points-in-python
# def angle_between(p1, p2, p3):
#     x1, y1 = p1
#     x2, y2 = p2
#     x3, y3 = p3
#     deg1 = (360 + degrees(atan2(x1 - x2, y1 - y2))) % 360
#     deg2 = (360 + degrees(atan2(x3 - x2, y3 - y2))) % 360
#     return deg2 - deg1 if deg1 <= deg2 else 360 - (deg1 - deg2)


def min_max_angle_polygon(poly, vertices):
    min_angle = 360
    max_angle = 0
    for i in range(0, len(poly)):   
        p1 = vertices[poly[i]]
        p2 = vertices[poly[(i + 1) % len(poly)]]
        p3 = vertices[poly[(i + 2) % len(poly)]]
        angle = angle_between(p1, p2, p3)
        if angle < min_angle:
            min_angle = angle
        if angle > max_angle:
            max_angle = angle
    return min_angle, max_angle

def min_max_angle_mesh(regions, vertices):
    min_angle = 360
    max_angle = 0
    for i in regions:
        min_angle_poly, max_angle_poly = min_max_angle_polygon(i, vertices)
        if min_angle_poly < min_angle:
            min_angle = min_angle_poly
        if max_angle_poly > max_angle:
            max_angle = max_angle_poly
    return min_angle, max_angle

def min_max_edge_polygon(poly, vertices):
    min_edge = -1.0
    max_edge = -1.0
    for i in range(0, len(poly)):
        p1 = vertices[poly[i]]
        p2 = vertices[poly[(i + 1) % len(poly)]]
        edge_len = dist(p1, p2)
        if min_edge == -1.0 or edge_len < min_edge:
            min_edge = edge_len
        if max_edge == -1.0 or edge_len > max_edge:
            max_edge = edge_len
    return min_edge, max_edge

def min_max_edge_ratio_mesh(regions, vertices):
    min_aspect_ratio = -1.0
    max_aspect_ratio = -1.0
    for i in regions:
        min_edge_poly, max_edge_poly = min_max_edge_polygon(i, vertices)
        aspect_ratio = min_edge_poly / max_edge_poly
        if min_aspect_ratio == -1.0 or aspect_ratio < min_aspect_ratio:
            min_aspect_ratio = aspect_ratio
        if max_aspect_ratio == -1.0 or aspect_ratio > max_aspect_ratio:
            max_aspect_ratio = aspect_ratio
    return min_aspect_ratio, max_aspect_ratio

def avg_edge_ratio_mesh(regions, vertices):
    sum = 0
    for i in regions:
        min_edge_poly, max_edge_poly = min_max_edge_polygon(i, vertices)
        aspect_ratio = min_edge_poly / max_edge_poly
        sum += aspect_ratio
    return sum / len(regions)

def perimeter_poly(poly, vertices):
    total = 0
    for i in range(len(poly)):
        v0 = vertices[poly[i]]
        v1 = vertices[poly[(i + 1) % len(poly)]]
        total += dist(v0, v1)
    return total

def area_poly(poly, vertices):
    if len(poly) == 0:
        return 0
    total = 0
    p = vertices[poly[0]]
    for i in range(1, len(poly)):
        v0 = vertices[poly[i]]
        v1 = vertices[poly[(i + 1)%len(poly)]]
        total += (v0[0] - p[0])*(v1[1] - p[1]) - (v0[1] - p[1])*(v1[0] - p[0])
    return abs(total/2)

def area_ratio_kernel_polygon(poly, vertices):
    try:
        kernel_v = kernel_poly(poly, vertices)
    except(ValueError):
        return 0
    kernel_p = [i for i in range(len(kernel_v))]
    area_polygon = area_poly(poly, vertices)
    area_kernel = area_poly(kernel_p, kernel_v)
    if area_polygon == 0:
        return 0
    return area_kernel/area_polygon

def min_max_area_ratio_kernel_mesh(regions, vertices):
    min_ratio = -1
    max_ratio = -1
    for poly in regions:
        area_ratio = area_ratio_kernel_polygon(poly, vertices)
        if area_ratio == 0:
            print(poly)
        if min_ratio == -1 or area_ratio < min_ratio:
            min_ratio = area_ratio
        if max_ratio == -1 or area_ratio > max_ratio:
            max_ratio = area_ratio
    return min_ratio, max_ratio

def avg_area_ratio_kernel_mesh(regions, vertices):
    total_sum = 0
    for poly in regions:
        total_sum += area_ratio_kernel_polygon(poly, vertices)
    return total_sum/len(regions)

def apr_poly(poly, vertices):
    area = area_poly(poly, vertices)
    perimeter = perimeter_poly(poly, vertices)
    apr = (2 * pi * area)/(perimeter**2)
    return apr

def min_max_apr_mesh(regions, vertices):
    min_apr = -1
    max_apr = -1
    for poly in regions:
        apr = apr_poly(poly, vertices)
        if min_apr == -1 or apr < min_apr:
            min_apr = apr
        if max_apr == -1 or apr > max_apr:
            max_apr = apr
    return min_apr, max_apr

def avg_apr_mesh(regions, vertices):
    apr_sum = 0
    for poly in regions:
        apr_sum += apr_poly(poly, vertices)
    return apr_sum/len(regions)

if __name__ == "__main__":
    filename = sys.argv[1]
    vertices, regions = read_OFF(filename)
    # print(vertices)
    # print(regions)
    
    print("Edges per polygon:", calc_edges_per_polygon(regions))
    
    assert(angle_between((0,0), (0,1), (0,2)) == 180.0)
    assert(angle_between((0,0), (0,0), (0,0)) == 0.0)
    assert(angle_between((-1,0), (0,0), (0,1)) == 90.0)

    #square max min angle
    assert(min_max_angle_polygon([0, 1, 2, 3], [(0,0), (1,0), (1,1), (0,1)]) == (90.0, 90.0))

    #two triangle mesh in a square
    assert(min_max_angle_mesh([[0, 1, 3], [1, 2, 3]], [(0,0), (1,0), (1,1), (0,1)]) == (45.0, 90.0))

    #perimeter
    assert(perimeter_poly([0, 1, 2, 3], [(0, 0), (1, 0), (1, 1), (0, 1)]) == 4)
    assert(isclose(perimeter_poly([0, 1, 2, 3, 4], [(0, 0), (2, 1), (2, 2), (0, 3), (-1, -1)]), 11.009455143, rel_tol=1e-6))

    #area
    assert(area_poly([0, 1, 2, 3], [(0, 0), (1, 0), (1, 1), (0, 1)]) == 1)
    assert(area_poly([0, 1, 2, 3], [(0, 0), (2, 0), (2, 2), (0, 2)]) == 4)
    assert(area_poly([0, 1, 2, 3, 4, 5], [(0, 0), (1, 0), (1, 1), (2, 1), (2, 2), (0, 2)]) == 3)
    assert(area_poly([0, 1, 2, 3, 4, 5], [(1, 1), (2, 1), (2, 2), (3, 2), (3, 3), (1, 3)]) == 3)
    assert(area_poly([0, 1, 2], [(1, 1), (3, 1), (2, 3)]) == 2)

    #kernel polygon area ratio
    assert(area_ratio_kernel_polygon([0, 1, 2, 3, 4, 5], [(0, 0), (1, 0), (1, 1), (2, 1), (2, 2), (0, 2)]) == 1/3)
    assert(area_ratio_kernel_polygon([0, 1, 2, 3], [(0, 0), (2, 0), (2, 2), (0, 2)]) == 1)

    min_angle, max_angle = min_max_angle_mesh(regions, vertices)
    print("Min angle:", min_angle)
    print("Max angle:", max_angle)

    min_edge_ratio, max_edge_ratio = min_max_edge_ratio_mesh(regions, vertices)
    avg_edge_ratio = avg_edge_ratio_mesh(regions, vertices)
    print("Min edge ratio:", min_edge_ratio)
    print("Max edge ratio:", max_edge_ratio)
    print("Avg edge ratio:", avg_edge_ratio)

    min_area_ratio_kernel_poly, max_area_ratio_kernel_poly = min_max_area_ratio_kernel_mesh(regions, vertices)
    avg_area_ratio_kernel_poly = avg_area_ratio_kernel_mesh(regions, vertices)
    print("Min area ratio kernel poly:", min_area_ratio_kernel_poly)
    print("Max area ratio kernel poly:", max_area_ratio_kernel_poly)
    print("Avg area ratio kernel poly:", avg_area_ratio_kernel_poly)

    min_apr, max_apr = min_max_apr_mesh(regions, vertices)
    avg_apr = avg_apr_mesh(regions, vertices)
    print("Min apr:", min_apr)
    print("Max apr:", max_apr)
    print("Avg apr:", avg_apr)

    if len(sys.argv) == 3:
        with open(sys.argv[2], "a") as f:
            data = [min_angle, max_angle, min_edge_ratio, max_edge_ratio, 
                    avg_edge_ratio, min_area_ratio_kernel_poly, 
                    max_area_ratio_kernel_poly, avg_area_ratio_kernel_poly,
                    min_apr, max_apr, avg_apr]
            f.write(os.path.basename(filename) + ",")
            f.write(",".join(map(lambda x: "{:.2f}".format(x), data)))
            f.write("\n")
            f.close()
