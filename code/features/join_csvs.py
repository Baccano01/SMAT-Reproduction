"""
Use this script to do an inner join on multiple CSV files
that have the column 'Matrix' in common.

Author: Jofeth Abello
"""

from sys import exit
import pandas as pd
import glob
import argparse


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="join_csvs",
        description="""performs inner join on multiple CSV files
        that have the col 'Matrix' in common""",
    )

    parser.add_argument(
        "target", type=str,
        help="target dir where CSVs are found"
    )

    args = parser.parse_args()

    files = glob.glob(f"{args.target}/*.csv")
    if len(files) == 0:
        print("Error: files is empty")
        exit(1)

    df = pd.read_csv(files[0])
    for f in files[1:]:
        df = pd.merge(df, pd.read_csv(f), how="inner", on="Matrix",
                      validate="one_to_one")

    print(df.to_csv(index=False, header=True, encoding="utf8"))
