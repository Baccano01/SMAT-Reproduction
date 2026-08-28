import ssgetpy
import csv

MATRICES_COUNT = 2904
results = ssgetpy.search(
    dtype="real", rowbounds=(None, 10 ** 5),
    colbounds=(None, 10 ** 5), nzbounds=(None, 10 ** 6),
    limit=MATRICES_COUNT
)

with open("manifest.csv", "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(['id', 'group', 'name', 'url', 'nnz', 'row', 'cols'])
    for r in results:
        url = f"https://sparse.tamu.edu/MM/{r.group}/{r.name}.tar.gz"
        writer.writerow([r.id, r.group, r.name, url, r.nnz, r.rows, r.cols])

print(f"{len(results)} matrices queued")
