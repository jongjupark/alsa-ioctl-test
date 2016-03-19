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
	int version;
	TEST_IOCTL(fd, SNDRV_HWDEP_IOCTL_PVERSION, &version, 0);
}

static bool check_info(int fd)
{
	struct snd_hwdep_info info = {0};
	TEST_IOCTL(fd, SNDRV_HWDEP_IOCTL_INFO, &info, 0);
}

static bool check_dsp_status(int fd)
{
	struct snd_hwdep_dsp_status status = {0};
	TEST_IOCTL(fd, SNDRV_HWDEP_IOCTL_DSP_STATUS, &status, ENXIO);
}

static bool check_dsp_load(int fd)
{
	struct snd_hwdep_dsp_image image = {0};
	TEST_IOCTL(fd, SNDRV_HWDEP_IOCTL_DSP_LOAD, &image, ENXIO);
}

int main(void)
{
	bool (*const funcs[])(int) = {
		check_pversion,
		check_info,
		check_dsp_status,
		check_dsp_load,
		NULL,
	};
	unsigned int i;
	int fd;

	fd = open("/dev/snd/hwC0D0", O_RDONLY);
	if (fd < 0) {
		printf("%s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	i = 0;
	while (funcs[i]) {
		if (!funcs[i++](fd)) {
			printf("Hwdep test aborts.\n");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
