/*
 * refine-pcm-params.c
 *
 * Copyright (c) 2017 Takashi Sakamoto
 *
 * Licensed under the terms of the GNU General Public License, version 3.
 */

#include <stdio.h>
#include <stdlib.h>
#include <linux/version.h>

#include <string.h>
#include <limits.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>

#include <unistd.h>

#include <sound/asound.h>

#define ARRAY_SIZE(array)   (sizeof(array) / sizeof(array[0]))

static const char *const class_labels[] = {
    [SNDRV_PCM_CLASS_GENERIC]   = "generic",
    [SNDRV_PCM_CLASS_MULTI]     = "multi",
    [SNDRV_PCM_CLASS_MODEM]     = "modem",
    [SNDRV_PCM_CLASS_DIGITIZER] = "digitizer"
};
static const char *const subclass_labels[] = {
    [SNDRV_PCM_SUBCLASS_GENERIC_MIX]    = "generic-mix",
    [SNDRV_PCM_SUBCLASS_MULTI_MIX]      = "multi-mix"
};
static const char *const direction_labels[] = {
    [SNDRV_PCM_STREAM_PLAYBACK] = "playback",
    [SNDRV_PCM_STREAM_CAPTURE]  = "capture"
};

static const char *const param_labels[] = {
    [SNDRV_PCM_HW_PARAM_ACCESS]         = "access",
    [SNDRV_PCM_HW_PARAM_FORMAT]         = "format",
    [SNDRV_PCM_HW_PARAM_SUBFORMAT]      = "subformat",
    [SNDRV_PCM_HW_PARAM_SAMPLE_BITS]    = "sample-bits",
    [SNDRV_PCM_HW_PARAM_FRAME_BITS]     = "frame-bits",
    [SNDRV_PCM_HW_PARAM_CHANNELS]       = "channels",
    [SNDRV_PCM_HW_PARAM_RATE]           = "rate",
    [SNDRV_PCM_HW_PARAM_PERIOD_TIME]    = "period-time",
    [SNDRV_PCM_HW_PARAM_PERIOD_SIZE]    = "period-size",
    [SNDRV_PCM_HW_PARAM_PERIOD_BYTES]   = "period-bytes",
    [SNDRV_PCM_HW_PARAM_PERIODS]        = "periods",
    [SNDRV_PCM_HW_PARAM_BUFFER_TIME]    = "buffer-time",
    [SNDRV_PCM_HW_PARAM_BUFFER_SIZE]    = "buffer-size",
    [SNDRV_PCM_HW_PARAM_BUFFER_BYTES]   = "buffer-bytes",
    [SNDRV_PCM_HW_PARAM_TICK_TIME]      = "tick-time",
};

static const char *const access_labels[] = {
    [SNDRV_PCM_ACCESS_MMAP_INTERLEAVED]     = "mmap-interleaved",
    [SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED]  = "mmap-noninterleaved",
    [SNDRV_PCM_ACCESS_MMAP_COMPLEX]         = "mmap-complex",
    [SNDRV_PCM_ACCESS_RW_INTERLEAVED]       = "readwrite-interleaved",
    [SNDRV_PCM_ACCESS_RW_NONINTERLEAVED]    = "readwrite-noninterleaved",
};

static const char *const format_labels[] = {
    [SNDRV_PCM_FORMAT_S8]               = "s8",
    [SNDRV_PCM_FORMAT_U8]               = "u8",
    [SNDRV_PCM_FORMAT_S16_LE]           = "s16-le",
    [SNDRV_PCM_FORMAT_S16_BE]           = "s16-be",
    [SNDRV_PCM_FORMAT_U16_LE]           = "u16-le",
    [SNDRV_PCM_FORMAT_U16_BE]           = "u16-be",
    [SNDRV_PCM_FORMAT_S24_LE]           = "s24-le",
    [SNDRV_PCM_FORMAT_S24_BE]           = "s24-be",
    [SNDRV_PCM_FORMAT_U24_LE]           = "u24-le",
    [SNDRV_PCM_FORMAT_U24_BE]           = "u24-be",
    [SNDRV_PCM_FORMAT_S32_LE]           = "s32-le",
    [SNDRV_PCM_FORMAT_S32_BE]           = "s32-be",
    [SNDRV_PCM_FORMAT_U32_LE]           = "u32-le",
    [SNDRV_PCM_FORMAT_U32_BE]           = "u32-be",
    [SNDRV_PCM_FORMAT_FLOAT_LE]         = "float-le",
    [SNDRV_PCM_FORMAT_FLOAT_BE]         = "float-be",
    [SNDRV_PCM_FORMAT_FLOAT64_LE]       = "float64-be",
    [SNDRV_PCM_FORMAT_FLOAT64_BE]       = "float64-le",
    [SNDRV_PCM_FORMAT_IEC958_SUBFRAME_LE]   = "iec958subframe-le",
    [SNDRV_PCM_FORMAT_IEC958_SUBFRAME_BE]   = "iec958subframe-be",
    [SNDRV_PCM_FORMAT_MU_LAW]           = "mu-law",
    [SNDRV_PCM_FORMAT_A_LAW]            = "a-law",
    [SNDRV_PCM_FORMAT_IMA_ADPCM]        = "ima-adpcm",
    [SNDRV_PCM_FORMAT_MPEG]             = "mpg",
    [SNDRV_PCM_FORMAT_GSM]              = "gsm",
    /* Entries for 25-30 are absent. */
    [SNDRV_PCM_FORMAT_SPECIAL]          = "special",
    [SNDRV_PCM_FORMAT_S24_3LE]          = "s24-3le",
    [SNDRV_PCM_FORMAT_S24_3BE]          = "s24-3be",
    [SNDRV_PCM_FORMAT_U24_3LE]          = "u24-3le",
    [SNDRV_PCM_FORMAT_U24_3BE]          = "u24-3be",
    [SNDRV_PCM_FORMAT_S20_3LE]          = "s20-3le",
    [SNDRV_PCM_FORMAT_S20_3BE]          = "s20-3be",
    [SNDRV_PCM_FORMAT_U20_3LE]          = "u20-3le",
    [SNDRV_PCM_FORMAT_U20_3BE]          = "u20-3be",
    [SNDRV_PCM_FORMAT_S18_3LE]          = "s18-3le",
    [SNDRV_PCM_FORMAT_S18_3BE]          = "s18-3be",
    [SNDRV_PCM_FORMAT_U18_3LE]          = "u18-3le",
    [SNDRV_PCM_FORMAT_U18_3BE]          = "u18-3be",
    [SNDRV_PCM_FORMAT_G723_24]          = "g723-24",
    [SNDRV_PCM_FORMAT_G723_24_1B]       = "g723-241b",
    [SNDRV_PCM_FORMAT_G723_40]          = "g723-40",
    [SNDRV_PCM_FORMAT_G723_40_1B]       = "g723-401b",
    [SNDRV_PCM_FORMAT_DSD_U8]           = "dsd-u8",
    [SNDRV_PCM_FORMAT_DSD_U16_LE]       = "dsd-u16-le",
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
    [SNDRV_PCM_FORMAT_DSD_U32_LE]       = "dsd-u32-le",
    [SNDRV_PCM_FORMAT_DSD_U16_BE]       = "dsd-u16-be",
    [SNDRV_PCM_FORMAT_DSD_U32_BE]       = "dsd-u23-be",
#endif
};

static const char *const subformat_labels[] = {
    [SNDRV_PCM_SUBFORMAT_STD] = "std",
};

static const char *const flag_labels[] = {
    [0] = "noresample",
    [1] = "export-buffer",
    [2] = "no-period-wakeup",
};

static const char *const info_labels[] = {
    [0]     = "mmap",
    [1]     = "mmap-valid",
    [2]     = "double",
    [3]     = "batch",
    [4]     = "interleaved",
    [5]     = "non-interleaved",
    [6]     = "complex",
    [7]     = "block-transfer",
    [8]     = "overrange",
    [9]     = "resume",
    [10]    = "pause",
    [11]    = "half-duplex",
    [12]    = "joint-duplex",
    [13]    = "sync-start",
    [14]    = "no-period-wakeup",
    [15]    = "has-wall-clock",
    [16]    = "has-link-atime",
    [17]    = "has-link-absolute-atime",
    [18]    = "has-link-estimated-atime",
    [19]    = "has-link-synchronized-atime",
    /* Does not be disclosed to userspace. */
    [20]    = "drain-trigger",
    [21]    = "fifo-in-frames",
};

static const int info_flags[] = {
    [0]     = SNDRV_PCM_INFO_MMAP,
    [1]     = SNDRV_PCM_INFO_MMAP_VALID,
    [2]     = SNDRV_PCM_INFO_DOUBLE,
    [3]     = SNDRV_PCM_INFO_BATCH,
    [4]     = SNDRV_PCM_INFO_INTERLEAVED,
    [5]     = SNDRV_PCM_INFO_NONINTERLEAVED,
    [6]     = SNDRV_PCM_INFO_COMPLEX,
    [7]     = SNDRV_PCM_INFO_BLOCK_TRANSFER,
    [8]     = SNDRV_PCM_INFO_OVERRANGE,
    [9]     = SNDRV_PCM_INFO_RESUME,
    [10]    = SNDRV_PCM_INFO_PAUSE,
    [11]    = SNDRV_PCM_INFO_HALF_DUPLEX,
    [12]    = SNDRV_PCM_INFO_JOINT_DUPLEX,
    [13]    = SNDRV_PCM_INFO_SYNC_START,
    [14]    = SNDRV_PCM_INFO_NO_PERIOD_WAKEUP,
    [15]    = SNDRV_PCM_INFO_HAS_WALL_CLOCK,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
    [16]    = SNDRV_PCM_INFO_HAS_LINK_ATIME,
    [17]    = SNDRV_PCM_INFO_HAS_LINK_ABSOLUTE_ATIME,
    [18]    = SNDRV_PCM_INFO_HAS_LINK_ESTIMATED_ATIME,
    [19]    = SNDRV_PCM_INFO_HAS_LINK_SYNCHRONIZED_ATIME,
#endif
    /* Does not be disclosed to userspace. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0)
    [20]    = SNDRV_PCM_INFO_DRAIN_TRIGGER,
#endif
    [21]    = SNDRV_PCM_INFO_FIFO_IN_FRAMES,
};

static struct snd_mask *get_mask(struct snd_pcm_hw_params *params,
                                 unsigned int type)
{
    return &params->masks[type - SNDRV_PCM_HW_PARAM_FIRST_MASK];
}

static struct snd_interval *get_interval(struct snd_pcm_hw_params *params,
                                         unsigned int type)
{
    return &params->intervals[type - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL];
}

static unsigned int get_mask_count(void)
{
    return SNDRV_PCM_HW_PARAM_LAST_MASK - SNDRV_PCM_HW_PARAM_FIRST_MASK + 1;
}

static unsigned int get_interval_count(void)
{
    return SNDRV_PCM_HW_PARAM_LAST_INTERVAL -
                                        SNDRV_PCM_HW_PARAM_FIRST_INTERVAL + 1;
}

static unsigned int get_mask_index(unsigned int index)
{
    return index + SNDRV_PCM_HW_PARAM_FIRST_MASK;
}

static unsigned int get_interval_index(unsigned int index)
{
    return index + SNDRV_PCM_HW_PARAM_FIRST_INTERVAL;
}

static void dump_mask_param(struct snd_pcm_hw_params *hw_params,
                            snd_pcm_hw_param_t type, const char *const labels[],
                            unsigned int label_entries)
{
    const struct snd_mask *mask;
    unsigned int index;
    int i, j;

    if (type < SNDRV_PCM_HW_PARAM_FIRST_MASK ||
        type > SNDRV_PCM_HW_PARAM_LAST_MASK ||
        type >= ARRAY_SIZE(param_labels))
        return;
    printf("    %s:\n", param_labels[type]);

    mask = get_mask(hw_params, type);

    for (i = 0; i < ARRAY_SIZE(mask->bits); ++i) {
        for (j = 0; j < sizeof(mask->bits[0]) * 8; ++j) {
            index = i * sizeof(mask->bits[0]) * 8 + j;
            if (index >= label_entries)
                return;
            if ((mask->bits[i] & (1 << j)) && labels[index] != NULL)
                printf("      %s\n", labels[index]);
        }
    }
}

static void dump_interval_param(struct snd_pcm_hw_params *hw_params,
                                snd_pcm_hw_param_t type)
{
    const struct snd_interval *interval;

    if (type < SNDRV_PCM_HW_PARAM_FIRST_INTERVAL ||
        type > SNDRV_PCM_HW_PARAM_LAST_INTERVAL ||
        type >= ARRAY_SIZE(param_labels))
        return;
    printf("    %s:\n", param_labels[type]);

    interval = get_interval(hw_params, type);

    printf("      %c%u, %u%c, ",
           interval->openmin ? '(' : '[', interval->min,
           interval->max, interval->openmax ? ')' : ']');
    if (interval->integer > 0)
        printf("integer, ");
    if (interval->empty > 0)
        printf("empty, ");
    printf("\n");
}

static void initialize_hw_params(struct snd_pcm_hw_params *params)
{
    unsigned int i;

    for (i = 0; i < get_mask_count(); i++) {
        memset(&params->masks[i], 0xff, sizeof(struct snd_mask));

        params->rmask |= 1 << get_mask_index(i);
    }

    for (i = 0; i < get_interval_count(); i++) {
        params->intervals[i].min = 0;
        params->intervals[i].openmin = 0;
        params->intervals[i].max = UINT_MAX;
        params->intervals[i].openmax = 0;
        params->intervals[i].integer = 0;
        params->intervals[i].empty = 0;

        params->rmask |= 1 << get_interval_index(i);
    }

    params->cmask = 0;
    params->info = 0;
}

static int dump_pcm_caps(int fd)
{
    struct snd_pcm_hw_params hw_params = {0};
    int i;

    initialize_hw_params(&hw_params);

    if (ioctl(fd, SNDRV_PCM_IOCTL_HW_REFINE, &hw_params) < 0) {
        printf("  ioctl(HW_REFINE): %s\n", strerror(errno));
        return -errno;
    }

    printf("  Changed parameters:\n");

    for (i = 0; i < ARRAY_SIZE(param_labels); ++i) {
        if (hw_params.cmask & (1 << i))
            printf("    %s\n", param_labels[i]);
    }

    printf("  Runtime parameters:\n");

    dump_mask_param(&hw_params, SNDRV_PCM_HW_PARAM_ACCESS, access_labels,
                    ARRAY_SIZE(access_labels));
    dump_mask_param(&hw_params, SNDRV_PCM_HW_PARAM_FORMAT, format_labels,
                    ARRAY_SIZE(format_labels));
    dump_mask_param(&hw_params, SNDRV_PCM_HW_PARAM_SUBFORMAT, subformat_labels,
                    ARRAY_SIZE(subformat_labels));

    dump_interval_param(&hw_params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS);
    dump_interval_param(&hw_params, SNDRV_PCM_HW_PARAM_FRAME_BITS);
    dump_interval_param(&hw_params, SNDRV_PCM_HW_PARAM_CHANNELS);
    dump_interval_param(&hw_params, SNDRV_PCM_HW_PARAM_RATE);
    dump_interval_param(&hw_params, SNDRV_PCM_HW_PARAM_PERIOD_TIME);
    dump_interval_param(&hw_params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE);
    dump_interval_param(&hw_params, SNDRV_PCM_HW_PARAM_PERIODS);
    dump_interval_param(&hw_params, SNDRV_PCM_HW_PARAM_BUFFER_TIME);
    dump_interval_param(&hw_params, SNDRV_PCM_HW_PARAM_BUFFER_SIZE);
    dump_interval_param(&hw_params, SNDRV_PCM_HW_PARAM_BUFFER_BYTES);
    dump_interval_param(&hw_params, SNDRV_PCM_HW_PARAM_TICK_TIME);

    if (hw_params.flags > 0) {
        printf("      flags: \n");
        for (i = 0; i < ARRAY_SIZE(flag_labels); ++i) {
            if (hw_params.flags & (1 << i))
                printf("  %s\n", flag_labels[i]);
        }
    }

    printf("    info:\n");
    for (i = 0; i < ARRAY_SIZE(info_flags); ++i) {
        if (hw_params.info & info_flags[i])
            printf("      %s\n", info_labels[i]);
    }

    if (hw_params.msbits > 0)
        printf("      most-significant-bits:    %u\n", hw_params.msbits);

    if (hw_params.rate_num > 0 && hw_params.rate_den > 0) {
        printf("      rate_num: %u\n", hw_params.rate_num);
        printf("      rate_den: %u\n", hw_params.rate_den);
    }

    return 0;
}

static int dump_pcm_info(int fd)
{
    struct snd_pcm_info info = {0};

    if (ioctl(fd, SNDRV_PCM_IOCTL_INFO, &info) < 0) {
        printf("ioctl(2): %s\n", strerror(errno));
        return -errno;
    }

    printf("  Substream information:\n");
    printf("    device:       %u\n", info.device);
    printf("    subdevice:    %u\n", info.subdevice);
    printf("    direction:    %s\n", direction_labels[info.stream]);
    printf("    card:         %d\n", info.card);
    printf("    id:           '%s'\n", info.id);
    printf("    name:         '%s'\n", info.name);
    printf("    subname:      '%s'\n", info.subname);
    printf("    dev-class:    %s\n", class_labels[info.dev_class]);
    printf("    dev-subclass: %s\n", subclass_labels[info.dev_subclass]);
    printf("    subdevices-count:   %u\n", info.subdevices_count);
    printf("    subdevices-avail:   %u\n", info.subdevices_avail);

    return 0;
}

int main(int argc, const char *const argv[])
{
    const char *path;
    int fd;

    if (argc < 2) {
        printf("At least, one argument is required for PCM character "
               "device.\n");
        return EXIT_FAILURE;
    }
    path = argv[1];

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("open(2): %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    printf("%s\n", path);
    if (dump_pcm_info(fd) < 0) {
        close(fd);
        return EXIT_FAILURE;
    }

    if (dump_pcm_caps(fd) < 0) {
        close(fd);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
