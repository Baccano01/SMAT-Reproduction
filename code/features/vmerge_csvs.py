"""
Use this script for merging CSV files with the same header
vertically (i.e. adding rows together to form a single bigger CSV).

Author: Jofeth Abello
"""


from sys import exit
import pandas as pd
import glob
import argparse


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="vmerge",
        description="merges CSV files with the same header vertically"
    )

    parser.add_argument("target", type=str, help="target dir where CSV files are found")

    args = parser.parse_args()

    dfs = [pd.read_csv(f) for f in glob.glob(f"{args.target}/*.csv")]
    if len(dfs) == 0:
        print("Error: 'dfs' is empty")
        exit(1)

    df = pd.concat(dfs, ignore_index=True)
    print(df.to_csv(index=False, header=True))
