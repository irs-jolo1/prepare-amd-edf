// SPDX-License-Identifier: GPL-2.0
/*
 * IRS ADC Aggregator driver
 *


    ┌─────┐                           ┌──────┐
    │ADC 1│      ────┬─────────────►  │DMAC 1│
    └─────┘          │                └──────┘
                     │
    ┌─────┐          │                ┌──────┐
    │ADC 2│      ────┼──┬──────────►  │DMAC 2│
    └─────┘          │  │             └──────┘
                     │  │
    ┌──────────┐    ┌▼──▼───┐         ┌─────────┐
    │AGGREGATOR│────│  MUX  ├──────►  │DMAC Main│
    └──────────┘    └───────┘         └─────────┘


 * Copyright 2026 IRS Systems GmbH
 */
#define pr_fmt(fmt) "adc_aggregator: " fmt
#define ADC_AGGREGATOR_MAX_REGISTER		(0x26 << 2)	/* word 0x26, byte 0x98 */

/* Register addresses (byte offsets: word_index × 4) */
#define ADC_AGGREGATOR_REG_VERSION		0x00	/* word 0x00 */
#define ADC_AGGREGATOR_REG_SCRATCH		0x08	/* word 0x02 */
#define ADC_AGGREGATOR_REG_SW_RESET		0x40	/* word 0x10 */
#define ADC_AGGREGATOR_REG_CONTROL		0x44	/* word 0x11 */
#define ADC_AGGREGATOR_REG_STATUS		0x48	/* word 0x12 */
#define ADC_AGGREGATOR_REG_FIFO_LEVEL		0x4C	/* word 0x13 */
#define ADC_AGGREGATOR_REG_FIFO_ROOM		0x50	/* word 0x14 */
#define ADC_AGGREGATOR_REG_FRAME_COUNTER	0x54	/* word 0x15 */
#define ADC_AGGREGATOR_REG_CHANNEL_CONFIG	0x58	/* word 0x16 */
#define ADC_AGGREGATOR_REG_IRQ_MASK		0x80	/* word 0x20 */
#define ADC_AGGREGATOR_REG_IRQ_PENDING		0x84	/* word 0x21 */
#define ADC_AGGREGATOR_REG_IRQ_SOURCE		0x88	/* word 0x22 */

/* GPIO regmap register addresses (byte offsets: word_index × 4) */
#define ADC_AGGREGATOR_GPIO_REG_DATA0		0x00	/* word 0x00: GPIO 00-31 */
#define ADC_AGGREGATOR_GPIO_REG_DATA1		0x04	/* word 0x01: GPIO 32-63 */
#define ADC_AGGREGATOR_GPIO_REG_DELAY_TICKS	0x08	/* word 0x02: delay ticks */
#define ADC_AGGREGATOR_GPIO_REG_DELAY_RESOLUTION 0x0C	/* word 0x03: delay resolution */

/* IRQ_SOURCE / IRQ_MASK / IRQ_PENDING bit definitions */
#define ADC_AGGREGATOR_IRQ_ALMOST_FULL		BIT(0)	/* level: FIFO room < ALMOST_FULL_THRESHOLD (mirrors STATUS bit2); not latched */
#define ADC_AGGREGATOR_IRQ_OVERFLOW		BIT(1)	/* sticky W1C: combined beat lost while FIFO full; mirrors STATUS.overflow_sticky (bit4) */
#define ADC_AGGREGATOR_IRQ_FLUSH_DONE		BIT(2)	/* sticky W1C: flush_partial command completed */
#define ADC_AGGREGATOR_IRQ_FIFO_RESET_DONE	BIT(3)	/* sticky W1C: fifo_reset command completed */
#define ADC_AGGREGATOR_IRQ_FRAME_READY		BIT(4)	/* sticky W1C: FIFO transitioned empty -> non-empty */
#define ADC_AGGREGATOR_IRQ_ENGINE_IRQ		BIT(5)	/* level: |(engine_irq_source & engine_irq_mask); reads 0 when ENGINE_IRQ_EN=0; not latched */

/* VERSION register: bits[31:16] = major, bits[15:0] = minor */
#define ADC_AGGREGATOR_VERSION_MAJOR_MASK	GENMASK(31, 16)
#define ADC_AGGREGATOR_VERSION_MAJOR(v)		FIELD_GET(ADC_AGGREGATOR_VERSION_MAJOR_MASK, (v))
#define ADC_AGGREGATOR_DRIVER_VERSION_MAJOR	1U

/* CHANNEL_CONFIG register: bits[7:0] hold NUM_CHANNELS */
#define ADC_AGGREGATOR_CHANNEL_CONFIG_NUM_CH_MASK	GENMASK(7, 0)
#define ADC_AGGREGATOR_CTRL_ENABLE		BIT(0)
#define ADC_AGGREGATOR_CTRL_FLUSH_PARTIAL	BIT(1)
#define ADC_AGGREGATOR_CTRL_FIFO_RESET		BIT(2)

/* STATUS register bit definitions */
#define ADC_AGGREGATOR_STATUS_FIFO_FULL		BIT(0)	/* level: FIFO is full (write-side saturation) */
#define ADC_AGGREGATOR_STATUS_FIFO_EMPTY	BIT(1)	/* level: FIFO is empty (no complete frame available to DMA) */
#define ADC_AGGREGATOR_STATUS_ALMOST_FULL	BIT(2)	/* level: FIFO room < ALMOST_FULL_THRESHOLD */
#define ADC_AGGREGATOR_STATUS_ALMOST_EMPTY	BIT(3)	/* level: FIFO fill < ALMOST_EMPTY_THRESHOLD */
#define ADC_AGGREGATOR_STATUS_OVERFLOW_STICKY	BIT(4)	/* sticky (W1C via IRQ_PENDING bit1): combined beat lost while FIFO full */
#define ADC_AGGREGATOR_STATUS_FLUSH_BUSY	BIT(5)	/* level: flush_partial command in flight */
#define ADC_AGGREGATOR_STATUS_FIFO_RESET_BUSY	BIT(6)	/* level: fifo_reset command in flight */

/* SW_RESET: write BIT(0) to force hardware defaults */
#define ADC_AGGREGATOR_SW_RESET_BIT		BIT(0)

/* Scratch bus-test pattern */
#define ADC_AGGREGATOR_SCRATCH_TEST_VAL		0xA5A5A5A5U

#include <linux/bitfield.h>
#include <linux/spi/offload/types.h>
#include <linux/spi/spi.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/log2.h>
#include <linux/iio/buffer_impl.h>
#include <linux/iio/iio.h>
#include <linux/iio/buffer-dmaengine.h>
#include <linux/iio/adc/ad7768.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/completion.h>
#include <linux/interrupt.h>

/* AD7768_MULTI registers definition */

struct adc_aggregator_chan_map {
  struct iio_dev *client;
  unsigned int chan_idx;
};

struct adc_aggregator_client {
  struct iio_dev *indio_dev;
  struct device *dev;
};

struct adc_aggregator_state {
	struct device *dev;
	struct adc_aggregator_client *clients;
  unsigned int num_clients;

  struct iio_chan_spec *channels;
  struct adc_aggregator_chan_map *chan_map;
  unsigned int num_channels;
  struct regmap *regmap;
  struct completion completion_flush;
  struct completion completion_reset;
  int irq;

  struct regmap *regmap_gpio;
};

static const struct iio_scan_type adc_aggregator_scan_type_offload = {
  .sign = 's',
  .realbits = 24,
  .storagebits = 32,
  .endianness = IIO_CPU,
};

static const struct iio_scan_type adc_aggregator_scan_type_offload_timestamp = {
  .sign = 's',
  .realbits = 32,
  .storagebits = 32,
  .endianness = IIO_CPU,
};

/*
 * Wrappers that forward the filter type get/set to the underlying ad7768-1
 * clients. The aggregator's iio_priv() points to adc_aggregator_state, not
 * ad7768_state, so we cannot call the ad7768 functions directly with the
 * aggregator's iio_dev.
 */
static int adc_aggregator_set_filter_type(struct iio_dev *indio_dev,
					  const struct iio_chan_spec *chan,
					  unsigned int filter)
{
	struct adc_aggregator_state *st = iio_priv(indio_dev);
	unsigned int i;
	int ret;

	for (i = 0; i < st->num_clients; i++) {
		ret = ad7768_set_filter_type_attr(st->clients[i].indio_dev,
						  &st->clients[i].indio_dev->channels[0],
						  filter);
		if (ret)
			return ret;
	}

	return 0;
}

static int adc_aggregator_get_filter_type(struct iio_dev *indio_dev,
					  const struct iio_chan_spec *chan)
{
	struct adc_aggregator_state *st = iio_priv(indio_dev);

	if (!st->num_clients)
		return -EOPNOTSUPP;

	return ad7768_get_filter_type_attr(st->clients[0].indio_dev,
					   &st->clients[0].indio_dev->channels[0]);
}

enum adc_aggregator_filter_type {
	ADC_AGGREGATOR_FILTER_SINC5,
	ADC_AGGREGATOR_FILTER_SINC3,
	ADC_AGGREGATOR_FILTER_WIDEBAND,
	ADC_AGGREGATOR_FILTER_SINC3_REJ60,
};


static const char * const adc_aggregator_filter_enum[] = {
	[ADC_AGGREGATOR_FILTER_SINC5] = "sinc5",
	[ADC_AGGREGATOR_FILTER_SINC3] = "sinc3",
	[ADC_AGGREGATOR_FILTER_WIDEBAND] = "wideband",
	[ADC_AGGREGATOR_FILTER_SINC3_REJ60] = "sinc3+rej60",
};


static const struct iio_enum adc_aggregator_filter_type_iio_enum = {
	.items = adc_aggregator_filter_enum,
	.num_items = ARRAY_SIZE(adc_aggregator_filter_enum),
	.set = adc_aggregator_set_filter_type,
	.get = adc_aggregator_get_filter_type,
};

static const struct iio_chan_spec_ext_info adc_aggregator_ext_info[] = {
	IIO_ENUM("filter_type", IIO_SHARED_BY_ALL, &adc_aggregator_filter_type_iio_enum),
	IIO_ENUM_AVAILABLE("filter_type", IIO_SHARED_BY_ALL, &adc_aggregator_filter_type_iio_enum),
	{ }
};


// ADC_AGGREGATOR regmap config
static const struct regmap_range adc_aggregator_regmap_rd_ranges[] = {
	regmap_reg_range(ADC_AGGREGATOR_REG_VERSION,  ADC_AGGREGATOR_REG_VERSION),   /* VERSION */
	regmap_reg_range(ADC_AGGREGATOR_REG_SCRATCH,  ADC_AGGREGATOR_REG_SCRATCH),   /* SCRATCH */
	regmap_reg_range(ADC_AGGREGATOR_REG_SW_RESET, ADC_AGGREGATOR_REG_CHANNEL_CONFIG), /* SW_RESET..CHANNEL_CONFIG */
	regmap_reg_range(ADC_AGGREGATOR_REG_IRQ_MASK, ADC_AGGREGATOR_REG_IRQ_SOURCE), /* IRQ_MASK..IRQ_SOURCE */
};

static const struct regmap_access_table adc_aggregator_regmap_rd_table = {
	.yes_ranges = adc_aggregator_regmap_rd_ranges,
	.n_yes_ranges = ARRAY_SIZE(adc_aggregator_regmap_rd_ranges),
};

static const struct regmap_range adc_aggregator_regmap_wr_ranges[] = {
	regmap_reg_range(ADC_AGGREGATOR_REG_SCRATCH,   ADC_AGGREGATOR_REG_SCRATCH),  /* SCRATCH */
	regmap_reg_range(ADC_AGGREGATOR_REG_SW_RESET,  ADC_AGGREGATOR_REG_CONTROL),  /* SW_RESET, CONTROL */
	regmap_reg_range(ADC_AGGREGATOR_REG_IRQ_MASK,  ADC_AGGREGATOR_REG_IRQ_PENDING), /* IRQ_MASK, IRQ_PENDING (W1C) */
};

static const struct regmap_access_table adc_aggregator_regmap_wr_table = {
	.yes_ranges = adc_aggregator_regmap_wr_ranges,
	.n_yes_ranges = ARRAY_SIZE(adc_aggregator_regmap_wr_ranges),
};

static const struct regmap_config adc_aggregator_regmap_config = {
	.name = "adc-aggregator",
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32,
	.rd_table = &adc_aggregator_regmap_rd_table,
	.wr_table = &adc_aggregator_regmap_wr_table,
	.max_register = ADC_AGGREGATOR_MAX_REGISTER,
	.use_single_write = true,
	.use_single_read = true,
	.fast_io = true,	/* MMIO: use spinlock, safe in hard-IRQ context */
};

// GPIO regmap config
static const struct regmap_range adc_aggregator_regmap_gpio_rd_ranges[] = {
	regmap_reg_range(ADC_AGGREGATOR_GPIO_REG_DATA0, ADC_AGGREGATOR_GPIO_REG_DATA1), /* DATA0, DATA1 */
	regmap_reg_range(ADC_AGGREGATOR_GPIO_REG_DELAY_TICKS, ADC_AGGREGATOR_GPIO_REG_DELAY_RESOLUTION), /* DELAY_TICKS, DELAY_RESOLUTION */
};

static const struct regmap_access_table adc_aggregator_regmap_gpio_rd_table = {
	.yes_ranges = adc_aggregator_regmap_gpio_rd_ranges,
	.n_yes_ranges = ARRAY_SIZE(adc_aggregator_regmap_gpio_rd_ranges),
};

static const struct regmap_range adc_aggregator_regmap_gpio_wr_ranges[] = {
	regmap_reg_range(ADC_AGGREGATOR_GPIO_REG_DELAY_TICKS, ADC_AGGREGATOR_GPIO_REG_DELAY_RESOLUTION), /* DELAY_TICKS, DELAY_RESOLUTION */
};

static const struct regmap_access_table adc_aggregator_regmap_gpio_wr_table = {
	.yes_ranges = adc_aggregator_regmap_gpio_wr_ranges,
	.n_yes_ranges = ARRAY_SIZE(adc_aggregator_regmap_gpio_wr_ranges),
};
static const struct regmap_config adc_aggregator_regmap_gpio_config = {
	.name = "adc-aggregator_gpio",
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32,
	.rd_table = &adc_aggregator_regmap_gpio_rd_table,
	.wr_table = &adc_aggregator_regmap_gpio_wr_table,
	.use_single_write = true,
	.use_single_read = true,
	.fast_io = true,	/* MMIO: use spinlock, safe in hard-IRQ context */
};

static int adc_aggregator_probe_channels(struct iio_dev *indio_dev)
{
  struct adc_aggregator_state *st = iio_priv(indio_dev);
  struct device *dev = indio_dev->dev.parent;
  struct iio_chan_spec *channels, *chan;
  unsigned int n = st->num_clients;
  unsigned int total_words, num_pad, i, k;

  /*
   * axis_sample_combiner MUX frame, one fixed record per sample. The HDL
   * places ch0 at the LSB end and the AXI DMA writes the beat little-endian,
   * so as ascending 32-bit memory words (= ascending scan_index) the record
   * reads:
   *
   *   { ch0, ch1, ... , ch[n-1], frame_counter[31:0], 0..0 (zero pad) }  (LE words)
   *
   * ch i is client i (client0 = spi_ad77681_1 -> s00, ...):
   *
   *   word 0 .. n-1        : ch0 .. ch[n-1]   (client k -> voltage k)
   *   word n               : frame_counter (sequence number / timestamp)
   *   word n+1 .. total-1  : zero padding
   *
   * total_words = roundup_pow_of_two(n + 1). scan_index MUST follow this
   * word order.
   */
  total_words = roundup_pow_of_two(n + 1);
  num_pad = total_words - (n + 1);

  indio_dev->num_channels = total_words;

  channels = devm_kcalloc(dev, total_words, sizeof(*channels), GFP_KERNEL);
  if (!channels)
		return -ENOMEM;

  /*
   * word 0 .. n-1: the per-client adc channel, ascending. voltage k reads
   * client k (voltage0 = adc1, voltage1 = adc2, ...).
   *
   * This assumes each client is an ad7768-1 exposing a single voltage
   * channel. Support for other clients has to be added manually.
   */
  for (k = 0; k < n; k++) {
    chan = &channels[k];
    chan->type = IIO_VOLTAGE;
    chan->indexed = 1;
    chan->channel = k;
    chan->scan_index = k;
    chan->scan_type = adc_aggregator_scan_type_offload;

    chan->info_mask_separate_available = 0;
    chan->info_mask_separate = BIT(IIO_CHAN_INFO_RAW);
    chan->info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE) |
              BIT(IIO_CHAN_INFO_LOW_PASS_FILTER_3DB_FREQUENCY) |
              BIT(IIO_CHAN_INFO_OVERSAMPLING_RATIO);
    chan->info_mask_shared_by_type_available = BIT(IIO_CHAN_INFO_OVERSAMPLING_RATIO);
    chan->info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SAMP_FREQ);
    chan->info_mask_shared_by_all_available = BIT(IIO_CHAN_INFO_SAMP_FREQ);
    chan->ext_info = adc_aggregator_ext_info;
  }

  /*
   * word n: today this carries the frame_counter (a sequence number).
   * Exposed as IIO_TIMESTAMP because the slot will later be replaced by a
   * real hardware timestamp.
   *
   * (axis_sample_combiner CTR_WIDTH == 32). If the hardware timestamp word
   * is widened later, widen this scan_type accordingly.
   */
  chan = &channels[n];
  chan->type = IIO_TIMESTAMP;
  chan->channel = -1;
  chan->scan_index = n;
  chan->scan_type = adc_aggregator_scan_type_offload_timestamp;

  /*
   * word n+1 .. total-1: zero-padding words. Declared only so the stride
   * matches the hardware record; they expose no readable attribute. Typed
   * IIO_COUNT with extend_name "padding" so they are clearly not mistaken
   * for ADC voltage channels in sysfs / iio_info.
   */
  for (i = 0; i < num_pad; i++) {
    chan = &channels[n + 1 + i];
    chan->type = IIO_COUNT;
    chan->indexed = 1;
    chan->channel = i;
    chan->extend_name = "padding";
    chan->scan_index = n + 1 + i;
    chan->scan_type = adc_aggregator_scan_type_offload_timestamp;
  }

  indio_dev->channels = channels;

  return 0;
}

static int adc_aggregator_read_raw(struct iio_dev *indio_dev,
         struct iio_chan_spec const *chan,
         int *val, int *val2, long info)
{

  struct adc_aggregator_state *st = iio_priv(indio_dev);
  unsigned int idx = chan->channel;

  if (idx >= st->num_clients)
    return -EINVAL;

  /*
   * Each aggregated client (ad7768-1) exposes a single voltage
   * channel at index 0; forward the read to that channel.
   */
  return ad7768_read_raw(st->clients[idx].indio_dev,
                         &st->clients[idx].indio_dev->channels[0],
                         val, val2, info);
}

static int adc_aggregator_read_avail(struct iio_dev *indio_dev,
           struct iio_chan_spec const *chan,
           const int **vals, int *type, int *length,
           long info)
{
  struct adc_aggregator_state *st = iio_priv(indio_dev);

  if (!st->num_clients)
    return -EOPNOTSUPP;

  return ad7768_read_avail(st->clients[0].indio_dev,
                           &st->clients[0].indio_dev->channels[0],
                           vals, type, length, info);
}

static int adc_aggregator_write_raw_get_fmt(struct iio_dev *indio_dev,
            struct iio_chan_spec const *chan, long mask)
{
  switch (mask) {
  case IIO_CHAN_INFO_SCALE:
    return IIO_VAL_INT_PLUS_NANO;
  default:
    return IIO_VAL_INT_PLUS_MICRO;
  }
}

static int adc_aggregator_write_raw(struct iio_dev *indio_dev,
          struct iio_chan_spec const *chan,
          int val, int val2, long info)
{
  struct adc_aggregator_state *st = iio_priv(indio_dev);
  unsigned int i;
  int ret;

  for (i = 0; i < st->num_clients; i++) {
    ret = ad7768_write_raw(st->clients[i].indio_dev,
                           &st->clients[i].indio_dev->channels[0],
                           val, val2, info);
    if (ret)
      return ret;
  }

  return 0;
}

static ssize_t gpio_data0_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	struct adc_aggregator_state *st = iio_priv(indio_dev);
	unsigned int val;
	int ret;

	ret = regmap_read(st->regmap_gpio, ADC_AGGREGATOR_GPIO_REG_DATA0, &val);
	if (ret)
		return ret;

	return sysfs_emit(buf, "0x%08x\n", val);
}
static DEVICE_ATTR_RO(gpio_data0);

static ssize_t gpio_data1_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	struct adc_aggregator_state *st = iio_priv(indio_dev);
	unsigned int val;
	int ret;

	ret = regmap_read(st->regmap_gpio, ADC_AGGREGATOR_GPIO_REG_DATA1, &val);
	if (ret)
		return ret;

	return sysfs_emit(buf, "0x%08x\n", val);
}
static DEVICE_ATTR_RO(gpio_data1);

static struct attribute *adc_aggregator_gpio_attrs[] = {
	&dev_attr_gpio_data0.attr,
	&dev_attr_gpio_data1.attr,
	NULL
};

static const struct attribute_group adc_aggregator_gpio_attr_group = {
	.attrs = adc_aggregator_gpio_attrs,
};

static const struct iio_info adc_aggregator_info = {
  .read_raw = &adc_aggregator_read_raw,
  .read_avail = &adc_aggregator_read_avail,
  .write_raw = &adc_aggregator_write_raw,
  .write_raw_get_fmt = &adc_aggregator_write_raw_get_fmt,
  .attrs = &adc_aggregator_gpio_attr_group,
  /* .read_label = ad7768_multi_read_label, */
  /* .get_current_scan_type = &ad7768_multi_get_current_scan_type, */
  /* .debugfs_reg_access = &ad7768_multi_reg_access, */
};

static struct iio_dev *adc_aggregator_get_client(struct device *dev,
					     struct device_node *np)
{
	struct device *cdev;
	struct spi_device *spi;
	struct iio_dev *indio_dev;

	cdev = bus_find_device_by_of_node(&spi_bus_type, np);
	if (!cdev)
		return ERR_PTR(-EPROBE_DEFER);

	spi = to_spi_device(cdev);
	indio_dev = spi_get_drvdata(spi);
	if (!indio_dev) {
		put_device(cdev);
		return ERR_PTR(-EPROBE_DEFER);
	}

	/*
	 * Verify the SPI device is actually bound to an ad7768-1 (or variant)
	 * driver to avoid mis-aggregating unrelated devices.
	 */
	if (!spi->dev.driver ||
	    strcmp(spi->dev.driver->name, "ad7768-1") != 0) {
		put_device(cdev);
		return ERR_PTR(-EINVAL);
	}

	/*
	 * Establish a device link so the client is kept bound while the
	 * aggregator exists; the link is auto-removed when the consumer
	 * (aggregator) is unbound.
	 */
	if (!device_link_add(dev, &spi->dev,
			     DL_FLAG_AUTOREMOVE_CONSUMER | DL_FLAG_PM_RUNTIME)) {
		put_device(cdev);
		return ERR_PTR(-ENODEV);
	}

	/* Take an IIO reference; matched by iio_device_put in cleanup. */
	iio_device_get(indio_dev);

	return indio_dev;
}

static void adc_aggregator_release_clients(void *data)
{
	struct adc_aggregator_state *st = data;
	unsigned int i;

	for (i = 0; i < st->num_clients; i++) {
		if (st->clients[i].indio_dev)
			iio_device_put(st->clients[i].indio_dev);
		if (st->clients[i].dev)
			put_device(st->clients[i].dev);
	}
}

static int adc_aggregator_clients(struct device *dev,
					struct adc_aggregator_state *st)
{
	struct device_node *np = dev->of_node;
	int count, i, ret;

	count = of_count_phandle_with_args(np, "adi,adc-devices", NULL);
	if (count <= 0)
		return dev_err_probe(dev, count ? count : -EINVAL,
				     "missing/empty 'adi,adc-devices'\n");

	st->clients = devm_kcalloc(dev, count, sizeof(*st->clients), GFP_KERNEL);
	if (!st->clients)
		return -ENOMEM;

	/*
	 * Register the cleanup action early so partially-populated clients[]
	 * is released on any error path below.
	 */
	ret = devm_add_action_or_reset(dev, adc_aggregator_release_clients, st);
	if (ret)
		return ret;

	for (i = 0; i < count; i++) {
		struct device_node *cnp;
		struct iio_dev *cindio;

		cnp = of_parse_phandle(np, "adi,adc-devices", i);
		if (!cnp)
			return -EINVAL;

		cindio = adc_aggregator_get_client(dev, cnp);
		of_node_put(cnp);
		if (IS_ERR(cindio))
			return dev_err_probe(dev, PTR_ERR(cindio),
					     "failed to get client %d\n", i);

		st->clients[i].indio_dev = cindio;
		st->clients[i].dev = cindio->dev.parent;
		/*
		 * The reference returned by bus_find_device_by_of_node()
		 * inside ad7768_agg_get_client() is kept and released by
		 * ad7768_agg_release_clients().
		 */
		st->num_clients++;
	}

	return 0;
}

static irqreturn_t adc_aggregator_interrupt(int irq, void *dev_id)
{
  struct iio_dev *indio_dev = dev_id;
  struct adc_aggregator_state *st = iio_priv(indio_dev);
  unsigned int pending;
  int ret;

  ret = regmap_read(st->regmap, ADC_AGGREGATOR_REG_IRQ_PENDING, &pending);
  if (ret) {
    pr_err("adc_aggregator: IRQ %d: failed to read IRQ_PENDING: %d\n", irq, ret);
    return IRQ_HANDLED;
  }

  if (pending & ADC_AGGREGATOR_IRQ_FLUSH_DONE) {
    regmap_write(st->regmap, ADC_AGGREGATOR_REG_IRQ_PENDING, ADC_AGGREGATOR_IRQ_FLUSH_DONE);
    complete(&st->completion_flush);
  }

  if (pending & ADC_AGGREGATOR_IRQ_FIFO_RESET_DONE) {
    regmap_write(st->regmap, ADC_AGGREGATOR_REG_IRQ_PENDING, ADC_AGGREGATOR_IRQ_FIFO_RESET_DONE);
    complete(&st->completion_reset);
  }

  return IRQ_HANDLED;
};

/*
 * Issue one or both CONTROL one-shots (flush_partial / fifo_reset) and wait
 * for the matching *_done IRQs. The IRQ mask is enabled only for the duration
 * of the wait, to avoid racing with stale IRQ_PENDING bits, and is always
 * disabled again before returning.
 */
static int adc_aggregator_flush_partial_fifo_reset(struct adc_aggregator_state *st,
                                      unsigned int cmd_mask)
{
  struct device *dev = st->dev;
  unsigned int irq_mask = 0;
  int ret;

  if (cmd_mask & ADC_AGGREGATOR_CTRL_FLUSH_PARTIAL)
    irq_mask |= ADC_AGGREGATOR_IRQ_FLUSH_DONE;
  if (cmd_mask & ADC_AGGREGATOR_CTRL_FIFO_RESET)
    irq_mask |= ADC_AGGREGATOR_IRQ_FIFO_RESET_DONE;

  /* reinit before enabling IRQ mask to avoid racing with stale IRQ_PENDING bits */
  reinit_completion(&st->completion_flush);
  reinit_completion(&st->completion_reset);

  ret = regmap_update_bits(st->regmap, ADC_AGGREGATOR_REG_IRQ_MASK,
                           irq_mask, irq_mask);
  if (ret)
    return ret;

  ret = regmap_update_bits(st->regmap, ADC_AGGREGATOR_REG_CONTROL,
                           cmd_mask, cmd_mask);
  if (ret)
    goto err_mask;

  if ((cmd_mask & ADC_AGGREGATOR_CTRL_FLUSH_PARTIAL) &&
      !wait_for_completion_timeout(&st->completion_flush, msecs_to_jiffies(1000))) {
    dev_err(dev, "flush_partial timed out\n");
    ret = -ETIMEDOUT;
    goto err_mask;
  }

  if ((cmd_mask & ADC_AGGREGATOR_CTRL_FIFO_RESET) &&
      !wait_for_completion_timeout(&st->completion_reset, msecs_to_jiffies(1000))) {
    dev_err(dev, "fifo_reset timed out\n");
    ret = -ETIMEDOUT;
    goto err_mask;
  }

err_mask:
  regmap_update_bits(st->regmap, ADC_AGGREGATOR_REG_IRQ_MASK, irq_mask, 0);
  return ret;
}

static int adc_aggregator_offload_buffer_preenable(struct iio_dev *indio_dev)
{
  struct adc_aggregator_state *st = iio_priv(indio_dev);

  return adc_aggregator_flush_partial_fifo_reset(st, ADC_AGGREGATOR_CTRL_FIFO_RESET);
}

static int adc_aggregator_offload_buffer_postenable(struct iio_dev *indio_dev)
{
	struct adc_aggregator_state *st = iio_priv(indio_dev);
	struct device *dev = st->dev;
	unsigned int i;
	int ret;

	for (i = 0; i < st->num_clients; i++) {
		ret = ad7768_set_enabled(st->clients[i].indio_dev, true);
		if (ret) {
			dev_err(dev, "client %u enable failed: %d\n", i, ret);
			goto err_rollback;
		}
	}

  ret = adc_aggregator_flush_partial_fifo_reset(st, ADC_AGGREGATOR_CTRL_FLUSH_PARTIAL);
  if (ret)
    goto err_rollback;

  ret = regmap_update_bits(st->regmap, ADC_AGGREGATOR_REG_CONTROL, ADC_AGGREGATOR_CTRL_ENABLE, ADC_AGGREGATOR_CTRL_ENABLE);
  if (ret)
    goto err_rollback;

	return 0;

err_rollback:
	while (i-- > 0)
		ad7768_set_enabled(st->clients[i].indio_dev, false);
	return ret;
}

static int adc_aggregator_offload_buffer_predisable(struct iio_dev *indio_dev)
{
	struct adc_aggregator_state *st = iio_priv(indio_dev);
	unsigned int i;
	int ret;

  ret = regmap_update_bits(st->regmap, ADC_AGGREGATOR_REG_CONTROL, ADC_AGGREGATOR_CTRL_ENABLE, 0);
  if (ret)
    return ret;

	for (i = st->num_clients; i-- > 0;)
		ad7768_set_enabled(st->clients[i].indio_dev, false);

	return 0;
}

static const struct iio_buffer_setup_ops adc_aggregator_buffer_ops = {
  .preenable = adc_aggregator_offload_buffer_preenable,
  .postenable = adc_aggregator_offload_buffer_postenable,
  .predisable = adc_aggregator_offload_buffer_predisable,
};

static const struct spi_offload_config adc_aggregator_offload_config = {
	.capability_flags = SPI_OFFLOAD_CAP_TRIGGER | SPI_OFFLOAD_CAP_RX_STREAM_DMA,
};

static int adc_aggregator_offload_probe(struct iio_dev *indio_dev,
				    struct adc_aggregator_state *st)
{
	struct device *dev = st->dev;
	int ret;

  ret = devm_iio_dmaengine_buffer_setup(dev, indio_dev, "rx");
	if (ret)
		return dev_err_probe(dev, ret, "failed to setup offload RX DMA\n");

  indio_dev->setup_ops = &adc_aggregator_buffer_ops;

	return 0;
}

// init ADC_AGGREGATOR regmap
static int adc_aggregator_init_regmap(struct platform_device *pdev, struct adc_aggregator_state *st)
{
	struct device *dev = st->dev;
	void __iomem *base;
	unsigned int val;
	int ret;

	base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(base))
		return PTR_ERR(base);

	st->regmap = devm_regmap_init_mmio(dev, base, &adc_aggregator_regmap_config);
	if (IS_ERR(st->regmap))
		return PTR_ERR(st->regmap);

	/* Force hardware defaults before touching any other register */
	ret = regmap_write(st->regmap, ADC_AGGREGATOR_REG_SW_RESET, ADC_AGGREGATOR_SW_RESET_BIT);
	if (ret)
		return dev_err_probe(dev, ret, "SW_RESET write failed\n");

	/* Log hardware version; warn if major revision does not match driver */
	ret = regmap_read(st->regmap, ADC_AGGREGATOR_REG_VERSION, &val);
	if (ret)
		return dev_err_probe(dev, ret, "VERSION read failed\n");

	dev_dbg(dev, "hardware version 0x%08x\n", val);
	if (ADC_AGGREGATOR_VERSION_MAJOR(val) != ADC_AGGREGATOR_DRIVER_VERSION_MAJOR)
		dev_warn(dev, "version major mismatch: hw=%u driver=%u\n",
			 ADC_AGGREGATOR_VERSION_MAJOR(val),
			 ADC_AGGREGATOR_DRIVER_VERSION_MAJOR);

	/* Bus integrity test: write a known pattern and read it back */
	ret = regmap_write(st->regmap, ADC_AGGREGATOR_REG_SCRATCH, ADC_AGGREGATOR_SCRATCH_TEST_VAL);
	if (ret)
		return dev_err_probe(dev, ret, "SCRATCH write failed\n");

	ret = regmap_read(st->regmap, ADC_AGGREGATOR_REG_SCRATCH, &val);
	if (ret)
		return dev_err_probe(dev, ret, "SCRATCH read failed\n");

	if (val != ADC_AGGREGATOR_SCRATCH_TEST_VAL)
		return dev_err_probe(dev, -EIO,
				     "SCRATCH mismatch: wrote 0x%08x, read 0x%08x\n",
				     ADC_AGGREGATOR_SCRATCH_TEST_VAL, val);

	/* Strictly verify NUM_CHANNELS reported by hardware matches DT */
	ret = regmap_read(st->regmap, ADC_AGGREGATOR_REG_CHANNEL_CONFIG, &val);
	if (ret)
		return dev_err_probe(dev, ret, "CHANNEL_CONFIG read failed\n");


	{
		unsigned int num_ch = FIELD_GET(ADC_AGGREGATOR_CHANNEL_CONFIG_NUM_CH_MASK, val);

		if (num_ch != st->num_clients)
			return dev_err_probe(dev, -EINVAL,
					     "CHANNEL_CONFIG mismatch: hw=%u dt=%u\n",
					     num_ch, st->num_clients);
	}

	return 0;
}


// init GPIO regmap
static int adc_aggregator_init_regmap_gpio(struct platform_device *pdev, struct adc_aggregator_state *st)
{
	struct device *dev = st->dev;
	struct device_node *gpio_np;
	struct resource res;
	void __iomem *base;
	int ret;

	gpio_np = of_parse_phandle(dev->of_node, "irs,gpio-sampler", 0);
	if (!gpio_np)
		return dev_err_probe(dev, -ENODEV,
				     "missing 'irs,gpio-sampler' phandle\n");

	ret = of_address_to_resource(gpio_np, 0, &res);
	of_node_put(gpio_np);
	if (ret)
		return dev_err_probe(dev, ret,
				     "failed to get 'irs,gpio-sampler' reg\n");

	base = devm_ioremap_resource(dev, &res);
	if (IS_ERR(base))
		return PTR_ERR(base);

	st->regmap_gpio = devm_regmap_init_mmio(dev, base, &adc_aggregator_regmap_gpio_config);
	if (IS_ERR(st->regmap_gpio))
		return PTR_ERR(st->regmap_gpio);

	return 0;
}



static int adc_aggregator_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct iio_dev *indio_dev;
	struct adc_aggregator_state *st;
	int ret;

	indio_dev = devm_iio_device_alloc(dev, sizeof(*st));
	if (!indio_dev)
		return -ENOMEM;

	st = iio_priv(indio_dev);
  st->dev = dev;

  ret = adc_aggregator_clients(dev, st);
  if (ret)
    return ret;

  ret = adc_aggregator_probe_channels(indio_dev);
  if (ret)
    return ret;

	indio_dev->name = "adc_aggregator";
	indio_dev->info = &adc_aggregator_info;
	indio_dev->modes = INDIO_BUFFER_HARDWARE;

  ret = adc_aggregator_offload_probe(indio_dev, st);
  if (ret)
    return ret;

  ret = adc_aggregator_init_regmap(pdev, st);
  if (ret)
    return ret;

  ret = adc_aggregator_init_regmap_gpio(pdev, st);
  if (ret)
    return ret;

  st->irq = platform_get_irq(pdev, 0);
  if (st->irq < 0)
    return st->irq;

  init_completion(&st->completion_flush);
  init_completion(&st->completion_reset);
  ret = devm_request_irq(&pdev->dev, st->irq, &adc_aggregator_interrupt,
                         IRQF_TRIGGER_RISING | IRQF_NO_THREAD,
                         indio_dev->name, indio_dev);
  if (ret)
    return ret;


	return devm_iio_device_register(dev, indio_dev);
}

static const struct of_device_id adc_aggregator_match[] = {
	{ .compatible = "adc_aggregator" },
	{ }
};
MODULE_DEVICE_TABLE(of, adc_aggregator_match);

static struct platform_driver adc_aggregator_driver = {
	.driver = {
		.name = "adc-aggregator",
		.of_match_table = adc_aggregator_match,
	},
	.probe = adc_aggregator_probe,
};
module_platform_driver(adc_aggregator_driver);

MODULE_AUTHOR("Joscha Loos <joscha.loos@irs.systems>");
MODULE_DESCRIPTION("Aggregate multiple ADCs");
MODULE_LICENSE("GPL v2");
MODULE_IMPORT_NS("IIO_DMAENGINE_BUFFER");
MODULE_IMPORT_NS("AD7768");
