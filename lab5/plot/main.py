import sys

import matplotlib.pyplot as plt
import pandas as pd


def main():
    if len(sys.argv) < 2:
        print("Usage: plot.py <csv file>")
        return

    df = pd.read_csv(sys.argv[1])
    df.plot.line(x=0, y=1)
    plt.show()


if __name__ == "__main__":
    main()
