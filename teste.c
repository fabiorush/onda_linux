#include <poll.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main () {
	int fd;
	struct pollfd fds;

	printf("Inicio do programa\n");

	fd = open("/sys/class/gpio/gpio139/value",O_RDONLY);

	fds.fd = fd;
	fds.events = POLLPRI|POLLERR;

	poll(&fds, 1, -1);


	printf("Fim do programa\n");
}
