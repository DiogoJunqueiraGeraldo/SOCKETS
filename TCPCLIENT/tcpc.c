#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>

#define SERVER_PORT 80
#define MAX_LINE 4096

void err_n_die(const char *fmt, ...);

int main(int argc, char** argv)
{
	if (argc != 2)
		err_n_die("Usage: %s <target_ip>", argv[0]);

	int socket_network = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_network < 0)
		err_n_die("Error while creating the socket!");

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVER_PORT);

	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		err_n_die("inet_pton error converting %s address to binary representation", argv[1]);

	if (connect(socket_network, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
		err_n_die("Error while trying to connect!");

	char* sendline = "GET / HTTP/1.1\r\n\r\n";
	int sendbytes = strlen(sendline);

	if (write(socket_network, sendline, sendbytes) != sendbytes)
		err_n_die("Error while writing into socket");

	char recvline[MAX_LINE];
	int n;

	for ( ; ; )
	{

		if ((n = read(socket_network, recvline, MAX_LINE-1)) == 0)
			break;

		if (n == -1)
			err_n_die("there was an error while trying to read from socket.");

		printf("%s", recvline);
		memset(recvline, 0, MAX_LINE);
	}

	int shutdown_code = shutdown(socket_network, 2);
	switch(shutdown_code)
	{
		case EBADF:
			err_n_die("socket is not a valid file descriptor.");
		case ENOTSOCK:
			err_n_die("socket is not a socket.");
		case ENOTCONN:
			err_n_die("socket is not connected.");
		default:
		{
			if (shutdown_code < 0)
				err_n_die("error while trying to close connection with socket");
		}
	}

	return 0;
}

void err_n_die(const char *fmt, ...)
{
	printf("\033[0;31m");
	int errno_save;
	va_list ap;
	errno_save = errno;

	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	fprintf(stdout, "\n");
	fflush(stdout);

	if (errno_save != 0)
	{
		fprintf(stdout, "errno = %d) : %s\n", errno_save, strerror(errno_save));
		fprintf(stdout, "\n");
		fflush(stdout);
	}

	va_end(ap);
	printf("\033[0m");

	exit(1);
}
