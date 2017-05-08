/*
 * ctl-elems.c
 *
 * Copyright (c) 2017 Takashi Sakamoto
 *
 * Licensed under the terms of the GNU General Public License, version 3.
 */

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <limits.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>

#include <unistd.h>

#include <sound/asound.h>

static const char *const type_labels[] = {
    [SNDRV_CTL_ELEM_TYPE_NONE]          = "none",
    [SNDRV_CTL_ELEM_TYPE_BOOLEAN]       = "boolean",
    [SNDRV_CTL_ELEM_TYPE_INTEGER]       = "integer",
    [SNDRV_CTL_ELEM_TYPE_ENUMERATED]    = "enumerated",
    [SNDRV_CTL_ELEM_TYPE_BYTES]         = "bytes",
    [SNDRV_CTL_ELEM_TYPE_IEC958]        = "iec60958",
    [SNDRV_CTL_ELEM_TYPE_INTEGER64]     = "integer64",
};

static const char *const iface_labels[] = {
    [SNDRV_CTL_ELEM_IFACE_CARD]         = "card",
    [SNDRV_CTL_ELEM_IFACE_HWDEP]        = "hwdep",
    [SNDRV_CTL_ELEM_IFACE_MIXER]        = "mixer",
    [SNDRV_CTL_ELEM_IFACE_PCM]          = "pcm",
    [SNDRV_CTL_ELEM_IFACE_RAWMIDI]      = "rawmidi",
    [SNDRV_CTL_ELEM_IFACE_TIMER]        = "timer",
    [SNDRV_CTL_ELEM_IFACE_SEQUENCER]    = "sequencer",
};

static const char *const access_labels[] = {
    [0]		= "read",
    [1]     = "write",
    [2]     = "volatile",
    [3]     = "timestamp",
    [4]     = "tlv-read",
    [5]     = "tlv-write",
    [6]     = "tlv-command",
    [7]     = "inactive",
    [8]     = "lock",
    [9]     = "owner",
    [10]    = "tlv-callback",
    [11]    = "user",
};

static const int access_flags[] = {
    [0]		= SNDRV_CTL_ELEM_ACCESS_READ,
    [1]     = SNDRV_CTL_ELEM_ACCESS_WRITE,
    [2]     = SNDRV_CTL_ELEM_ACCESS_VOLATILE,
    [3]     = SNDRV_CTL_ELEM_ACCESS_TIMESTAMP,
    [4]     = SNDRV_CTL_ELEM_ACCESS_TLV_READ,
    [5]     = SNDRV_CTL_ELEM_ACCESS_TLV_WRITE,
    [6]     = SNDRV_CTL_ELEM_ACCESS_TLV_COMMAND,
    [7]     = SNDRV_CTL_ELEM_ACCESS_INACTIVE,
    [8]     = SNDRV_CTL_ELEM_ACCESS_LOCK,
    [9]     = SNDRV_CTL_ELEM_ACCESS_OWNER,
    [10]    = SNDRV_CTL_ELEM_ACCESS_TLV_CALLBACK,
    [11]    = SNDRV_CTL_ELEM_ACCESS_USER,
};

static int dump_integer_elem(int fd, struct snd_ctl_elem_info *info)
{
    printf("      min:  %ld\n", info->value.integer.min);
    printf("      max:  %ld\n", info->value.integer.max);
    printf("      step: %ld\n", info->value.integer.step);

    return 0;
}

static int dump_enumerated_elem(int fd, struct snd_ctl_elem_info *info)
{
    int i;

    printf("      items:  %d\n", info->value.enumerated.items);

    printf("      labels:\n");
    printf("        ");
    for (i = 0; i < info->value.enumerated.items; ++i) {
        info->value.enumerated.item = i;
        /* Just querying, has no side effects. */
        if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_INFO, info) < 0)
            return errno;

        printf("'%s', ", info->value.enumerated.name);
    }
    printf("\n");

    return 0;
}

static int dump_integer64_elem(int fd, struct snd_ctl_elem_info *info)
{
    printf("      min:  %ld\n", info->value.integer.min);
    printf("      max:  %ld\n", info->value.integer.max);
    printf("      step: %ld\n", info->value.integer.step);

    return 0;
}

static int dump_elem(int fd, struct snd_ctl_elem_id *id)
{
    int (*const funcs[])(int fd, struct snd_ctl_elem_info *info) = {
        [SNDRV_CTL_ELEM_TYPE_NONE]          = NULL,
        [SNDRV_CTL_ELEM_TYPE_BOOLEAN]       = NULL,
        [SNDRV_CTL_ELEM_TYPE_INTEGER]       = dump_integer_elem,
        [SNDRV_CTL_ELEM_TYPE_ENUMERATED]    = dump_enumerated_elem,
        [SNDRV_CTL_ELEM_TYPE_BYTES]         = NULL,
        [SNDRV_CTL_ELEM_TYPE_IEC958]        = NULL,
        [SNDRV_CTL_ELEM_TYPE_INTEGER64]     = dump_integer64_elem,
    };
    struct snd_ctl_elem_info info = {0};
    int i;
    int err = 0;

    info.id = *id;
    if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_INFO, &info) < 0) {
        printf("ioctl(2) with SNDRV_CTL_IOCTL_ELEM_INFO\n");
        return -errno;
    }

    if (info.type >= sizeof(funcs)/sizeof(funcs[0])) {
        return -EIO;
    }

    printf("  Element %d:\n", info.id.numid);
    printf("    iface:      %s\n", iface_labels[info.id.iface]);
    printf("    device:     %d\n", info.id.device);
    printf("    subdevice:  %d\n", info.id.subdevice);
    printf("    name:       '%s'\n", info.id.name);
    printf("    index:      %d\n", info.id.index);

    printf("    type:       %s\n", type_labels[info.type]);

    printf("    access:     ");
    for (i = 0; i < sizeof(access_labels)/sizeof(access_labels[0]); ++i) {
        if (info.access & access_flags[i])
            printf("%s, ", access_labels[i]);
    }
    printf("\n");
    printf("    members:    %d\n", info.count);
    printf("    lock-owner: %d\n", info.owner);

    printf("    dimension:  ");
    for (i = 0; i < sizeof(info.dimen.d)/sizeof(info.dimen.d[0]); ++i)
        printf("[%d] => %d, ", i, info.dimen.d[i]);
    printf("\n");

    if (funcs[info.type]) {
        printf("    type dependent:\n");
        err = funcs[info.type](fd, &info);
    }

    return err;
}

static int dump_elems(int fd, struct snd_ctl_elem_list *list)
{
    int i;
    int err;

    for (i = 0; i < list->count; ++i) {
        err = dump_elem(fd, &list->pids[i]);
        if (err < 0)
            break;
    }

    return err;
}

static int allocate_elem_ids(int fd, struct snd_ctl_elem_list *list)
{
    struct snd_ctl_elem_id *ids;

    /* Get the number of elements in this control device. */
    if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_LIST, list) < 0)
        return -errno;

    /* No elements found. */
    if (list->count == 0)
        return 0;

    /* Allocate spaces for these elements. */
    ids = calloc(list->count, sizeof(struct snd_ctl_elem_id));
    if (ids == NULL)
        return -ENOMEM;

    list->offset = 0;
    while (list->offset < list->count) {
        /*
         * ALSA middleware has limitation of one operation.
         * 1000 is enought less than the limitation.
         */
        if (list->count - list->offset > 1000)
            list->space = 1000;
        else
            list->space = list->count - list->offset;
        list->pids = ids + list->offset;

        /* Get the IDs of elements in this control device. */
        if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_LIST, list) < 0) {
            free(ids);
            list->pids = NULL;
            return errno;
        }

        list->offset += list->space;
    }

    list->pids = ids;
    list->space = list->count;

    return 0;
}

static void deallocate_elem_ids(struct snd_ctl_elem_list *list)
{
    if (list->pids != NULL)
        free(list->pids);
}

static int dump_card_info(int fd)
{
    struct snd_ctl_card_info info = {0};

    if (ioctl(fd, SNDRV_CTL_IOCTL_CARD_INFO, &info) < 0) {
        printf("ioctl(2): %s\n", strerror(errno));
        return -errno;
    }

    printf("  Card information:\n");
    printf("    card:       %d\n", info.card);
    printf("    id:         '%s'\n", info.id);
    printf("    driver:     '%s'\n", info.driver);
    printf("    name:       '%s'\n", info.name);
    printf("    long-name:  '%s'\n", info.longname);
    printf("    mixer-name: '%s'\n", info.mixername);
    printf("    components: '%s'\n", info.components);

    return 0;
}

int main(int argc, const char *const argv[])
{
    const char *path;
    int fd;
    struct snd_ctl_elem_list list = {0};
    int err;

    if (argc < 2) {
        printf("At least one argument is required for ALSA control character "
               "device.\n");
        return EXIT_FAILURE;
    }
    path = argv[1];

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("open(2): %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    err = allocate_elem_ids(fd, &list);
    if (err < 0) {
        close(fd);
        return EXIT_FAILURE;
    }

    printf("%s\n", path);
    err = dump_card_info(fd);
    if (err >= 0)
        err = dump_elems(fd, &list);

    deallocate_elem_ids(&list);

    close(fd);

    return EXIT_SUCCESS;
}
