import argparse
import re
import subprocess
import traceback
import gzip
import iio
import numpy as np
from collections import defaultdict
import requests
from io import StringIO
import sys
import json
from pathlib import Path
from typing import Callable, List, Optional
from packaging.version import Version
from irsutils.utils import run_cmd
from datetime import datetime

counter = 0
success = 0
failed = 0

tests: List[Callable[[int, Optional[str]], None]] = []

RED = 31
GREEN = 32

def colored(text: str, code: int, ci: Optional[str]) -> str:
    """Wrap text in an ANSI color code, unless running in CI mode (ci is truthy)."""
    if ci:
        return text
    return f"\033[{code}m{text}\033[0m" # }]

# function decoration for catching and counting errors
def check(f: Callable[[], None]) -> None:
    """Register `f` as a health check, wrapping it so failures are caught, counted, and reported."""
    global counter
    counter += 1

    func_name = f.__name__

    def test_func(i: int, ci: Optional[str]) -> None:
        """Run the wrapped check, printing progress and success/failure status."""
        print(f"{i + 1}/{counter} - Checking {func_name}")
        if ci:
            print(f"{i + 1}/{counter} - Checking {func_name}", file=sys.stderr) ## also print to stderror

        global success, failed
        try:
            #  add test to queue
            f()
            success += 1
            print(colored("Test passed!", GREEN, ci))
            print()
        except Exception as e:
            failed += 1
            error_label = colored("ERROR:", RED, ci)
            tb = traceback.format_exc()
            print(f"{error_label} in function '{func_name}':\n\n{e}\n\n{tb}")

    test_func.__name__ = func_name ## change function name. needed for function filter

    tests.append(test_func)

@check
def check_iio_info() -> None:
    """Verify that the installed `iio_info` (libiio) version meets the minimum required version."""
    required_version = Version("0.25")

    try:
        result = subprocess.run(
            ["iio_info", "-V"],
            capture_output=True,
            text=True,
            timeout=10,
        )
    except FileNotFoundError:
        raise RuntimeError("iio_info binary not found")
    except subprocess.TimeoutExpired:
        raise RuntimeError("iio_info -V timed out")

    match = re.search(r"iio_info version:\s*([0-9.]+)", result.stdout) #])

    if not match:
        raise RuntimeError("Could not detect iio_info version")

    installed_version = Version(match.group(1))
    if installed_version < required_version:
        raise AssertionError(f"Libiio version ({installed_version}) is below required version({required_version})")

    print(f"The libiio version is {installed_version} (matches {required_version})")

@check
def check_irsutils_backend() -> None:
    """Verify that the stream-server-rs process is running."""
    process_name = "stream-server-rs"
    try:
        result = subprocess.run(
            ["ps"],
            capture_output=True,
            text=True,
            timeout=10,
        )
    except FileNotFoundError:
        raise RuntimeError("ps binary not found")
    except subprocess.TimeoutExpired:
        raise RuntimeError("ps timed out")

    assert result.returncode == 0, f"ps failed: {result.stderr.strip()}"

    # busybox ps output columns are: PID USER VSZ STAT COMMAND
    # The COMMAND column is the last field and may include arguments.
    running = False
    for line in result.stdout.splitlines()[1:]:
        parts = line.split(None, 4)
        if len(parts) < 5:
            continue
        command = parts[4]
        # First token of COMMAND is the executable (possibly a path).
        exe = command.split()[0].rsplit("/", 1)[-1]
        if exe == process_name:
            running = True
            break

    assert running, f"The streaming server ({process_name}) is not running"

    print(f"The streaming server ({process_name}) is running")

@check
def check_enabled_modules() -> None:
    """Verify that the required kernel config options are enabled (built-in or as modules)."""
    modules = [
        "CONFIG_AD7606_IFACE_SPI,CONFIG_AD7768_1",
        "CONFIG_SPI_AXI_SPI_ENGINE",
        "CONFIG_PWM_AXI_PWMGEN",
        "CONFIG_AXI_DMAC",
        "CONFIG_COMMON_CLK_AXI_CLKGEN",
    ]

    enabled_modules = {}
    with gzip.open('/proc/config.gz', 'rt') as f:
        for line in f:
            line = line.strip()

            if not line or line.startswith("#"):
                continue

            mod, val = line.split("=",1)
            enabled_modules[mod] = val

    result = {mods: any([m in enabled_modules and enabled_modules[m] in ["y", "m"] for m in mods.split(",")]) for mods in modules}

    #  assert all([b for b in result.values()]), f"Not all modules enabled correctly:\n{json.dumps(result, indent=2)}"
    if not all(result.values()):
        failed = [k for k, v in result.items() if not v]
        raise AssertionError(
            "Not all modules *enabled* correctly:\n"
            f"Failed modules: {failed}\n"
        )

    print("All expected kernel modules are enabled")

@check
def check_loaded_modules() -> None:
    """Verify that the required kernel modules are currently loaded (present in /proc/modules)."""
    modules = [
        "ad7606,ad7768_1",
        "adc_aggregator",
        "spi_axi_spi_engine",
    ]

    with open("/proc/modules", "r") as f:
        loaded_modules = {line.split()[0] for line in f}

    result = {mods: any([m in loaded_modules for m in mods.split(",")]) for mods in modules}

    if not all(result.values()):
        failed = [k for k, v in result.items() if not v]
        raise AssertionError(
            "Not all modules *loaded* correctly:\n"
            f"Failed modules: {failed}\n"
        )

    print("All expected kernel modules are loaded")

def connect_iio_device() -> List["iio.Device"]:
    """Connect to the local IIO context and return the adc_aggregator device(s) with all scan channels enabled."""
    try:
        ctx = iio.Context("local:")
    except Exception as e:
        raise RuntimeError(f"Could not create IIO Context: {e}")

    devices = [
        "adc_aggregator",
    ]

    found_devs = []

    for d in devices:
        dev = ctx.find_device(d)
        if dev:
            found_devs.append(dev)

    if len(found_devs) == 0:
        raise RuntimeError("No matching IIO device found")

    # enable all channels, needed for dma read
    for dev in found_devs:
        for ch in dev.channels:
            if ch.scan_element:
                ch.enabled = True

    return found_devs

@check
def check_read_raw() -> None:
    """Verify every ADC channel with a 'raw' attribute can be read and does not return an error value."""
    devs = connect_iio_device()

    errors = []
    found = False
    for dev in devs:
        for ch in dev.channels:
            if "raw" in ch.attrs:
                found = True
                ch.enabled = True
                value = ch.attrs['raw'].value

                if value is None or value == "-1":
                    errors.append((dev, ch, value))

    if not found:
        raise AssertionError("No channels with raw values found")

    if len(errors) > 0:
        details = "\n - ".join(f"Got {value} on {dev} ({ch.id})" for dev, ch, value in errors)
        raise AssertionError(f"Could not read raw values:\n - {details}")

    print("ADC Aggregator can raw-read values from all ADCs")

@check
def check_read_dma() -> None:
    """Verify the adc_aggregator can stream non-zero data over DMA via an IIO buffer."""
    devs = connect_iio_device()

    data = []
    found = False


    for dev in devs:

        if dev.name != "adc_aggregator":
            continue

        found = True

        try:
            buf = iio.Buffer(dev, 10) # read 10 values
        except Exception as e:
            raise RuntimeError(f"Could not create buffer: {e}")

        try:
            buf.refill()
        except Exception as e:
            raise RuntimeError(f"Could not refill buffer: {e}")

        for ch in dev.channels:
            name = ch.id
            chan = dev.find_channel(name)
            if chan is None:
                raise AssertionError(f"Channel '{name}' not found on device '{dev.name}'")
            bytearray_data = chan.read(buf)  # Do local type conversion
            # create format strings
            df = chan.data_format
            fmt = ("i" if df.is_signed is True else "u") + str(df.length // 8)
            fmt = ">" + fmt if df.is_be else fmt
            data += np.frombuffer(bytearray_data, dtype=fmt).tolist()

    if not found:
        raise AssertionError("The adc_aggregator was not found")

    if all([d == 0 for d in data]):
        raise AssertionError("Read values from DMAC, but they are all 0")

    print("ADC Aggregator can read values from all ADCs over DMA")

@check
def check_fpga_build_info() -> None:
    """Verify the number of ADC IIO devices matches num_spi_engines from the FPGA build info JSON."""
    # read buildinfo
    JSON_FILE = "/root/fpga_build_info.json"

    with open(JSON_FILE, "r") as file:
        fpga_build_info = json.load(file)

    spi_engine_num = fpga_build_info["modules"]["num_spi_engines"]
    adc_type = fpga_build_info["config"]["adc_type"]

    try:
        ctx = iio.Context("local:")
    except Exception as e:
        raise RuntimeError(f"Could not create IIO Context: {e}")


    num_adc = len(list(filter(lambda x: x.name == adc_type, ctx.devices)))

    assert spi_engine_num == num_adc, f"IIO devices do not match build info: expected: {spi_engine_num}, found: {num_adc}"

    print(f"Number of ADC IIO devices ({num_adc}) match num_spi_engines from .xsa build info ({spi_engine_num})")

def read_timestamp_register() -> Optional[int]:
    """Read and decode the FPGA timestamp register, as done in the `irsutils build` command."""

    address = "0x80040000"

    try:
        timestamp = run_cmd(f"devmem {address}")
        build_dt = datetime.fromtimestamp(int(timestamp, 16))
        return int(build_dt.timestamp())
    except Exception as e:
        print(f"Could not read timestamp register at {address}: {e}")
        return None

def read_fpga_build_info(path: str = "/root/fpga_build_info.json") -> Optional[dict]:
    """Read and parse the FPGA build info JSON file, returning None if it does not exist or is invalid."""
    json_path = Path(path)

    if not json_path.exists():
        return None

    try:
        with json_path.open("r") as f:
            return json.load(f)
    except Exception as e:
        print(f"Could not read FPGA build info file '{path}': {e}")
        return None

def run(args: argparse.Namespace) -> None:
    """Run all registered health checks (optionally filtered), report results, and exit with a status code.

    If `args.ci` is set, output is buffered and posted, along with dmesg output, to the CI webhook,
    and progress is additionally mirrored to stderr. Exits 0 if all checks passed, 1 otherwise.
    """
    # run all tests
    filter = str(args.filter) if args.filter else None

    buffer = None
    output = None
    if args.ci:
        buffer = StringIO()

        # store output in buffer
        sys.stdout = buffer

    t = [ f(i, args.ci) for i, f in enumerate(tests) if filter is None or filter in f.__name__ ]

    if buffer is not None:
        output = buffer.getvalue()

    # restore stdout output
    sys.stdout = sys.__stdout__

    print(f"\nRESULT: {len(t)} total checks. {colored(failed, RED, args.ci)} failed and {colored(success, GREEN, args.ci)} succeeded!")

    if args.ci:
        url = f"http://192.168.150.192:8080/webhook-step/{args.ci}"
        print(f"Sending report to {url}")

        try:
            result = subprocess.run(
                ["dmesg"],
                capture_output=True,
                text=True,
                timeout=10,
            )
            dmesg_output = result.stdout
        except (FileNotFoundError, subprocess.TimeoutExpired) as e:
            dmesg_output = f"Could not collect dmesg output: {e}"

        payload = dict(
            report=output,
            failed=failed,
            success=success,
            dmesg=dmesg_output,
            timestamp_register=read_timestamp_register(),
            fpga_build_info=read_fpga_build_info(),
        )

        try:
            response = requests.post(url, json=payload, timeout=30)
            print("Status code:", response.status_code)
            print("Response:", response.text)
        except requests.RequestException as e:
            print(f"Failed to send report to {url}: {e}")


    if failed > 0:
        sys.exit(1)
    else:
        sys.exit(0)
