import sys
import os
from math import atan2, degrees, dist, isclose

tol = 1e-6

# edge data structure
class Edge:
    v1 = None
    v2 = None
    is_v1_fixed = None
    is_v2_fixed = None
    def __init__(self, v1=None, v2=None, is_v1_fixed=True, is_v2_fixed=True):
        self.v1 = v1
        self.v2 = v2
        self.is_v1_fixed = is_v1_fixed
        self.is_v2_fixed = is_v2_fixed
    
    # return 1 if to the left, 0 if colinear, -1 if to the right
    def is_left(self, point):
        p_x = point[0]
        p_y = point[1]
        v1_x = self.v1[0]
        v1_y = self.v1[1]
        v2_x = self.v2[0]
        v2_y = self.v2[1]
        return (v2_x - v1_x) * (p_y - v1_y) - (v2_y - v1_y) * (p_x - v1_x)

    def intersection(self, edge):
        x1 = self.v1[0]
        y1 = self.v1[1]
        x2 = self.v2[0]
        y2 = self.v2[1]
        x3 = edge.v1[0]
        y3 = edge.v1[1]
        x4 = edge.v2[0]
        y4 = edge.v2[1]
        p_x = ((x1*y2 - y1*x2)*(x3 - x4) - (x1 - x2)*(x3*y4 - y3*x4))/((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4))
        p_y = ((x1*y2 - y1*x2)*(y3 - y4) - (y1 - y2)*(x3*y4 - y3*x4))/((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4))
        p = (p_x, p_y)
        return p if self.in_range(p) and edge.in_range(p) else None
    
    def in_range_x(self, p):
        x1 = self.v1[0]
        x2 = self.v2[0]
        px = p[0]
        if not self.is_v1_fixed and not self.is_v2_fixed:
            return True
        elif not self.is_v1_fixed:
            if x1 <= x2:
                return px <= x2
            else:
                return px >= x2
        elif not self.is_v2_fixed:
            if x2 <= x1:
                return px <= x1
            else:
                return px >= x1
        return min(x1, x2) <= px and px <= max(x1, x2)
    
    def in_range_y(self, p):
        y1 = self.v1[1]
        y2 = self.v2[1]
        py = p[1]
        if not self.is_v1_fixed and not self.is_v2_fixed:
            return True
        elif not self.is_v1_fixed:
            if y1 <= y2:
                return py <= y2
            else:
                return py >= y2
        elif not self.is_v2_fixed:
            if y2 <= y1:
                return py <= y1
            else:
                return py >= y1
        return min(y1, y2) <= py and py <= max(y1, y2)

    def in_range(self, p):
        return self.in_range_x(p) and self.in_range_y(p)

# return 1 if to the left, 0 if colinear, -1 if to the right
def is_left(v1, v2, p):
    p_x = p[0]
    p_y = p[1]
    v1_x = v1[0]
    v1_y = v1[1]
    v2_x = v2[0]
    v2_y = v2[1]
    return (v2_x - v1_x) * (p_y - v1_y) - (v2_y - v1_y) * (p_x - v1_x)

def intersection(v1, v2, v3, v4):
    x1 = v1[0]
    y1 = v1[1]
    x2 = v2[0]
    y2 = v2[1]
    x3 = v3[0]
    y3 = v3[1]
    x4 = v4[0]
    y4 = v4[1]
    det = (x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4)
    if isclose(0, det, abs_tol=tol):
        return None
    p_x = ((x1*y2 - y1*x2)*(x3 - x4) - (x1 - x2)*(x3*y4 - y3*x4))/det
    p_y = ((x1*y2 - y1*x2)*(y3 - y4) - (y1 - y2)*(x3*y4 - y3*x4))/det
    if (min(x1, x2) <= p_x and p_x <= max(x1, x2) and min(y1, y2) <= p_y and p_y <= max(y1, y2) and
        min(x3, x4) <= p_x and p_x <= max(x3, x4) and min(y3, y4) <= p_y and p_y <= max(y3, y4)):
        return (p_x, p_y)
    return None

def inf_intersection(v1, v2, v3, v4):
    x1 = v1[0]
    y1 = v1[1]
    x2 = v2[0]
    y2 = v2[1]
    x3 = v3[0]
    y3 = v3[1]
    x4 = v4[0]
    y4 = v4[1]
    det = (x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4)
    if isclose(0, det, abs_tol=tol):
        return None
    p_x = ((x1*y2 - y1*x2)*(x3 - x4) - (x1 - x2)*(x3*y4 - y3*x4))/det
    p_y = ((x1*y2 - y1*x2)*(y3 - y4) - (y1 - y2)*(x3*y4 - y3*x4))/det
    return (p_x, p_y)

def is_on(v1, v2, p, inf_v1=False, inf_v2=False):
    x1 = v1[0]
    y1 = v1[1]
    x2 = v2[0]
    y2 = v2[1]
    px = p[0]
    py = p[1]
    # if (isclose(x1, px) and isclose(y1, py)) or (isclose(x2, px) and isclose(y2, py)):
    #     return False
    on_line = isclose(0, is_left(v1, v2, p), abs_tol=tol)
    if not inf_v1 and not inf_v2:
        return min(x1, x2) <= px and px <= max(x1, x2) and min(y1, y2) <= py and py <= max(y1, y2) and on_line
    elif not inf_v1 and inf_v2:
        if x1 < x2:
            if y1 < y2:
                return x1 <= px and y1 <= py and on_line
            else:
                return x1 <= px and py <= y1 and on_line
        else:
            if y1 < y2:
                return px <= x1 and y1 <= py and on_line
            else:
                return px <= x1 and py <= y1 and on_line
    elif inf_v1 and not inf_v2:
        if x1 < x2:
            if y1 < y2:
                return px <= x2 and py <= y2 and on_line
            else:
                return px <= x2 and y2 <= py and on_line
        else:
            if y1 < y2:
                return x2 <= px and py <= y2 and on_line
            else:
                return x2 <= px and y2 <= py and on_line
    else:
        return on_line

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

#https://stackoverflow.com/questions/58953047/issue-with-finding-angle-between-3-points-in-python
def angle_between(p1, p2, p3):
    x1, y1 = p1
    x2, y2 = p2
    x3, y3 = p3
    deg1 = (360 + degrees(atan2(x1 - x2, y1 - y2))) % 360
    deg2 = (360 + degrees(atan2(x3 - x2, y3 - y2))) % 360
    return deg2 - deg1 if deg1 <= deg2 else 360 - (deg1 - deg2)


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

def min_max_aspect_ratio_mesh(regions, vertices):
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

def avg_aspect_ratio_mesh(regions, vertices):
    sum = 0
    for i in regions:
        min_edge_poly, max_edge_poly = min_max_edge_polygon(i, vertices)
        aspect_ratio = min_edge_poly / max_edge_poly
        sum += aspect_ratio
    return sum / len(regions)

def inf_coord(v1, v2, along_v2=False):
    mod = dist(v1, v2)
    direction = ((v2[0] - v1[0])/mod, (v2[1] - v1[1])/mod)
    inf_mult = 100_000
    if along_v2:
        return (v1[0] + direction[0] * inf_mult, v1[1] + direction[1] * inf_mult)
    else:
        return (v1[0] - direction[0] * inf_mult, v1[1] - direction[1] * inf_mult)

def get_first_reflex(poly, vertices):
    stard_id = 0
    n = len(poly)
    while stard_id != n:
        angle = angle_between(vertices[poly[(stard_id - 1)%n]], vertices[poly[(stard_id)%n]], vertices[poly[(stard_id + 1)%n]])
        if angle > 183 + 1e-6:
            print("ANGLE: ",angle)
            for i in range(len(poly)):
                print(angle_between(vertices[poly[(i-1)%len(poly)]], vertices[poly[(i)%len(poly)]], vertices[poly[(i+1)%len(poly)]]))
            break
        stard_id += 1
    return stard_id


def kernel_poly(poly, vertices):
    start_id = get_first_reflex(poly, vertices)
    # print(poly)
    # print(start_id)
    print("Poly:", [vertices[i] for i in poly])
    print("IDS:", [i for i in poly])
    n = len(poly)
    if start_id == n: # convex
        return [vertices[i] for i in poly]
    v_F = inf_coord(vertices[poly[start_id]], vertices[poly[(start_id + 1)%n]])
    v_L = inf_coord(vertices[poly[(start_id - 1)%n]], vertices[poly[start_id]], True)
    k_vertices = [v_F, vertices[poly[start_id]], v_L]
    F_id = 0
    L_id = 2
    bounded = False
    for p_i in range(1, len(poly) - 1):
        k_vertices_new = []
        i = poly[(start_id + p_i)%n]
        # print(vertices)
        print("Current:", p_i)
        print("Start id:", start_id)
        print("ID:", i)
        print("F:", F_id)
        print("L:", L_id)
        print("K vertices:", k_vertices)
        v0 = vertices[poly[(start_id + p_i - 1)%n]]
        v1 = vertices[poly[(start_id + p_i)%n]]
        v2 = vertices[poly[(start_id + p_i + 1)%n]]
        # print(v0, v1, v2)
        inf_v1 = inf_coord(v1, v2)
        inf_v2 = inf_coord(v1, v2, True)
        angle = angle_between(v0, v1, v2)
        print("Current angle:", angle)

        if isclose(180.0, angle, abs_tol=tol): # colinear
            print("Continued")
            continue
        elif angle > 180: # reflex angle
            left = is_left(v1, v2, k_vertices[F_id%len(k_vertices)])
            if left < 0 or isclose(0, left, abs_tol=tol): # F to the right or on edge
                # scan K ccw from F
                F_curr_t = F_id + 1
                w_p = None
                w_t_id = None
                while F_curr_t != L_id + 1:
                    w_t_id = F_curr_t
                    w_t1 = k_vertices[(w_t_id - 1)%len(k_vertices)]
                    w_t2 = k_vertices[w_t_id%len(k_vertices)]
                    w_p = inf_intersection(v1, v2, w_t1, w_t2)
                    if w_p is not None and is_on(v1, v2, w_p, inf_v1=True) and is_on(w_t1, w_t2, w_p):
                        break
                    F_curr_t += 1
                if w_p is None: # kernel is empty
                    print("EMPTY 1")
                    return []
                # scan K cw from F
                F_curr_s = F_id
                w_p2 = None
                w_s_id = None
                limit = 0 if not bounded else F_id + 1
                while w_s_id != limit:
                    w_s_id = F_curr_s
                    w_s1 = k_vertices[(w_s_id - 1)%len(k_vertices)]
                    w_s2 = k_vertices[w_s_id%len(k_vertices)]
                    w_p2 = inf_intersection(v1, v2, w_s1, w_s2)
                    if w_p2 is not None and is_on(v1, v2, w_p2, inf_v1=True) and is_on(w_s1, w_s2, w_p2):
                        break
                    F_curr_s = (F_curr_s - 1)%len(k_vertices)
                if w_p2 is not None:
                    k_vertices_new = k_vertices[:w_s_id] + [w_p2, w_p] + k_vertices[w_t_id:]
                else:
                    inter = inf_intersection(v1, v2, k_vertices[1], k_vertices[0])
                    # if is_left(v2, inf_v1, head) > 0 and is_left(pre_tail, tail, inf_v1) > 0: # slope between head and tail
                    if inter is not None and is_on(v1, v2, inter, inf_v2=True) and is_on(k_vertices[1], k_vertices[0], inter, inf_v2=True):
                        print("Enter here 2")
                        w_r_id = len(k_vertices) - 1
                        while w_p2 is None:
                            w_r1 = k_vertices[(w_r_id - 1)%len(k_vertices)]
                            w_r2 = k_vertices[w_r_id%len(k_vertices)]
                            w_p2 = inf_intersection(v1, v2, w_r1, w_r2)
                            if w_p2 is not None and is_on(v1, v2, w_p2, inf_v1=True) and is_on(w_r1, w_r2, w_p2):
                                break
                            w_r_id -= 1
                        k_vertices_new = k_vertices[w_t_id:w_r_id] + [w_p2, w_p]
                        bounded = True
                    else: # slope between head and tail
                        print("Enter here 1", w_t_id)
                        k_vertices_new = [inf_v1, w_p] + k_vertices[w_t_id:]
                # selection of new F
                if w_p2 is None:
                    F_id = 0
                else:
                    F_id = k_vertices_new.index(w_p2)
            else: # F to the left of edge
                k_vertices_new = k_vertices
                # selection of new F
                # scan K ccw from F
                F_curr_t = F_id
                while True:
                    w_t_id = F_curr_t
                    w_t1 = k_vertices[w_t_id%len(k_vertices)]
                    w_t2 = k_vertices[(w_t_id + 1)%len(k_vertices)]
                    inf_w = inf_coord(v2, w_t1, True)
                    if is_left(v2, inf_w, w_t2) < 0:
                        # F_id = w_t_id
                        F_id = k_vertices_new.index(k_vertices[w_t_id%len(k_vertices)])
                        break
                    F_curr_t += 1
            
            # selection of new L 1 1
            L_curr_u = L_id - 1
            limit = len(k_vertices) - 1 if not bounded else L_id - 2
            while L_curr_u != limit:
                print("Should be here")
                w_u1 = k_vertices[L_curr_u%len(k_vertices)]
                w_u2 = k_vertices[(L_curr_u + 1)%len(k_vertices)]
                inf_w = inf_coord(v2, w_u1, True)
                if is_left(v2, inf_w, w_u2) > 0:
                    L_id = k_vertices_new.index(k_vertices[L_curr_u%len(k_vertices)])
                    print("Breaking", L_id)
                    break
                L_curr_u = (L_curr_u + 1) % len(k_vertices)
            if L_curr_u == limit:
                L_id = k_vertices_new.index(k_vertices[L_id%len(k_vertices)])
            k_vertices = k_vertices_new
        else: # convex angle
            left = is_left(v1, v2, k_vertices[L_id%len(k_vertices)])
            # print(v1, v2, k_vertices[L_id])
            if left < 0 or isclose(0, left, abs_tol=tol): # L to the right or on edge
                # scan K cw from L
                L_curr_t = L_id
                w_p = None
                w_t_id = None
                while L_curr_t != F_id:
                    w_t_id = L_curr_t
                    w_t1 = k_vertices[(w_t_id - 1)%len(k_vertices)]
                    w_t2 = k_vertices[w_t_id%len(k_vertices)]
                    w_p = inf_intersection(v1, inf_v2, w_t1, w_t2)
                    if w_p is not None and is_on(v1, v2, w_p, inf_v2=True) and is_on(w_t1, w_t2, w_p):
                        break
                    L_curr_t -= 1
                if w_p is None: # kernel is empty
                    print("EMPTY 2")
                    return []
                
                L_curr_s = L_id + 1
                w_p2 = None
                w_s_id = None
                limit = len(k_vertices) if not bounded else L_id - 1
                while L_curr_s != limit:
                    w_s_id = L_curr_s
                    w_s1 = k_vertices[(w_s_id - 1)%len(k_vertices)]
                    w_s2 = k_vertices[w_s_id%len(k_vertices)]
                    w_p2 = inf_intersection(v1, inf_v2, w_s1, w_s2)
                    if w_p2 is not None and is_on(v1, v2, w_p2, inf_v2=True) and is_on(w_s1, w_s2, w_p2):
                        break
                    L_curr_s = (L_curr_s + 1) % len(k_vertices)
                if w_p2 is not None:
                    k_vertices_new = k_vertices[:w_t_id] + [w_p, w_p2] + k_vertices[w_s_id:]
                else:
                    head = k_vertices[0]
                    tail = k_vertices[-1]
                    pre_tail = k_vertices[-2]
                    inter = inf_intersection(v1, v2, k_vertices[1], k_vertices[0])
                    # if is_left(v1, inf_v2, head) > 0 and is_left(pre_tail, tail, inf_v2) > 0: # slope between head and tail
                    if inter is not None and is_on(v1, v2, inter, inf_v2=True) and is_on(k_vertices[1], k_vertices[0], inter, inf_v2=True):
                        w_r_id = 1
                        while w_p2 is None:
                            w_r1 = k_vertices[(w_r_id - 1)%len(k_vertices)]
                            w_r2 = k_vertices[w_r_id%len(k_vertices)]
                            w_p2 = inf_intersection(v1, v2, w_r1, w_r2)
                            if w_p2 is not None and is_on(v1, v2, w_p2, inf_v2=True):
                                break
                            w_r_id = (w_r_id - 1)%len(k_vertices)
                        k_vertices_new = k_vertices[w_r_id:w_t_id] + [w_p, w_p2]
                        bounded = True
                    else: # slope between head and tail
                        k_vertices_new = k_vertices[:w_t_id] + [w_p, inf_v2]
                # set new F and L
                if w_p2 is not None: # 2 1 1
                    # new F
                    if is_on(v1, w_p, v2): # colinear
                        F_curr_t = F_id
                        while True:
                            w_t_id = F_curr_t
                            w_t1 = k_vertices[w_t_id%len(k_vertices)]
                            w_t2 = k_vertices[(w_t_id + 1)%len(k_vertices)]
                            inf_w = inf_coord(v2, w_t1, True)
                            if is_left(v2, inf_w, w_t2) < 0:
                                # F_id = w_t_id
                                F_id = k_vertices_new.index(k_vertices[w_t_id%len(k_vertices)])
                                break
                            F_curr_t += 1
                    else:
                        F_id = k_vertices_new.index(w_p)
                    
                    # new L
                    if isclose(0, is_left(v1, w_p2, v2), abs_tol=tol):
                        L_id = k_vertices_new.index(w_p2)
                    else:
                        start = k_vertices_new.index(w_p2)
                        L_curr_u = start - 1
                        limit = len(k_vertices) - 1 if not bounded else start - 2
                        while L_curr_u != limit:
                            w_u1 = k_vertices[L_curr_u%len(k_vertices)]
                            w_u2 = k_vertices[(L_curr_u + 1)%len(k_vertices)]
                            inf_w = inf_coord(v2, w_u1, True)
                            if is_left(v2, inf_w, w_u2) > 0:
                                # L_id = L_curr_u
                                L_id = k_vertices_new.index(k_vertices[L_curr_u%len(k_vertices)])
                                break
                            L_curr_u = (L_curr_u + 1) % len(k_vertices)
                else: # 2 1 2
                    # new F
                    if is_on(v1, w_p, v2): # colinear
                        F_curr_t = F_id
                        while True:
                            w_t_id = F_curr_t
                            w_t1 = k_vertices[w_t_id%len(k_vertices)]
                            w_t2 = k_vertices[(w_t_id + 1)%len(k_vertices)]
                            inf_w = inf_coord(v2, w_t1, True)
                            if is_left(v2, inf_w, w_t2) < 0:
                                # F_id = w_t_id
                                F_id = k_vertices_new.index(k_vertices[w_t_id%len(k_vertices)])
                                break
                            F_curr_t += 1
                    else:
                        F_id = k_vertices_new.index(w_p)
                    # new L
                    L_id = len(k_vertices_new) - 1
                k_vertices = k_vertices_new
            else: # L to the left of edge
                k_vertices_new = k_vertices
                # new F
                F_curr_t = F_id
                while True:
                    w_t_id = F_curr_t
                    w_t1 = k_vertices[w_t_id%len(k_vertices)]
                    w_t2 = k_vertices[(w_t_id + 1)%len(k_vertices)]
                    inf_w = inf_coord(v2, w_t1, True)
                    if is_left(v2, inf_w, w_t2) < 0:
                        # F_id = w_t_id
                        F_id = k_vertices_new.index(k_vertices[w_t_id%len(k_vertices)])
                        break
                    F_curr_t += 1
                # new L
                if bounded:
                    L_curr_u = L_id - 1
                    limit = len(k_vertices) - 1 if not bounded else L_id - 2
                    while L_curr_u != limit:
                        w_u1 = k_vertices[L_curr_u%len(k_vertices)]
                        w_u2 = k_vertices[(L_curr_u + 1)%len(k_vertices)]
                        inf_w = inf_coord(v2, w_u1, True)
                        if is_left(v2, inf_w, w_u2) > 0:
                            # L_id = L_curr_u
                            L_id = k_vertices_new.index(k_vertices[L_curr_u%len(k_vertices)])
                            break
                        L_curr_u = (L_curr_u + 1) % len(k_vertices)

    print("end:", k_vertices)
    return k_vertices

def kernel_mesh(regions, vertices):
    output = "OFF\n"
    v = ""
    index = ""
    offset = 0
    v_count = 0
    faces = 0
    for poly in regions:
        color = "255 0 0"
        print("NEW REGION")
        kernel_vertices = kernel_poly(poly, vertices)
        original_poly = [vertices[i] for i in poly]
        if kernel_vertices == original_poly:
            print("None")
            color = "255 255 0"
        print("Kernel:", kernel_vertices)
        v_count += len(kernel_vertices)
        faces += 1
        index += str(len(kernel_vertices))
        for j in range(len(kernel_vertices)):
            vert = kernel_vertices[j]
            v += "{} {} 0.0\n".format(vert[0], vert[1])
            index += " " + str(offset + j)
        index += " " + color + "\n"
        offset += len(kernel_vertices)
    output += "{} {} 0\n".format(v_count, faces)
    output += v
    output += index
    return output

if __name__ == "__main__":
    filename = sys.argv[1]
    vertices, regions = read_OFF(filename)
    print(vertices)
    print(regions)
    
    print("Edges per polygon:", calc_edges_per_polygon(regions))
    
    assert(angle_between((0,0), (0,1), (0,2)) == 180.0)
    assert(angle_between((0,0), (0,0), (0,0)) == 0.0)
    assert(angle_between((-1,0), (0,0), (0,1)) == 90.0)

    #square max min angle
    assert(min_max_angle_polygon([0, 1, 2, 3], [(0,0), (1,0), (1,1), (0,1)]) == (90.0, 90.0))

    #two triangle mesh in a square
    assert(min_max_angle_mesh([[0, 1, 3], [1, 2, 3]], [(0,0), (1,0), (1,1), (0,1)]) == (45.0, 90.0))

    # is_left
    assert(Edge((0, 0), (0, 1)).is_left((-1, 0)) > 0)
    assert(Edge((0, 0), (0, 1)).is_left((1, 0)) < 0)
    assert(Edge((0, 0), (0, 1)).is_left((0, 3)) == 0)
    assert(is_left((0, 0), (0, 1), (-1, 0)) > 0)
    assert(is_left((0, 0), (0, 1), ( 1, 0)) < 0)
    assert(is_left((0, 0), (0, 1), ( 0, 3)) == 0)

    # intersection
    assert(Edge((0, -1), (0, 1)).intersection(Edge((-1, 0), (1, 0))) == (0, 0))
    assert(Edge((0, 0), (2, 1)).intersection(Edge((2, 0), (0, 1))) == (1, 0.5))
    assert(intersection((-1, 0), (1, 0), (0, -1), (0, 1)) == (0, 0))
    assert(intersection((2, 0), (0, 1), (0, 0), (2, 1)) == (1, 0.5))
    
    # intersection in inf edges
    assert(Edge((0, -1), (0, 1)).intersection(Edge((-1, 10), (1, 10))) is None)
    assert(Edge((0, -1), (0, 1), is_v2_fixed=False).intersection(Edge((-1, 10), (1, 10))) == (0, 10))
    assert(Edge((0, -1), (0, 1), is_v1_fixed=False).intersection(Edge((-1, -10), (1, -10))) == (0, -10))

    # inf vertex
    test_coord = inf_coord((3.5, 3.5), (4.5, 4.5), True)
    assert(test_coord[0] > 4.5)
    assert(test_coord[1] > 4.5)
    assert(Edge((0.0, 0.0), (1.0, 1.0)).is_left(test_coord) == 0)
    assert(is_left((3.5, 3.5), (4.5, 4.5), test_coord) == 0)

    min_angle, max_angle = min_max_angle_mesh(regions, vertices)
    print("Min angle:", min_angle)
    print("Max angle:", max_angle)

    min_aspect_ratio, max_aspect_ratio = min_max_aspect_ratio_mesh(regions, vertices)
    avg_aspect_ratio = avg_aspect_ratio_mesh(regions, vertices)
    print("Min aspect ratio:", min_aspect_ratio)
    print("Max aspect ratio:", max_aspect_ratio)
    print("Avg aspect ratio:", avg_aspect_ratio)

    kernel_off = kernel_mesh(regions, vertices)

    with open("kernel_res.off", "w") as f:
        f.write(kernel_off)

    if len(sys.argv) == 3:
        with open(sys.argv[2], "a") as f:
            data = [min_angle, max_angle, min_aspect_ratio, max_aspect_ratio, avg_aspect_ratio]
            f.write(os.path.basename(filename) + ",")
            f.write(",".join(map(lambda x: "{:.2f}".format(x), data)))
            f.write("\n")
            f.close()
