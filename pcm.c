#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>

#include <errno.h>
#include <string.h>

#include <sound/asound.h>

#define TEST_IOCTL(fd, command, argument, expected)			\
	if (ioctl(fd, command, argument) < 0 && errno != expected) {	\
		printf("%s: %s\n", #command, strerror(errno));		\
		return false;						\
	}								\
	return true;							\

static bool check_pversion(int fd)
{
	int version = 0;
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_PVERSION, &version, 0);
}

static bool check_info(int fd)
{
	struct snd_pcm_info info = {0};
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_INFO, &info, 0);
}

static bool check_tstamp(int fd)
{
	int tstamp = 0;
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_TSTAMP, &tstamp, 0);
}

static bool check_ttstamp(int fd)
{
	int tstamp = 0;
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_TTSTAMP, &tstamp, 0);
}

static bool check_hw_refine(int fd)
{
	struct snd_pcm_hw_params params = {0};
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_HW_REFINE, &params, EINVAL);
}

static bool check_hw_params(int fd)
{
	struct snd_pcm_hw_params params = {0};
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_HW_PARAMS, &params, EINVAL);
}

static bool check_hw_free(int fd)
{
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_HW_FREE, NULL, EBADFD);
}

static bool check_sw_params(int fd)
{
	struct snd_pcm_sw_params params = {0};
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_SW_PARAMS, &params, EBADFD);
}

static bool check_status(int fd)
{
	struct snd_pcm_status status = {0};
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_STATUS, &status, 0);
}

static bool check_delay(int fd)
{
	snd_pcm_sframes_t delay = 0;
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_DELAY, &delay, EBADFD);
}

static bool check_hwsync(int fd)
{
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_HWSYNC, NULL, EBADFD);
}

static bool check_sync_ptr(int fd)
{
	struct snd_pcm_sync_ptr ptr = {0};
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_SYNC_PTR, &ptr, 0);
}

static bool check_status_ext(int fd)
{
	struct snd_pcm_status status = {0};
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_STATUS_EXT, &status, 0);
}

static bool check_channel_info(int fd)
{
	struct snd_pcm_channel_info info = {0};
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_CHANNEL_INFO, &info, EBADFD);
}

static bool check_prepare(int fd)
{
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_PREPARE, NULL, EBADFD);
}

static bool check_reset(int fd)
{
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_RESET, NULL, EBADFD);
}

static bool check_start(int fd)
{
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_START, NULL, EBADFD);
}

static bool check_drop(int fd)
{
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_DROP, NULL, EBADFD);
}

static bool check_drain(int fd)
{
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_DRAIN, NULL, EBADFD);
}

static bool check_pause(int fd)
{
	int pause = 0;
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_PAUSE, &pause, ENOSYS);
}

static bool check_rewind(int fd)
{
	snd_pcm_uframes_t frames = 0;
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_REWIND, &frames, 0);
}

static bool check_resume(int fd)
{
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_RESUME, NULL, ENOSYS);
}

static bool check_xrun(int fd)
{
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_XRUN, NULL, EBADFD);
}

static bool check_forward(int fd)
{
	snd_pcm_uframes_t frames = 0;
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_FORWARD, &frames, 0);
}

static bool check_writei_frames(int fd)
{
	struct snd_xferi xferi = {0};
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &xferi, EBADFD);
}

static bool check_readi_frames(int fd)
{
	struct snd_xferi xferi = {0};
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_READI_FRAMES, &xferi, EBADFD);
}

static bool check_writen_frames(int fd)
{
	struct snd_xfern xfern = {0};
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_WRITEN_FRAMES, &xfern, EBADFD);
}

static bool check_readn_frames(int fd)
{
	struct snd_xfern xfern = {0};
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_READN_FRAMES, &xfern, EBADFD);
}

static bool check_link(int fd)
{
	int link = 0;
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_LINK, &link, EBADFD);
}

static bool check_unlink(int fd)
{
	TEST_IOCTL(fd, SNDRV_PCM_IOCTL_UNLINK, NULL, EALREADY);
}

int main(void)
{
	bool (*const funcs[])(int) = {
		check_pversion,
		check_info,
		check_tstamp,
		check_ttstamp,
		check_hw_refine,
		check_hw_params,
		check_hw_free,
		check_sw_params,
		check_status,
		check_delay,
		check_hwsync,
		check_sync_ptr,
		check_status_ext,
		check_channel_info,
		check_prepare,
		check_reset,
		check_start,
		check_drop,
		check_drain,
		check_pause,
		check_rewind,
		check_resume,
		check_xrun,
		check_forward,
		check_writei_frames,
		check_readi_frames,
		check_writen_frames,
		check_readn_frames,
		check_link,
		check_unlink,
		NULL,
	};
	unsigned int i;
	int fd;

	fd = open("/dev/snd/pcmC0D0p", O_RDONLY);
	if (fd < 0) {
		printf("%s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	i = 0;
	while (funcs[i]) {
		if (!funcs[i++](fd)) {
			printf("PCM test aborts.\n");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
