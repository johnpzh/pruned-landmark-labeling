import sys

def average():
    if len(sys.argv) < 3:
        print("Usage: python3 average_speed.py <input_file> <output_file>")
        sys.exit(1)
    with open(sys.argv[1], 'r') as fin, \
         open(sys.argv[2], 'w') as fout:
        count = 0
#         label_sum = 0
#         is_new_group = True
        time_last = 0
        label_last = 0
        for line in fin:
            count += 1
            columns = line.split()
            if (columns[0] == 'PLL'):
                break
#             label_sum += int(columns[-2])
            label = int(columns[-2])
            time = float(columns[-1])
#             if is_new_group:
#                 is_new_group = False
#                 time_start = time
#                 label_start = label
            if count % 10**3 == 0:
                time_range = time - time_last
                label_range = label - label_last
                average_speed = label_range / time_range
                time_last = time
                label_last = label
#                 label_sum = 0
#                 is_new_group = True
                fout.write("{} {}\n".format(time, average_speed))
                print("{} {}\n".format(time, average_speed))
                
                
            

if __name__ == '__main__':
    average()
