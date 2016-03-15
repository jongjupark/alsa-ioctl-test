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
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_PVERSION, &version, 0);
}

static bool check_card_info(int fd)
{
	struct snd_ctl_card_info info = {0};
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_CARD_INFO, &info, 0);
}

static bool check_elem_list(int fd)
{
	struct snd_ctl_elem_list list = {0};
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_ELEM_LIST, &list, 0);
}

static bool check_elem_info(int fd)
{
	struct snd_ctl_elem_info info = {0};
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_ELEM_INFO, &info, ENOENT);
}

static bool check_elem_read(int fd)
{
	struct snd_ctl_elem_value data = {0};
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_ELEM_READ, &data, ENOENT);
}

static bool check_elem_write(int fd)
{
	struct snd_ctl_elem_value data = {0};
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_ELEM_READ, &data, ENOENT);
}

static bool check_elem_lock(int fd)
{
	struct snd_ctl_elem_id id = {0};
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_ELEM_LOCK, &id, ENOENT);
}

static bool check_elem_unlock(int fd)
{
	struct snd_ctl_elem_id id = {0};
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_ELEM_UNLOCK, &id, ENOENT);
}

static bool check_subscribe_events(int fd)
{
	int subscribe = 0;
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_SUBSCRIBE_EVENTS, &subscribe, 0);
}

static bool check_elem_add(int fd)
{
	struct snd_ctl_elem_info info = {0};
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_ELEM_ADD, &info, EINVAL);
}

static bool check_elem_replace(int fd)
{
	struct snd_ctl_elem_info info = {0};
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_ELEM_REPLACE, &info, EINVAL);
}

static bool check_elem_remove(int fd)
{
	struct snd_ctl_elem_info info = {0};
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_ELEM_REMOVE, &info, ENOENT);
}

static bool check_tlv_read(int fd)
{
	struct snd_ctl_tlv tlv = {0};
	tlv.length = 0;
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_TLV_READ, &tlv, EINVAL);
}

static bool check_tlv_write(int fd)
{
	struct snd_ctl_tlv tlv = {0};
	tlv.length = 0;
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_TLV_WRITE, &tlv, EINVAL);
}

static bool check_tlv_command(int fd)
{
	struct snd_ctl_tlv tlv = {0};
	tlv.length = 0;
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_TLV_COMMAND, &tlv, EINVAL);
}

static bool check_hwdep_next_device(int fd)
{
	int next;
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_HWDEP_NEXT_DEVICE, &next, 0);
}

static bool check_hwdep_info(int fd)
{
	struct snd_hwdep_info info = {0};
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_HWDEP_INFO, &info, 0);
}

static bool check_pcm_next_device(int fd)
{
	int next;
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_PCM_NEXT_DEVICE, &next, 0);
}

static bool check_pcm_info(int fd)
{
	struct snd_pcm_info info = {0};
	info.subdevice = 300;
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_PCM_INFO, &info, ENXIO);
}

static bool check_pcm_prefer_subdevice(int fd)
{
	int prefer;
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_PCM_PREFER_SUBDEVICE, &prefer, 0);
}

static bool check_rawmidi_next_deivce(int fd)
{
	int next;
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_RAWMIDI_NEXT_DEVICE, &next, 0);
}

static bool check_rawmidi_info(int fd)
{
	struct snd_rawmidi_info info = {0};
	info.subdevice = 300;
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_RAWMIDI_INFO, &info, ENXIO);
}

static bool check_rawmidi_prefer_subdevice(int fd)
{
	int prefer;
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_RAWMIDI_PREFER_SUBDEVICE, &prefer, 0);
}

static bool check_power(int fd)
{
	int powering;
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_POWER, &powering, ENOPROTOOPT);
}

static bool check_power_state(int fd)
{
	int state;
	TEST_IOCTL(fd, SNDRV_CTL_IOCTL_POWER_STATE, &state, 0);
}

int main(void)
{
	bool (*const funcs[])(int) = {
		check_pversion,
		check_card_info,
		check_elem_list,
		check_elem_info,
		check_elem_read,
		check_elem_write,
		check_elem_lock,
		check_elem_unlock,
		check_subscribe_events,
		check_elem_add,
		check_elem_replace,
		check_elem_remove,
		check_tlv_read,
		check_tlv_write,
		check_tlv_command,
		check_hwdep_next_device,
		check_hwdep_info,
		check_pcm_next_device,
		check_pcm_info,
		check_pcm_prefer_subdevice,
		check_rawmidi_next_deivce,
		check_rawmidi_info,
		check_rawmidi_prefer_subdevice,
		check_power,
		check_power_state,
		NULL,
	};
	unsigned int i;
	int fd;

	fd = open("/dev/snd/controlC0", O_RDONLY);
	if (fd < 0) {
		printf("%s", strerror(errno));
		return EXIT_FAILURE;
	}

	i = 0;
	while (funcs[i]) {
		if (!funcs[i++](fd)) {
			printf("Ctl test aborts.\n");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
