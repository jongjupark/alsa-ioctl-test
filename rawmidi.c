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
	TEST_IOCTL(fd, SNDRV_RAWMIDI_IOCTL_PVERSION, &version, 0);
}

static bool check_info(int fd)
{
	struct snd_rawmidi_info info = {0};
	TEST_IOCTL(fd, SNDRV_RAWMIDI_IOCTL_INFO, &info, 0);
}

static bool check_params(int fd)
{
	struct snd_rawmidi_params params = {0};
	params.stream = SNDRV_RAWMIDI_STREAM_OUTPUT;
	params.buffer_size = 1024;
	params.avail_min = 2;
	TEST_IOCTL(fd, SNDRV_RAWMIDI_IOCTL_PARAMS, &params, 0);
}

static bool check_status(int fd)
{
	struct snd_rawmidi_status status = {0};
	TEST_IOCTL(fd, SNDRV_RAWMIDI_IOCTL_STATUS, &status, 0);
}

static bool check_drop(int fd)
{
	int drop = 0;
	TEST_IOCTL(fd, SNDRV_RAWMIDI_IOCTL_DROP, &drop, 0);
}

static bool check_drain(int fd)
{
	int drain = 0;
	TEST_IOCTL(fd, SNDRV_RAWMIDI_IOCTL_DRAIN, &drain, 0);
}

int main(void)
{
	bool (*const funcs[])(int) = {
		check_pversion,
		check_info,
		check_params,
		check_status,
		check_drop,
		check_drain,
		NULL,
	};
	unsigned int i;
	int fd;

	fd = open("/dev/snd/midiC0D0", O_RDWR);
	if (fd < 0) {
		printf("%s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	i = 0;
	while (funcs[i]) {
		if (!funcs[i++](fd)) {
			printf("Rawmidi test aborts.\n");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
