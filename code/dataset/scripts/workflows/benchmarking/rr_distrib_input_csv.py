import csv
import sys

NUM_FILES = 32

dest_dir = input("Enter dest dir path: ")
input_file = input("Enter csv file: ")
base_name = input_file.removesuffix(".csv").removeprefix("../")

with open(input_file, "r", encoding="utf-8") as f:
    reader = csv.reader(f)
    header = next(reader, None)


    files = [open(f"{dest_dir}/{base_name}_{i}.csv", "w", encoding="utf-8") for i in range(NUM_FILES)]
    writers = [csv.writer(file) for file in files]

    if header:
        for w in writers:
            w.writerow(header)

        for i, row in enumerate(reader):
            writers[i % NUM_FILES].writerow(row)

    for file in files:
        f.close()
