from math import dist, isclose, degrees, atan2

tol = 1e-6
inter_tol = tol

#https://stackoverflow.com/questions/58953047/issue-with-finding-angle-between-3-points-in-python
def angle_between(p1, p2, p3):
    x1, y1 = p1
    x2, y2 = p2
    x3, y3 = p3
    deg1 = (360 + degrees(atan2(x1 - x2, y1 - y2))) % 360
    deg2 = (360 + degrees(atan2(x3 - x2, y3 - y2))) % 360
    return deg2 - deg1 if deg1 <= deg2 else 360 - (deg1 - deg2)

def get_first_reflex(poly, vertices):
    stard_id = 0
    n = len(poly)
    while stard_id != n:
        angle = angle_between(vertices[poly[(stard_id - 1)%n]], vertices[poly[(stard_id)%n]], vertices[poly[(stard_id + 1)%n]])
        if angle > 180 and not isclose(180, angle, rel_tol=1e-2, abs_tol=1e-2):
        # if angle > 180 and not isclose(180, angle, rel_tol=tol, abs_tol=tol):
            # print(angle)
            break
        stard_id += 1
    return stard_id

def inf_coord(v1, v2, along_v2=False):
    mod = dist(v1, v2)
    if isclose(0, mod, abs_tol=tol):
        return v2 if along_v2 else v1
    direction = ((v2[0] - v1[0])/mod, (v2[1] - v1[1])/mod)
    inf_mult = 100000
    if along_v2:
        return (v2[0] + direction[0] * inf_mult, v2[1] + direction[1] * inf_mult)
    else:
        return (v1[0] - direction[0] * inf_mult, v1[1] - direction[1] * inf_mult)


# return >0 if to the left, 0 if colinear, <0 if to the right
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
    if isclose(0, det, abs_tol=tol, rel_tol=tol):
        return None
    p_x = ((x1*y2 - y1*x2)*(x3 - x4) - (x1 - x2)*(x3*y4 - y3*x4))/det
    p_y = ((x1*y2 - y1*x2)*(y3 - y4) - (y1 - y2)*(x3*y4 - y3*x4))/det
    if (min(x1, x2) <= p_x and p_x <= max(x1, x2) and min(y1, y2) <= p_y and p_y <= max(y1, y2) and
        min(x3, x4) <= p_x and p_x <= max(x3, x4) and min(y3, y4) <= p_y and p_y <= max(y3, y4)):
        return (p_x, p_y)
    return None

def inf_intersection(v1, v2, v3, v4):
    global inter_tol
    inter_tol = tol
    x1 = v1[0]
    y1 = v1[1]
    x2 = v2[0]
    y2 = v2[1]
    x3 = v3[0]
    y3 = v3[1]
    x4 = v4[0]
    y4 = v4[1]
    det = (x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4)
    if isclose(0, det, abs_tol=1e-9):
        return None
    p_x = ((x1*y2 - y1*x2)*(x3 - x4) - (x1 - x2)*(x3*y4 - y3*x4))/det
    p_y = ((x1*y2 - y1*x2)*(y3 - y4) - (y1 - y2)*(x3*y4 - y3*x4))/det
    # if abs(det) < 1:
    #     inter_tol = 1
    while (not is_on(v1, v2, (p_x, p_y), inf_v1=True, inf_v2=True, local_tol=inter_tol) or 
            not is_on(v3, v4, (p_x, p_y), inf_v1=True, inf_v2=True, local_tol=inter_tol)):
        inter_tol *= 10
        # print("BAD", inter_tol)
    return (p_x, p_y)

def inf_is_left(v1, v2, p1, p2):
    left = is_left(v1, v2, p2)
    inter = inf_intersection(v1, v2, p1, p2)
    if inter is not None and not is_on(p1, p2, inter) and is_on(p1, p2, inter, inf_v2=True):
        return -left
    return left

def is_on(v1, v2, p, inf_v1=False, inf_v2=False, local_tol=tol, exact=False):
    x1 = v1[0]
    y1 = v1[1]
    x2 = v2[0]
    y2 = v2[1]
    px = p[0]
    py = p[1]
    on_line = isclose(0, is_left(v1, v2, p), abs_tol=local_tol)
    if not inf_v1 and not inf_v2:
        if not exact:
            return ((min(x1, x2) <= px or isclose(min(x1, x2), px, rel_tol=local_tol, abs_tol=local_tol)) and 
                    (px <= max(x1, x2) or isclose(max(x1, x2), px, rel_tol=local_tol, abs_tol=local_tol)) and 
                    (min(y1, y2) <= py or isclose(min(y1, y2), py, rel_tol=local_tol, abs_tol=local_tol)) and 
                    (py <= max(y1, y2) or isclose(max(y1, y2), py, rel_tol=local_tol, abs_tol=local_tol)) and 
                    on_line)
        else:
            return ((min(x1, x2) <= px) and 
                    (px <= max(x1, x2)) and 
                    (min(y1, y2) <= py) and 
                    (py <= max(y1, y2)) and 
                    on_line)
    elif not inf_v1 and inf_v2:
        if x1 < x2:
            if y1 < y2:
                return ((x1 < px or isclose(x1, px, rel_tol=local_tol, abs_tol=local_tol)) and 
                        (y1 < py or isclose(y1, py, rel_tol=local_tol, abs_tol=local_tol)) and 
                        on_line)
            else:
                return ((x1 < px or isclose(x1, px, rel_tol=local_tol, abs_tol=local_tol)) and 
                        (py < y1 or isclose(y1, py, rel_tol=local_tol, abs_tol=local_tol)) and 
                        on_line)
        else:
            if y1 < y2:
                return ((px < x1 or isclose(x1, px, rel_tol=local_tol, abs_tol=local_tol)) and 
                        (y1 < py or isclose(y1, py, rel_tol=local_tol, abs_tol=local_tol)) and
                        on_line)
            else:
                return ((px < x1 or isclose(x1, px, rel_tol=local_tol, abs_tol=local_tol)) and 
                        (py < y1 or isclose(y1, py, rel_tol=local_tol, abs_tol=local_tol)) and 
                        on_line)
    elif inf_v1 and not inf_v2:
        if x1 < x2:
            if y1 < y2:
                return ((px <= x2 or isclose(x2, px, rel_tol=local_tol, abs_tol=local_tol)) and 
                        (py <= y2 or isclose(y2, py, rel_tol=local_tol, abs_tol=local_tol)) and 
                        on_line)
            else:
                return ((px <= x2 or isclose(x2, px, rel_tol=local_tol, abs_tol=local_tol)) and 
                        (y2 <= py or isclose(y2, py, rel_tol=local_tol, abs_tol=local_tol)) and 
                        on_line)
        else:
            if y1 < y2:
                return ((x2 <= px or isclose(x2, px, rel_tol=local_tol, abs_tol=local_tol)) and 
                        (py <= y2 or isclose(y2, py, rel_tol=local_tol, abs_tol=local_tol)) and 
                        on_line)
            else:
                return ((x2 <= px or isclose(x2, px, rel_tol=local_tol, abs_tol=local_tol)) and 
                        (y2 <= py or isclose(y2, py, rel_tol=local_tol, abs_tol=local_tol)) and 
                        on_line)
    else:
        return on_line

def kernel_poly(poly, vertices):
    start_id = get_first_reflex(poly, vertices)
    # print(poly)
    # print(start_id)
    # print("Poly:", [vertices[i] for i in poly])
    # print("IDS:", [i for i in poly])
    n = len(poly)
    if start_id == n: # convex
        return [vertices[i] for i in poly]
    # v_F = inf_coord(vertices[poly[start_id]], vertices[poly[(start_id + 1)%n]])
    # v_L = inf_coord(vertices[poly[(start_id - 1)%n]], vertices[poly[start_id]], True)
    dx_F = vertices[poly[start_id]][0] - vertices[poly[(start_id + 1)%n]][0]
    dy_F = vertices[poly[start_id]][1] - vertices[poly[(start_id + 1)%n]][1]
    v_F = (vertices[poly[start_id]][0] + dx_F * 100, vertices[poly[start_id]][1] + dy_F * 100)
    dx_L = vertices[poly[start_id]][0] - vertices[poly[(start_id - 1)%n]][0]
    dy_L = vertices[poly[start_id]][1] - vertices[poly[(start_id - 1)%n]][1]
    v_L = (vertices[poly[start_id]][0] + dx_L * 100, vertices[poly[start_id]][1] + dy_L * 100)
    k_vertices = [v_F, vertices[poly[start_id]], v_L]
    F_id = 0
    L_id = 2
    bounded = False
    skip = 1
    for p_i in range(1, len(poly) - 1):
        k_vertices_new = []
        i = poly[(start_id + p_i)%n]
        # print(vertices)
        # print("Current:", p_i)
        # print("Start id:", start_id)
        # print("ID:", i)
        # print("F:", F_id)
        # print("L:", L_id)
        # print("K vertices:", k_vertices)
        v0 = vertices[poly[(start_id + p_i - 1)%n]]
        v1 = vertices[poly[(start_id + p_i)%n]]
        v2 = vertices[poly[(start_id + p_i + 1)%n]]
        # print(v0, v1, v2)
        inf_v1 = inf_coord(v1, v2)
        inf_v2 = inf_coord(v1, v2, True)
        angle = angle_between(v0, v1, v2)
        # print("Current angle:", angle)
        # print(poly[(start_id + p_i)%n])
        # print(bounded)
        # print(k_vertices)
        if isclose(0.0, angle, abs_tol=tol, rel_tol=tol):
            return []
        
        if skip > 1: # skip collinear points
            skip -= 1
            continue
        
        next_v0 = vertices[poly[(start_id + p_i + skip - 1)%n]]
        next_v1 = vertices[poly[(start_id + p_i + skip)%n]]
        next_v2 = vertices[poly[(start_id + p_i + skip + 1)%n]]
        next_angle = angle_between(next_v0, next_v1, next_v2)
        # if isclose(180.0, next_angle, abs_tol=1e-2, rel_tol=1e-2): # colinear
        #     print("continued")
        #     continue
        while isclose(0.0, next_angle - 180.0, abs_tol=1e-4): # colinear
            v2 = next_v2
            inf_v2 = inf_coord(v1, v2, True)
            skip += 1
            next_v0 = vertices[poly[(start_id + p_i + skip - 1)%n]]
            next_v1 = vertices[poly[(start_id + p_i + skip)%n]]
            next_v2 = vertices[poly[(start_id + p_i + skip + 1)%n]]
            next_angle = angle_between(next_v0, next_v1, next_v2)
            if (isclose(next_v1[0], k_vertices[0][0], abs_tol=tol, rel_tol=tol) and 
                isclose(next_v1[1], k_vertices[0][1], abs_tol=tol, rel_tol=tol)):
                return k_vertices
            # continue
        if angle > 180: # reflex angle
            left = None
            if not bounded and F_id == 0:
                left = inf_is_left(v1, v2, k_vertices[(F_id + 1)%len(k_vertices)], k_vertices[F_id%len(k_vertices)])
            else:
                left = is_left(v1, v2, k_vertices[F_id%len(k_vertices)])
            if left < 0: # F to the right or on edge
            # if left <= 0: # F to the right or on edge
                # scan K ccw from F
                F_curr_t = F_id + 1
                w_p = None
                w_t_id = None
                while F_curr_t != (L_id + 1)%len(k_vertices):
                    w_t_id = F_curr_t%len(k_vertices)
                    w_t1 = k_vertices[(w_t_id - 1)%len(k_vertices)]
                    w_t2 = k_vertices[w_t_id%len(k_vertices)]
                    w_p = inf_intersection(v1, v2, w_t1, w_t2)
                    inf_v1_b = w_t_id == 1 and not bounded
                    inf_v2_b = w_t_id == len(k_vertices) - 1 and not bounded
                    if (w_p is not None and 
                        is_on(v1, v2, w_p, inf_v1=True, local_tol=inter_tol) and 
                        is_on(w_t1, w_t2, w_p, inf_v1=inf_v1_b, inf_v2=inf_v2_b, local_tol=inter_tol)):
                        break
                    w_p = None
                    F_curr_t = (F_curr_t + 1) % len(k_vertices)
                if w_p is None or F_curr_t == (L_id + 1)%len(k_vertices):
                    # print(poly)
                    # print(start_id)
                    # print("EMPTY 1")
                    # if isclose(180.0, angle, rel_tol=1e-2, abs_tol=1e-2):
                    #     print("continue")
                    #     continue
                    return [] # kernel is empty
                # scan K cw from F
                F_curr_s = F_id
                w_p2 = None
                w_s_id = None
                limit = 0 if not bounded else (F_id + 1)%len(k_vertices)
                while w_s_id != limit:
                    w_s_id = F_curr_s%len(k_vertices)
                    w_s1 = k_vertices[(w_s_id - 1)%len(k_vertices)]
                    w_s2 = k_vertices[w_s_id%len(k_vertices)]
                    w_p2 = inf_intersection(v1, v2, w_s1, w_s2)
                    inf_v1_b = w_s_id == 1 and not bounded
                    inf_v2_b = w_s_id == len(k_vertices) - 1 and not bounded
                    if (w_p2 is not None and 
                        is_on(v1, v2, w_p2, inf_v1=True, local_tol=inter_tol) and 
                        is_on(w_s1, w_s2, w_p2, inf_v1=inf_v1_b, inf_v2=inf_v2_b, local_tol=inter_tol)):
                        break
                    w_p2 = None
                    F_curr_s = (F_curr_s - 1)%len(k_vertices)
                if w_p2 is not None:
                    if (bounded and isclose(0, dist(w_p, w_p2), abs_tol=1e-4) and 
                        dist(w_p2, k_vertices[w_s_id]) < dist(w_p, k_vertices[w_s_id])):
                        # print("swapping reflex", w_p, w_p2, dist(w_p, w_p2))
                        (w_p2, w_p) = (w_p, w_p2)
                    if bounded and w_t_id < w_s_id:
                        k_vertices_new = k_vertices[w_t_id:w_s_id] + [w_p2, w_p]
                    else:
                        s_0 = k_vertices[(w_s_id - 1)%len(k_vertices)]
                        s_1 = k_vertices[w_s_id%len(k_vertices)]
                        if dist(w_p2, s_1) > dist(s_0, s_1):
                            # intersection is farther away than the infinite point
                            d_x = s_1[0] - s_0[0]
                            d_y = s_1[1] - s_0[1]
                            k_vertices[(w_s_id - 1)%len(k_vertices)] = (s_0[0] - d_x, s_0[1] - d_y)
                        t_0 = k_vertices[(w_t_id - 1)%len(k_vertices)]
                        t_1 = k_vertices[w_t_id%len(k_vertices)]
                        if dist(w_p, t_0) > dist(t_0, t_1):
                            # intersection is farther away than the infinite point
                            d_x = t_1[0] - t_0[0]
                            d_y = t_1[1] - t_0[1]
                            k_vertices[w_t_id%len(k_vertices)] = (s_0[0] + d_x, s_0[1] + d_y)
                        k_vertices_new = k_vertices[:w_s_id] + [w_p2, w_p] + k_vertices[w_t_id:]
                else:
                    head = k_vertices[0]
                    post_head = k_vertices[1]
                    tail = k_vertices[-1]
                    pre_tail = k_vertices[-2]
                    inter = inf_intersection(v1, v2, k_vertices[1], k_vertices[0])
                    # if is_left(v2, inf_v1, head) > 0 and is_left(pre_tail, tail, inf_v1) > 0: # slope between head and tail
                    # if (inter is not None and 
                    #     is_on(v1, v2, inter, inf_v2=True, local_tol=inter_tol) and 
                    #     is_on(k_vertices[1], k_vertices[0], inter, inf_v2=True, local_tol=inter_tol)):
                    if not (inf_is_left(v2, v1, post_head, head) > 0 and inf_is_left(v1, v2, pre_tail, tail) < 0):
                        w_r_id = len(k_vertices) - 1
                        while w_p2 is None:
                            w_r1 = k_vertices[(w_r_id - 1)%len(k_vertices)]
                            w_r2 = k_vertices[w_r_id%len(k_vertices)]
                            w_p2 = inf_intersection(v1, v2, w_r1, w_r2)
                            inf_v1_b = w_r_id == 1 and not bounded
                            inf_v2_b = w_r_id == len(k_vertices) - 1 and not bounded
                            if (w_p2 is not None and 
                                is_on(v1, v2, w_p2, inf_v1=True, local_tol=inter_tol) and 
                                is_on(w_r1, w_r2, w_p2, inf_v1=inf_v1_b, inf_v2=inf_v2_b, local_tol=inter_tol)):
                                break
                            w_p2 = None
                            w_r_id = (w_r_id - 1) % len(k_vertices)
                        k_vertices_new = k_vertices[w_t_id:w_r_id] + [w_p2, w_p]
                        bounded = True
                    else: # slope between head and tail
                        inf_v = inf_coord(w_p, v2)
                        start_v = inf_v1 if dist(inf_v1, v2) > dist(inf_v, v2) else inf_v
                        # k_vertices_new = [inf_v1, w_p] + k_vertices[w_t_id:]
                        k_vertices_new = [start_v, w_p] + k_vertices[w_t_id:]
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
                    w_t_id = F_curr_t%len(k_vertices)
                    w_t1 = k_vertices[w_t_id%len(k_vertices)]
                    w_t2 = k_vertices[(w_t_id + 1)%len(k_vertices)]
                    # inf_w = inf_coord(v2, w_t1, True)
                    if is_left(v2, w_t1, w_t2) < 0:
                        # F_id = w_t_id
                        F_id = k_vertices_new.index(k_vertices[w_t_id%len(k_vertices)])
                        break
                    F_curr_t = (F_curr_t + 1) % len(k_vertices)
            
            # selection of new L 1 1
            L_curr_u = L_id - 1
            limit = len(k_vertices) - 1 if not bounded else (L_id - 2)%len(k_vertices)
            while L_curr_u != limit:
                w_u1 = k_vertices[L_curr_u%len(k_vertices)]
                w_u2 = k_vertices[(L_curr_u + 1)%len(k_vertices)]
                # inf_w = inf_coord(v2, w_u1, True)
                if is_left(v2, w_u1, w_u2) > 0:
                    # print("new", k_vertices_new)
                    L_id = k_vertices_new.index(k_vertices[L_curr_u%len(k_vertices)])
                    break
                L_curr_u = (L_curr_u + 1) % len(k_vertices)
            if L_curr_u == limit:
                L_id = k_vertices_new.index(k_vertices[L_id%len(k_vertices)])
            k_vertices = k_vertices_new
        else: # convex angle
            # print("convex")
            left = None
            if not bounded and L_id == len(k_vertices) - 1:
                left = inf_is_left(v1, v2, k_vertices[(L_id - 1)%len(k_vertices)], k_vertices[L_id%len(k_vertices)])
            else:
                left = is_left(v1, v2, k_vertices[L_id%len(k_vertices)])
            # print(v1, v2, k_vertices[L_id])
            if left < 0: # L to the right
                # scan K cw from L
                L_curr_t = L_id
                w_p = None
                w_t_id = None
                while L_curr_t != F_id:
                    w_t_id = L_curr_t%len(k_vertices)
                    w_t1 = k_vertices[(w_t_id - 1)%len(k_vertices)]
                    w_t2 = k_vertices[w_t_id%len(k_vertices)]
                    w_p = inf_intersection(v1, v2, w_t1, w_t2)
                    inf_v1_b = w_t_id == 1 and not bounded
                    inf_v2_b = w_t_id == len(k_vertices) - 1 and not bounded
                    if (w_p is not None and 
                        is_on(v1, v2, w_p, inf_v2=True, local_tol=inter_tol) and 
                        is_on(w_t1, w_t2, w_p, inf_v1=inf_v1_b, inf_v2=inf_v2_b, local_tol=inter_tol)):
                        break
                    w_p = None
                    L_curr_t = (L_curr_t - 1)%len(k_vertices)
                if w_p is None or L_curr_t == F_id:
                    # print(poly)
                    # print(start_id)
                    # print("EMPTY 2", w_p)
                    # if isclose(180.0, angle, rel_tol=1e-2, abs_tol=1e-2):
                    #     print("continue")
                    #     continue
                    return [] # kernel is empty
                
                L_curr_s = L_id + 1
                w_p2 = None
                w_s_id = None
                limit = len(k_vertices) if not bounded else (L_id - 1)%len(k_vertices)
                while L_curr_s != limit:
                    w_s_id = L_curr_s%len(k_vertices)
                    w_s1 = k_vertices[(w_s_id - 1)%len(k_vertices)]
                    w_s2 = k_vertices[w_s_id%len(k_vertices)]
                    w_p2 = inf_intersection(v1, v2, w_s1, w_s2)
                    inf_v1_b = w_s_id == 1 and not bounded
                    inf_v2_b = w_s_id == len(k_vertices) - 1 and not bounded
                    if (w_p2 is not None and 
                        is_on(v1, v2, w_p2, inf_v2=True, local_tol=inter_tol) and 
                        is_on(w_s1, w_s2, w_p2, inf_v1=inf_v1_b, inf_v2=inf_v2_b, local_tol=inter_tol)):
                        break
                    w_p2 = None
                    L_curr_s = (L_curr_s + 1) % len(k_vertices)
                if w_p2 is not None:
                    if (bounded and isclose(0, dist(w_p, w_p2), abs_tol=1e-4) and 
                        dist(w_p2, k_vertices[w_t_id]) > dist(w_p, k_vertices[w_t_id])):
                        # print("swapping convex", w_t_id, w_s_id, w_p, w_p2, dist(w_p, w_p2))
                        (w_p2, w_p) = (w_p, w_p2)
                    if bounded and w_s_id < w_t_id:
                        st_id = w_s_id if w_s_id < w_t_id else w_s_id - len(k_vertices)
                        k_vertices_new = k_vertices[w_s_id:w_t_id] + [w_p, w_p2]
                    else:
                        t_0 = k_vertices[(w_t_id - 1)%len(k_vertices)]
                        t_1 = k_vertices[w_t_id%len(k_vertices)]
                        if dist(w_p, t_1) > dist(t_0, t_1):
                            # intersection is farther away than the infinite point
                            d_x = t_1[0] - t_0[0]
                            d_y = t_1[1] - t_0[1]
                            k_vertices[(w_t_id - 1)%len(k_vertices)] = (t_0[0] - d_x, t_0[1] - d_y)
                        s_0 = k_vertices[(w_s_id - 1)%len(k_vertices)]
                        s_1 = k_vertices[w_s_id%len(k_vertices)]
                        if dist(w_p2, t_0) > dist(s_0, s_1):
                            # intersection is farther away than the infinite point
                            d_x = s_1[0] - s_0[0]
                            d_y = s_1[1] - s_0[1]
                            k_vertices[w_s_id%len(k_vertices)] = (s_0[0] + d_x, s_0[1] + d_y)
                        k_vertices_new = k_vertices[:w_t_id] + [w_p, w_p2] + k_vertices[w_s_id:]
                else:
                    head = k_vertices[0]
                    post_head = k_vertices[1]
                    tail = k_vertices[-1]
                    pre_tail = k_vertices[-2]
                    inter = inf_intersection(v1, v2, k_vertices[1], k_vertices[0])
                    # if is_left(v1, inf_v2, head) > 0 and is_left(pre_tail, tail, inf_v2) > 0: # slope between head and tail
                    # if (inter is not None and 
                    #     is_on(v1, v2, inter, inf_v2=True, local_tol=inter_tol) and 
                    #     is_on(k_vertices[1], k_vertices[0], inter, inf_v1=True, inf_v2=True, local_tol=inter_tol)):
                    if not (inf_is_left(v1, v2, post_head, head) > 0 and inf_is_left(v1, v2, pre_tail, tail) < 0):
                        w_r_id = 1
                        while w_p2 is None:
                            w_r1 = k_vertices[(w_r_id - 1)%len(k_vertices)]
                            w_r2 = k_vertices[w_r_id%len(k_vertices)]
                            w_p2 = inf_intersection(v1, v2, w_r1, w_r2)
                            inf_v1_b = w_r_id == 1 and not bounded
                            inf_v2_b = w_r_id == len(k_vertices) - 1 and not bounded
                            if (w_p2 is not None and 
                                is_on(v1, v2, w_p2, inf_v2=True, local_tol=inter_tol) and 
                                is_on(w_r1, w_r2, w_p2, inf_v1=inf_v1_b, inf_v2=inf_v2_b, local_tol=inter_tol)):
                                break
                            w_p2 = None
                            w_r_id = (w_r_id + 1)%len(k_vertices)
                        k_vertices_new = k_vertices[w_r_id:w_t_id] + [w_p, w_p2]
                        bounded = True
                    else: # slope between head and tail
                        inf_v = inf_coord(v1, w_p, along_v2=True)
                        end_v = inf_v2 if dist(v1, inf_v2) > dist(v1, inf_v) else inf_v
                        # k_vertices_new = k_vertices[:w_t_id] + [w_p, inf_v2]
                        k_vertices_new = k_vertices[:w_t_id] + [w_p, end_v]
                # set new F and L
                if w_p2 is not None: # 2 1 1
                    # new F
                    if is_on(v1, w_p, v2, exact=True): # colinear
                        F_curr_t = F_id
                        while True:
                            w_t_id = F_curr_t%len(k_vertices)
                            w_t1 = k_vertices[w_t_id%len(k_vertices)]
                            w_t2 = k_vertices[(w_t_id + 1)%len(k_vertices)]
                            # inf_w = inf_coord(v2, w_t1, True)
                            if is_left(v2, w_t1, w_t2) < 0:
                                # F_id = w_t_id
                                F_id = k_vertices_new.index(k_vertices[w_t_id%len(k_vertices)])
                                break
                            F_curr_t = (F_curr_t + 1) % len(k_vertices)
                    else:
                        F_id = k_vertices_new.index(w_p)
                    
                    # new L
                    # if isclose(0, is_left(v1, w_p2, v2), abs_tol=tol):
                    if is_on(v1, w_p2, v2, exact=True):
                        L_id = k_vertices_new.index(w_p2)
                    else:
                        start = k_vertices_new.index(w_p2)
                        # L_curr_u = start - 1
                        L_curr_u = start
                        limit = len(k_vertices_new) - 1 if not bounded else (start - 2)%len(k_vertices)
                        while L_curr_u != limit:
                            w_u1 = k_vertices_new[L_curr_u%len(k_vertices_new)]
                            w_u2 = k_vertices_new[(L_curr_u + 1)%len(k_vertices_new)]
                            # inf_w = inf_coord(v2, w_u1, True)
                            if is_left(v2, w_u1, w_u2) > 0:
                                # L_id = L_curr_u
                                L_id = k_vertices_new.index(k_vertices_new[L_curr_u%len(k_vertices_new)])
                                break
                            L_curr_u = (L_curr_u + 1) % len(k_vertices_new)
                else: # 2 1 2
                    # new F
                    if is_on(v1, w_p, v2, exact=True): # colinear
                        F_curr_t = F_id
                        # print("here")
                        while True:
                            w_t_id = F_curr_t
                            w_t1 = k_vertices[w_t_id%len(k_vertices)]
                            w_t2 = k_vertices[(w_t_id + 1)%len(k_vertices)]
                            # inf_w = inf_coord(v2, w_t1, True)
                            if is_left(v2, w_t1, w_t2) < 0:
                                # F_id = w_t_id
                                F_id = k_vertices_new.index(k_vertices[w_t_id%len(k_vertices)])
                                break
                            F_curr_t = (F_curr_t + 1) % len(k_vertices)
                    else:
                        # print("or here")
                        F_id = k_vertices_new.index(w_p)
                    # new L
                    L_id = len(k_vertices_new) - 1
                k_vertices = k_vertices_new
            else: # L to the left of edge
                k_vertices_new = k_vertices
                # new F
                F_curr_t = F_id
                # print("here?")
                while True:
                    w_t_id = F_curr_t%len(k_vertices)
                    w_t1 = k_vertices[w_t_id%len(k_vertices)]
                    w_t2 = k_vertices[(w_t_id + 1)%len(k_vertices)]
                    # inf_w = inf_coord(v2, w_t1, True)
                    if is_left(v2, w_t1, w_t2) < 0:
                        # F_id = w_t_id
                        F_id = k_vertices_new.index(k_vertices[w_t_id%len(k_vertices)])
                        break
                    F_curr_t = (F_curr_t + 1) % len(k_vertices)
                # new L
                if bounded:
                    L_curr_u = L_id - 1
                    limit = len(k_vertices) - 1 if not bounded else (L_id - 2)%len(k_vertices)
                    while L_curr_u != limit:
                        w_u_id = L_curr_u%len(k_vertices)
                        w_u1 = k_vertices[w_u_id%len(k_vertices)]
                        w_u2 = k_vertices[(w_u_id + 1)%len(k_vertices)]
                        # inf_w = inf_coord(v2, w_u1, True)
                        if is_left(v2, w_u1, w_u2) > 0:
                            # L_id = L_curr_u
                            L_id = k_vertices_new.index(k_vertices[w_u_id%len(k_vertices)])
                            break
                        L_curr_u = (L_curr_u + 1) % len(k_vertices)
    return k_vertices



if __name__ == "__main__":
    # is_left
    assert(is_left((0, 0), (0, 1), (-1, 0)) > 0)
    assert(is_left((0, 0), (0, 1), ( 1, 0)) < 0)
    assert(is_left((0, 0), (0, 1), ( 0, 3)) == 0)

    # intersection
    assert(intersection((-1, 0), (1, 0), (0, -1), (0, 1)) == (0, 0))
    assert(intersection((2, 0), (0, 1), (0, 0), (2, 1)) == (1, 0.5))

    # inf vertex
    test_coord = inf_coord((3.5, 3.5), (4.5, 4.5), True)
    assert(test_coord[0] > 4.5)
    assert(test_coord[1] > 4.5)
    assert(is_left((3.5, 3.5), (4.5, 4.5), test_coord) == 0)
