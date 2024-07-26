import utils
from analytics import read_OFF
import sys

def kernel_mesh(regions, vertices):
    output = "OFF\n"
    v = ""
    index = ""
    offset = 0
    v_count = 0
    faces = 0
    for poly in regions:
        color = "255 0 0"
        # print("NEW REGION")
        try:
            kernel_vertices = utils.kernel_poly(poly, vertices)
        except(ValueError):
            continue
        original_poly = [vertices[i] for i in poly]
        if kernel_vertices == original_poly:
            # print("None")
            color = "255 255 0"
        elif len(kernel_vertices) == 0:
            print(poly)
            continue
        # print("Kernel:", kernel_vertices)
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
    kernel_off = kernel_mesh(regions, vertices)

    kernel_output = filename[:-4] + "_kernel.off"
    with open(kernel_output, "w") as f:
        f.write(kernel_off)
        print("Written kernel mesh to", kernel_output)
