import argparse
import time

from plumbum import local
from pathlib import Path


def median(dist):
    med_idx = (len(dist) + 1) // 2
    return sorted(dist)[med_idx]


def mean(dist):
    return sum(dist) / len(dist)


def var(dist):
    d_mean = mean(dist)
    return mean([(v - d_mean) ** 2 for v in dist])


def std(dist):
    return var(dist) ** 0.5


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-n", "--n-iter", type=int, required=False, default=10)
    args = parser.parse_args()

    workdir = Path(__file__).parent
    test_data_dir = workdir / "test_data"
    exec_path = workdir / "build" / "nonogram"

    test_files = sorted(test_data_dir.iterdir())

    for test_file in test_files:
        assert test_file.is_file()
        if test_file.name.startswith("_"):
            continue
        test_dist = []
        for _ in range(args.n_iter):
            start_ts = time.time_ns()
            cmd = local[exec_path][test_file]["-q"]
            cmd.run_fg()
            end_ts = time.time_ns()
            test_dist.append(end_ts - start_ts)

        t_mid = median(test_dist)
        t_std = round(std(test_dist))
        print(f"{test_file.name}: q50={t_mid:,}ns std={t_std:,}ns")


if __name__ == "__main__":
    main()
