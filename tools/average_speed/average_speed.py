import sys

def average():
    if len(sys.argv) < 3:
        print("Usage: python3 average_speed.py <input_file> <output_file>")
        sys.exit(1)
    with open(sys.argv[1], 'r') as fin, \
         open(sys.argv[2], 'w') as fout:
        count = 0
        label_sum = 0
        is_new_group = True
        for line in fin:
            count += 1
            columns = line.split()
            if (columns[0] == 'PLL'):
                break
            label_sum += int(columns[-2])
            time = float(columns[-1])
            if is_new_group:
                is_new_group = False
                time_start = time
            if count % 10**3 == 0:
                time_range = time - time_start
                average_speed = label_sum / time_range
                label_sum = 0
                is_new_group = True
                fout.write("{} {}\n".format(time, average_speed))
                print("{} {}\n".format(time, average_speed))
                
                
            

if __name__ == '__main__':
    average()
