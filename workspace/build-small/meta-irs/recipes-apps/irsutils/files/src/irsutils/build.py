
def info(args):
    from irsutils.utils import run_cmd
    from datetime import datetime

    timestamp = run_cmd("devmem 0x80040000")

    build_dt = datetime.fromtimestamp(int(timestamp, 16))
    today = datetime.today()

    days_since_build = (today - build_dt).days
    print(f"Build Date: {build_dt.strftime('%d.%m.%Y %H:%M:%S')} ({days_since_build} days)")

