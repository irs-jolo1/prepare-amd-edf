import adi

class ADC:
    uri: str
    device_name: str
    adc: adi.ad7606


    def __init__(self, uri, device_name):
        self.uri = uri
        self.device_name = device_name
        if "7606" in device_name:
            self.adc = adi.ad7606(uri=uri, device_name=device_name)
        elif "7768" in device_name:
            self.adc = adi.ad7768(uri=uri, device_name=device_name)
        else:
            print(f"Error unknown device: {device_name}")

        self.adc.rx_annotated = True
        self.adc.rx_output_type = "SI" # convert raw values to Volts


    def read(self, buffer_size, sample_rate):

        self.adc.rx_buffer_size = buffer_size
        self.adc.sampling_frequency = sample_rate

        samples = self.adc.rx()

        x = { k: v.tolist() for k,v in samples.items() }
        return x

