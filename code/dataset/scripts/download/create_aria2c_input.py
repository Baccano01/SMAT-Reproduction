import csv

with open("manifest.csv") as f, open("aria2_input.txt", "w") as out:
    reader = csv.DictReader(f)
    for row in reader:
        out.write(f"{row['url']}\n")
        out.write(f"  dir=matrices/{row['group']}\n")
        out.write(f"  out={row['name']}.tar.gz\n")
