import argparse
from typing import cast

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
    exec_path = workdir / "build" / "Release" / "nonogram"

    test_files = sorted(test_data_dir.iterdir())

    for test_file in test_files:
        assert test_file.is_file()
        if test_file.name.startswith("_"):
            continue
        test_dist = []
        for _ in range(args.n_iter):
            cmd = local[exec_path][test_file]["-q"]["-b"]
            fut = cmd.run_bg()
            fut.wait()
            # fut.stdout = "solve_puzzle took X ns"
            stdout = fut.stdout
            assert stdout is not None
            stdout = cast(str, stdout)
            bench_time = int(stdout.removeprefix("solve_puzzle took ").split(" ")[0])
            test_dist.append(bench_time)

        t_mid = median(test_dist)
        t_std = round(std(test_dist))
        print(f"{test_file.name}: q50={t_mid:,}ns std={t_std:,}ns")


if __name__ == "__main__":
    main()
