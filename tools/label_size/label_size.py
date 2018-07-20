import sys

def label_size():
    if len(sys.argv) < 3:
        print("Usage: python3 label_size.py <input_file> <output_file>")
        sys.exit(1)
    with open(sys.argv[1], 'r') as fin, \
         open(sys.argv[2], 'w') as fout:
        group = 10**3
        count = 0
        label_sum = 0
        for line in fin:
            count += 1
            columns = line.split()
            if (columns[0] == 'PLL'):
                break
            bfs = int(columns[0])
            label = int(columns[-2])
            label_sum += label
            if count % group == 0:
                average_size = label_sum / group
                output = "{} {}\n".format(bfs, average_size)
                label_sum = 0
                fout.write(output)
                print(output, end='')

if __name__ == '__main__':
    label_size()
