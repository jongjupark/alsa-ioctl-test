#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>

#include <errno.h>
#include <string.h>

#include <limits.h>

#include <sound/asequencer.h>

#define TEST_IOCTL(fd, command, argument, expected)			\
	if (ioctl(fd, command, argument) < 0 && errno != expected) {	\
		printf("%s: %s\n", #command, strerror(errno));		\
		return false;						\
	}								\
	return true;							\

static bool check_pversion(int fd)
{
	int version;
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_PVERSION, &version, 0);
}

static bool check_client_id(int fd)
{
	int id;
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_CLIENT_ID, &id, 0);
}

static bool check_system_info(int fd)
{
	struct snd_seq_system_info info = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_SYSTEM_INFO, &info, 0);
}

static bool check_running_mode(int fd)
{
	struct snd_seq_running_info info = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_RUNNING_MODE, &info, 0);
}

static bool check_get_client_info(int fd)
{
	struct snd_seq_client_info info = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_GET_CLIENT_INFO, &info, 0);
}

static bool check_set_client_info(int fd)
{
	struct snd_seq_client_info info = {0};
	info.client = INT_MAX;
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_SET_CLIENT_INFO, &info, EPERM);
}

static bool check_create_port(int fd)
{
	struct snd_seq_port_info info = {0};
	info.addr.client = UCHAR_MAX;
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_CREATE_PORT, &info, EPERM);
}

static bool check_delete_port(int fd)
{
	struct snd_seq_port_info info = {0};
	info.addr.client = UCHAR_MAX;
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_DELETE_PORT, &info, EPERM);
}

static bool check_get_port_info(int fd)
{
	struct snd_seq_port_info info = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_GET_PORT_INFO, &info, 0);
}

static bool check_set_port_info(int fd)
{
	struct snd_seq_port_info info = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_SET_PORT_INFO, &info, EPERM);
}

static bool check_subscribe_port(int fd)
{
	struct snd_seq_port_subscribe subscribe = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_SUBSCRIBE_PORT, &subscribe, EPERM);
}

static bool check_unsubscribe_port(int fd)
{
	struct snd_seq_port_subscribe subscribe = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_UNSUBSCRIBE_PORT, &subscribe, EPERM);
}

static bool check_create_queue(int fd)
{
	struct snd_seq_queue_info info = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_CREATE_QUEUE, &info, 0);
}

static bool check_delete_queue(int fd)
{
	struct snd_seq_queue_info info = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_DELETE_QUEUE, &info, 0);
}

static bool check_get_queue_info(int fd)
{
	struct snd_seq_queue_info info = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_GET_QUEUE_INFO, &info, EINVAL);
}

static bool check_set_queue_info(int fd)
{
	struct snd_seq_queue_info info = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_SET_QUEUE_INFO, &info, EINVAL);
}

static bool check_get_named_queue(int fd)
{
	struct snd_seq_queue_info info = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_GET_NAMED_QUEUE, &info, EINVAL);
}

static bool check_get_queue_status(int fd)
{
	struct snd_seq_queue_status status = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_GET_QUEUE_STATUS, &status, EINVAL);
}

static bool check_get_queue_tempo(int fd)
{
	struct snd_seq_queue_status tempo = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_GET_QUEUE_TEMPO, &tempo, EINVAL);
}

static bool check_set_queue_tempo(int fd)
{
	struct snd_seq_queue_tempo tempo = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_SET_QUEUE_TEMPO, &tempo, EPERM);
}

static bool check_get_queue_timer(int fd)
{
	struct snd_seq_queue_timer timer = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_GET_QUEUE_TIMER, &timer, EINVAL);
}

static bool check_set_queue_timer(int fd)
{
	struct snd_seq_queue_timer timer = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_SET_QUEUE_TIMER, &timer, EPERM);
}

static bool check_get_queue_client(int fd)
{
	struct snd_seq_queue_client client = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_GET_QUEUE_CLIENT, &client, EINVAL);
}

static bool check_set_queue_client(int fd)
{
	struct snd_seq_queue_client client = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_SET_QUEUE_CLIENT, &client, EINVAL);
}

static bool check_get_client_pool(int fd)
{
	struct snd_seq_client_pool pool = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_GET_CLIENT_POOL, &pool, EINVAL);
}

static bool check_set_client_pool(int fd)
{
	struct snd_seq_client_pool pool = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_SET_CLIENT_POOL, &pool, EINVAL);
}

static bool check_remove_events(int fd)
{
	struct snd_seq_remove_events events = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_REMOVE_EVENTS, &events, 0);
}

static bool check_query_subs(int fd)
{
	struct snd_seq_query_subs subs = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_QUERY_SUBS, &subs, ENOENT);
}

static bool check_get_subscription(int fd)
{
	struct snd_seq_query_subs subs = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_GET_SUBSCRIPTION, &subs, ENOENT);
}

static bool check_query_next_client(int fd)
{
	struct snd_seq_client_info info = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_QUERY_NEXT_CLIENT, &info, 0);
}

static bool check_query_next_port(int fd)
{
	struct snd_seq_port_info info = {0};
	TEST_IOCTL(fd, SNDRV_SEQ_IOCTL_QUERY_NEXT_PORT, &info, 0);
}

int main(void)
{
	bool (*const funcs[])(int) = {
		check_pversion,
		check_client_id,
		check_system_info,
		check_running_mode,
		check_get_client_info,
		check_set_client_info,
		check_create_port,
		check_delete_port,
		check_get_port_info,
		check_set_port_info,
		check_subscribe_port,
		check_unsubscribe_port,
		check_create_queue,
		check_delete_queue,
		check_get_queue_info,
		check_set_queue_info,
		check_get_named_queue,
		check_get_queue_status,
		check_get_queue_tempo,
		check_set_queue_tempo,
		check_get_queue_timer,
		check_set_queue_timer,
		check_get_queue_client,
		check_set_queue_client,
		check_get_client_pool,
		check_set_client_pool,
		check_remove_events,
		check_query_subs,
		check_get_subscription,
		check_query_next_client,
		check_query_next_port,
		NULL,
	};
	unsigned int i;
	int fd;

	fd = open("/dev/snd/seq", O_RDONLY);
	if (fd < 0) {
		printf("%s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	i = 0;
	while (funcs[i]) {
		if (!funcs[i++](fd)) {
			printf("Seq test aborts.\n");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
