"""
Use this script for merging CSV files with the same header
vertically (i.e. adding rows together to form a single bigger CSV).

Author: Jofeth Abello
"""


from sys import exit
import pandas as pd
import glob


if __name__ == "__main__":
    dfs = [pd.read_csv(f) for f in glob.glob("*.csv")]
    if len(dfs) == 0:
        print("Error: 'dfs' is empty")
        exit(1)

    df = pd.concat(dfs, ignore_index=True)
    print(df.to_string(index=False))
