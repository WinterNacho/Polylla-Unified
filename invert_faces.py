import sys

n_vertices = None
n_regions = None

if __name__ == "__main__":
    filename = sys.argv[1]
    output = filename[:-4] + "_inverted.off"
    with open(filename) as fi:
        with open(output, "w") as fo:
            for line in fi:
                if line.startswith("#"):
                    continue
                l = line.split()
                if l[0] == "OFF":
                    fo.write("OFF\n")
                    break
                else:
                    sys.exit("Error: File is not an OFF file")
            for line in fi:
                if line.startswith('#'):
                    continue
                l = line.split()
                if l[0].isdigit():
                    n_vertices = int(l[0])
                    n_regions = int(l[1])
                    fo.write("{} {} 0\n".format(n_vertices, n_regions))
                    break
            for i in range(0, n_vertices):
                line = fi.readline()
                fo.write(line)
            for i in range(0, n_regions):
                line = fi.readline()
                l = line.split()
                fo.write(str(l.pop(0)) + " ")
                values = [l[j] for j in range(len(l) - 1, -1, -1)]
                fo.write(" ".join(values))
                fo.write("\n")



