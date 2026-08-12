cat /proc/interrupts
dmesg | grep '###'
iio_info
journalctl -u irsutils -f
modprobe -r spi_axi_spi_engine && modprobe spi_axi_spi_engine && systemctl restart iiod
cd /sys/bus/iio/devices/iio\:device1/
ip addr add 192.168.10.2/24 dev end0
iio_attr -d adc_aggregator
iio_attr -c adc_aggregator voltage0 raw
iio_attr -c adaq7768-1 voltage0 raw
