import adi
import subprocess

def run_cmd(cmd):
    return subprocess.check_output(cmd, shell=True).decode().strip()

def adc_read(*, buffer_size=10, sample_rate=100_000, uri=None, device_name=None):

    try:
        if uri:
            adc = adi.ad7606(uri=uri, device_name=device_name)
        else:
            adc = adi.ad7606(device_name=device_name)
    except Exception as e:
        print(f"Error: Could not connect to IIO device: {e}")
        exit(1)

    adc.rx_buffer_size = buffer_size
    adc.sample_rate = sample_rate
    adc.rx_annotated = True
    adc.rx_output_type = "SI" # convert raw values to Volts

    print(f"Sampling data. buffer size: {buffer_size}, sample rate: {sample_rate}")

    samples = adc.rx()

    return { k: v.tolist() for k,v in samples.items() }
