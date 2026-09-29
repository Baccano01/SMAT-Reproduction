from pathlib import Path
import csv

from concurrent.futures import (
    ProcessPoolExecutor,
    wait,
    FIRST_COMPLETED
)

import numpy as np
import powerlaw
from scipy.io import mmread
from scipy.sparse import issparse, csr_matrix

import argparse


N_PROCESSES = 1
P_THRESHOLD = 0.05


def analyze_matrix(path, isDebug=False):
    """
    Read one Matrix Market matrix, build an undirected graph
    from its sparsity pattern, and return the power-law exponent
    alpha only if power law is significantly favored over exponential.

    Note: self-loops and isolated vertices are maintained.

    Returns:
        alpha, or "inf".
    """

    A = mmread(path, spmatrix=True)

    if not issparse(A):
        A = csr_matrix(A)
    else:
        A = A.tocsr()

    # not a square matrix
    if A.shape[0] != A.shape[1]:
        return "inf"

    A.eliminate_zeros()

    # we only need to know whether an entry is nonzero
    S = A.copy()
    S.data[:] = 1

    # create undirected graph
    G = S.maximum(S.T)

    degrees = np.asarray(G.getnnz(axis=1)).ravel()

    if len(degrees) < 2:
        return "inf"

    try:
        fit = powerlaw.Fit(
            degrees,
            discrete=True,
            verbose=False,
            # estimate_discrete=True
        )

        alpha = fit.power_law.alpha

        if not np.isfinite(alpha):
            return "inf"

        R, p = fit.distribution_compare(
            "power_law",
            "exponential"
        )

        if not np.isfinite(R) or not np.isfinite(p):
            return "inf"

        # check if power law is a better fit than exponential
        if R > 0 and p < P_THRESHOLD:
            return alpha

    except Exception as e:
        if isDebug:
            raise e
    return "inf"


def parallel_analysis(files, writer):
    """
    Asynchronous and multiprocess execution
    of analyze_matrix with dynamic scheduling.
    """
    total = len(files)
    with ProcessPoolExecutor(max_workers=N_PROCESSES) as executor:
        pending = {}
        file_iter = iter(files)

        for _ in range(min(N_PROCESSES, total)):
            path = next(file_iter)
            future = executor.submit(
                    analyze_matrix,
                    path,
                    isDebug=False,
            )

            pending[future] = path

        completed = 0

        while pending:

            done, _ = wait(
                pending,
                return_when=FIRST_COMPLETED
            )

            for future in done:

                path = pending.pop(future)

                try:
                    alpha = future.result()
                except Exception as e:
                    print(
                        f"Error processing {path}: {e}"
                    )
                    alpha = "inf"

                writer.writerow([
                    str(path),
                    alpha,
                ])

                completed += 1
                print(
                    f"[{completed}/{total}] "
                    f"Processed: {path}"
                )

                # give another work
                try:
                    next_path = next(file_iter)

                    next_future = executor.submit(
                            analyze_matrix,
                            next_path,
                            isDebug=False
                    )

                    pending[next_future] = next_path

                except StopIteration:
                    pass




def main(in_path, out_path):
    ins = Path(in_path)
    out = Path(out_path)

    files = []
    with ins.open("r", newline="", encoding="utf8") as f:
        for line in f:
            files.append(line.strip())

    total = len(files)
    print(f"total={total}")
    with out.open("w", newline="", encoding="utf8") as f:
        writer = csv.writer(f)
        writer.writerow(["Matrix", "alpha"])

        if total == 0:
            return

        parallel_analysis(files, writer)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="PowerLawFactor",
        description="Computes the power law exponential factor"
    )

    parser.add_argument("src", help="src CSV path containing Matrix Market paths")
    parser.add_argument("dest", help="dest path of the output CSV file")

    args = parser.parse_args()
    main(args.src, args.dest)
