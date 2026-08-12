from statistics import mean
import requests

from irsutils.adc import ADC

API_KEY = "dk_77907b145646f6d7faae6ef275dadc4e865e3cacac9896c2" # WARN: Remove API key
API_URL = "https://api.irs.systems/functions/v1/iot-ingest-telemetry";

def send_iot(args):
    print("Preparing ADC setup")

    print("Checking if all 4 needed parameters are set")

    assert args.device_name, "Please specify --device-name <device_name>"
    assert args.uri, "Please specify --uri <uri>"
    assert args.buffer_size, "Please specify --buffer-size <size>"
    assert args.sample_rate, "Please specify --sample-rate <rate>"

    print("All done!")

    print("Sending data to IoT hub")

    adc = ADC(args.uri, args.device_name)
    data = adc.read(buffer_size=args.buffer_size, sample_rate=args.sample_rate)

    voltages = { k: dict(max=max(v), min=min(v), mean=mean(v)) for k,v in data.items() }

    data = dict(
        voltages=voltages,
        unit="mV"
    )

    headers = {
        "Content-Type": "application/json",
        "x-api-key": API_KEY,
    }

    try:
        print("Sending payload:", data)
        response = requests.post(API_URL, json=data, headers=headers)

        print("Status Code:", response.status_code)
        print("Response Body:", response.text)

    except requests.exceptions.RequestException as e:
        print("HTTP Error:", e)
