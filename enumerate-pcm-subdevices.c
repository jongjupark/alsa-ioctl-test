/*
 * enumerate-pcm-subdevices.c
 *
 * Copyright (c) 2017 Takashi Sakamoto
 *
 * Licensed under the terms of the GNU General Public License, version 3.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <string.h>
#include <limits.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>

#include <unistd.h>

#include <sound/asound.h>

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

static void dump_pcm_info(const struct snd_pcm_info *info)
{
    printf("        id:             %s\n", info->id);
    printf("        name:           %s\n", info->name);
    printf("        subname:        %s\n", info->subname);
    printf("        dev_class:      %s\n", class_labels[info->dev_class]);
    printf("        dev_subclass:   %s\n", subclass_labels[info->dev_subclass]);
}

static int enumerate_pcm_subdevices(int fd, int card, int device)
{
    static const int dirs[SNDRV_PCM_STREAM_LAST + 1] = {
        [0] = SNDRV_PCM_STREAM_PLAYBACK,
        [1] = SNDRV_PCM_STREAM_CAPTURE,
    };
    struct snd_pcm_info info;
    int subdevice;
    int i;

    for (i = 0; i < sizeof(dirs) / sizeof(dirs[0]); ++i) {
        subdevice = 0;
        while (1) {
            memset(&info, 0, sizeof(struct snd_pcm_info));
            info.device = device;
            info.subdevice = subdevice;
            info.card = card;
            info.stream = dirs[i];

            if (ioctl(fd, SNDRV_CTL_IOCTL_PCM_INFO, &info) < 0) {
                if (errno != ENOENT) {
                    printf("ioctl(2) with PCM_INFO: %s\n", strerror(errno));
                    return -errno;
                }
            } else {
                if (subdevice == 0) {
                    printf("    direction:          %s\n",
                           direction_labels[dirs[i]]);
                    printf("    node:               pcmC%dD%d%c\n",
                           card, device,
                           dirs[i] == SNDRV_PCM_STREAM_CAPTURE ? 'c' : 'p');
                }

                printf("      subdevice:        %u\n", info.subdevice);

                dump_pcm_info(&info);
            }

            if (++subdevice >= info.subdevices_count)
                break;
        }
    }

    return 0;
}

static void enumerate_pcm_devices(int fd, int card)
{
    int device;

    device = -1;
    while (1) {
        if (ioctl(fd, SNDRV_CTL_IOCTL_PCM_NEXT_DEVICE, &device) < 0) {
            printf("ioctl(2) with PCM_NEXT_DEVICE: %s\n", strerror(errno));
            return;
        }
        if (device < 0)
            break;

        printf("  device:               %d\n", device);

        enumerate_pcm_subdevices(fd, card, device);
        ++device;
    }
}

static int dump_card_info(int fd, struct snd_ctl_card_info *info)
{
    if (ioctl(fd, SNDRV_CTL_IOCTL_CARD_INFO, info) < 0) {
        printf("ioctl(2) for card info: %s\n", strerror(errno));
        return -errno;
    }

    printf("  card:                 %d\n", info->card);
    printf("  id:                   %s\n", info->id);
    printf("  driver:               %s\n", info->driver);
    printf("  name:                 %s\n", info->name);
    printf("  longname:             %s\n", info->longname);
    printf("  mixername:            %s\n", info->mixername);
    printf("  component:            %s\n", info->components);

    return 0;
}

int main(int argc, const char *const argv[])
{
    const char *path;
    struct snd_ctl_card_info info = {0};
    int fd;

    if (argc < 2) {
        printf("At least, one argument is required for control character "
               "device.\n");
        return EXIT_FAILURE;
    }
    path = argv[1];

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("open(2): %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    printf("%s:\n", path);
    dump_card_info(fd, &info);

    enumerate_pcm_devices(fd, info.card);

    close(fd);
    return EXIT_SUCCESS;
}
