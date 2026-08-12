#!/usr/bin/env python

import argparse
from irsutils import build, iio, healthcheck
from importlib.metadata import version

def print_help(parser):
    parser.print_help()
    exit(0)

def main():
    parser = argparse.ArgumentParser(description="Toolset for ADC related functions")
    parser.add_argument("-v", "--version", action="version", version=version("irsutils"))
    parser.set_defaults(func=lambda _: print_help(parser))

    subparsers = parser.add_subparsers(dest="command", required=False)

    build_parser = subparsers.add_parser("build", help="Show FPGA build information")
    build_parser.set_defaults(func=build.info)

    ### IIO PARSER

    iio_parser = subparsers.add_parser("iio", help="IIO related functions")

    iio_parser.add_argument("--reload", action="store_true", help="Reload ad7606_spi module using modprobe")
    iio_parser.add_argument("--read", nargs=2, metavar=("BUFFER_SIZE", "SAMPLE_RATE"), type=int, help="Read data from ADC")
    #  iio_parser.add_argument("--serve", nargs=2, metavar=("BUFFER_SIZE", "SAMPLE_RATE"), type=int, help="Read and serve data from ADC")
    iio_parser.add_argument("--serve", action="store_true", help="Serve data from ADC")
    iio_parser.add_argument("--iot", action="store_true", help="Send data to IoT hub")
    iio_parser.add_argument("--uri", type=str, help="Specify device uri. Default: local")
    iio_parser.add_argument("--device-name", type=str, help="Specify device name")
    iio_parser.add_argument("--sample-rate", type=int, help="Specify sample rate when sampling data")
    iio_parser.add_argument("--buffer-size", type=int, help="Specify buffer size when sampling data")
    iio_parser.set_defaults(func=iio.run)


    ### HEALTHCHECK

    healthcheck_parser = subparsers.add_parser("healthcheck", help="Run a quick healthcheck")
    healthcheck_parser.add_argument("--filter", type=str)
    healthcheck_parser.add_argument("--ci", type=str, help="Tell the healthcheck that it runs in a CI. Argument is the webhook token")
    healthcheck_parser.add_argument("--webhook", type=str, nargs=1, help="Tell the healthcheck that it runs in a CI. Changes the output format")
    healthcheck_parser.set_defaults(func=healthcheck.run)

    # Parse arguments
    args = parser.parse_args()

    # Run corresponding function
    args.func(args)

if __name__ == "__main__":
    main()
