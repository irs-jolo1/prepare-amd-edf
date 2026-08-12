
def run(args):
    if args.reload:
        reload()

    if args.read:
        from irsutils.utils import adc_read
        buffer_size, sample_rate = args.read
        adc_read(buffer_size=buffer_size, sample_rate=sample_rate, uri=args.uri, device_name=args.device_name)

    elif args.serve:
        from irsutils.server import run_server
        run_server(args)

    elif args.iot:
        from irsutils.iothub import send_iot
        send_iot(args)



def reload():
    from irsutils.utils import run_cmd
    print("Reloading ad7606_spi module using modprobe")
    run_cmd("modprobe -r ad7606_spi")
    run_cmd("modprobe ad7606_spi")
    print("Reloading ad7606_spi module: done")
