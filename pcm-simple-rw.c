#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>
#include <stdint.h>

#include <string.h>
#include <limits.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>

#include <unistd.h>

#include <sys/epoll.h>

#include <sound/asound.h>

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

static bool check_mask(struct snd_mask *mask, unsigned int index)
{
    int i, j;

    i = index / (sizeof(mask->bits[0]) * 4);
    j = index % (sizeof(mask->bits[0]) * 4);

    return mask->bits[i] & (1 << j);
}

static void set_mask(struct snd_mask *mask, int index)
{
    int i, j;

    i = index / (sizeof(mask->bits[0]) * 4);
    j = index % (sizeof(mask->bits[0]) * 4);

    mask->bits[i] |= 1 << j;
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

static int set_params(int fd, struct snd_pcm_hw_params *params)
{
    struct snd_mask *mask;

    initialize_hw_params(params);

    if (ioctl(fd, SNDRV_PCM_IOCTL_HW_REFINE, params) < 0) {
        printf("ioctl(HW_REFINE): %s\n", strerror(errno));
        return -errno;
    }

    /* This program has no interests in mmap operation. */
    mask = get_mask(params, SNDRV_PCM_HW_PARAM_ACCESS);
    if (!check_mask(mask, SNDRV_PCM_ACCESS_RW_INTERLEAVED) &&
        !check_mask(mask, SNDRV_PCM_ACCESS_RW_NONINTERLEAVED))
        return -ENXIO;
    memset(mask, 0, sizeof(*mask));
    set_mask(mask, SNDRV_PCM_ACCESS_RW_INTERLEAVED);
    set_mask(mask, SNDRV_PCM_ACCESS_RW_NONINTERLEAVED);

    if (ioctl(fd, SNDRV_PCM_IOCTL_HW_PARAMS, params) < 0) {
        printf("ioctl(HW_PARAMS): %s\n", strerror(errno));
        return -errno;
    }

    return 0;
}

static int keep_one_period(struct snd_pcm_hw_params *params, void **buf,
                           snd_pcm_uframes_t *frames)
{
    struct snd_mask *mask;
    struct snd_interval *interval;
    unsigned int bytes_per_sample;
    unsigned int samples_per_frame;
    unsigned int frames_per_period;
    unsigned int bytes_per_frames;

    mask = get_mask(params, SNDRV_PCM_HW_PARAM_FORMAT);
    if (check_mask(mask, SNDRV_PCM_FORMAT_S8) ||
        check_mask(mask, SNDRV_PCM_FORMAT_U8))
        bytes_per_sample = sizeof(uint8_t);
    else if (check_mask(mask, SNDRV_PCM_FORMAT_S16_LE) ||
             check_mask(mask, SNDRV_PCM_FORMAT_S16_BE) ||
             check_mask(mask, SNDRV_PCM_FORMAT_U16_LE) ||
             check_mask(mask, SNDRV_PCM_FORMAT_U16_BE))
        bytes_per_sample = sizeof(uint16_t);
    else if (check_mask(mask, SNDRV_PCM_FORMAT_S24_LE) ||
             check_mask(mask, SNDRV_PCM_FORMAT_S24_BE) ||
             check_mask(mask, SNDRV_PCM_FORMAT_U24_LE) ||
             check_mask(mask, SNDRV_PCM_FORMAT_U24_BE) ||
             check_mask(mask, SNDRV_PCM_FORMAT_S32_LE) ||
             check_mask(mask, SNDRV_PCM_FORMAT_S32_BE) ||
             check_mask(mask, SNDRV_PCM_FORMAT_U32_LE) ||
             check_mask(mask, SNDRV_PCM_FORMAT_U32_BE))
        bytes_per_sample = sizeof(uint32_t);
    else
        return -ENXIO;

    interval = get_interval(params, SNDRV_PCM_HW_PARAM_CHANNELS);
    if (interval->openmin > 0 || interval->integer == 0 || interval->empty > 0) {
        return -ENXIO;
    }
    samples_per_frame = interval->min;

    interval = get_interval(params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE);
    if (interval->openmin > 0 || interval->integer == 0 || interval->empty > 0) {
        return -ENXIO;
    }
    frames_per_period = interval->min;

    bytes_per_frames = bytes_per_sample * samples_per_frame;
    *buf = calloc(bytes_per_frames * frames_per_period, sizeof(*buf));
    if (*buf == NULL) {
        printf("calloc(3): %s\n", strerror(ENOMEM));
        return -ENOMEM;
    }
    *frames = frames_per_period;

    return 0;
}

static int run_io(int fd, struct snd_pcm_info *info,
                  struct snd_pcm_hw_params *params)
{
    union {
        struct snd_xferi i;
        struct snd_xfern n;
    } xfer_data;
    struct snd_mask *mask;

    int cmd;
    const char *cmd_name;

    int epfd;
    struct epoll_event ev = {0};

    int count;
    int err;

    ev.data.fd = fd;

    mask = get_mask(params, SNDRV_PCM_HW_PARAM_ACCESS);
    if (check_mask(mask, SNDRV_PCM_ACCESS_RW_INTERLEAVED)) {
        if (info->stream == SNDRV_PCM_STREAM_PLAYBACK) {
            cmd = SNDRV_PCM_IOCTL_WRITEI_FRAMES;
            cmd_name = "WRITEI";
            ev.events = EPOLLOUT;
        } else {
            cmd = SNDRV_PCM_IOCTL_READI_FRAMES;
            cmd_name = "READI";
            ev.events = EPOLLIN;
        }
    } else if (check_mask(mask, SNDRV_PCM_ACCESS_RW_NONINTERLEAVED)) {
        if (info->stream == SNDRV_PCM_STREAM_PLAYBACK) {
            cmd = SNDRV_PCM_IOCTL_WRITEN_FRAMES;
            cmd_name = "WRITEN";
            ev.events = EPOLLOUT;
        } else {
            cmd = SNDRV_PCM_IOCTL_READN_FRAMES;
            cmd_name = "READN";
            ev.events = EPOLLIN;
        }
    } else {
        return -ENXIO;
    }

    /* Both structures have the same layout. */
    memset(&xfer_data, 0, sizeof(xfer_data));
    err = keep_one_period(params, &xfer_data.i.buf, &xfer_data.i.frames);
    if (err < 0)
        return err;

    epfd = epoll_create1(0);
    if (epfd < 0) {
        printf("epoll_create1(2): %s\n", strerror(errno));
        err = -errno;
        goto end;
    }

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        printf("epoll_ctl(2): %s\n", strerror(errno));
        err = -errno;
        goto end_epoll;
    }

    if (ioctl(fd, SNDRV_PCM_IOCTL_PREPARE, NULL) < 0) {
        printf("ioctl(PREPARE): %s\n", strerror(errno));
        err = -errno;
        goto end_epoll;
    }

    if (ioctl(fd, SNDRV_PCM_IOCTL_START, NULL) < 0) {
        printf("ioctl(START): %s\n", strerror(errno));
        err = -errno;
        goto end_epoll;
    }

    while (1) {
        memset(&ev, 0, sizeof(ev));
        count = epoll_wait(epfd, &ev, 1, 200);
        if (count < 0) {
            if (errno == EINTR)
                continue;

            printf("epoll_wait(2): %s\n", strerror(errno));
            err = -errno;
            break;
        }
        if (count == 0)
            continue;

        if (ev.events & EPOLLERR) {
            printf("EPOLLERR\n");
            break;
        }

        /* Start PCM substream automatically. */
        if (ioctl(ev.data.fd, cmd, &xfer_data) < 0) {
            if (errno == EINTR)
                continue;

            if (errno == EPIPE) {
                if (ioctl(fd, SNDRV_PCM_IOCTL_PREPARE, NULL) < 0) {
                    printf("ioctl(PREPARE): %s\n", strerror(errno));
                    err = -errno;
                    goto end_epoll;
                }
            }

            printf("ioctl(%s): %s\n", cmd_name, strerror(errno));
            err = -errno;
            break;
        }
    }
end_epoll:
    close(epfd);
end:
    free(xfer_data.i.buf);
    return 0;
}

int main(int argc, const char *const argv[])
{
    const char *path;
    int fd;
    struct snd_pcm_info info = {0};
    struct snd_pcm_hw_params params = {0};
    int err;

    if (argc < 2) {
        printf("At least, one argument is required for a path to PCM character "
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

    if (ioctl(fd, SNDRV_PCM_IOCTL_INFO, &info) < 0) {
        printf("ioctl(INFO): %s\n", strerror(errno));
        goto end;
    }

    err = set_params(fd, &params);
    if (err < 0)
        goto end;

    err = run_io(fd, &info, &params);
end:
    close(fd);

    return EXIT_SUCCESS;
}
