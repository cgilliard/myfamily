// Copyright (c) 2024, The MyFamily Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <arpa/inet.h>	// for sockaddr_in
#include <base/err.h>
#include <base/print_util.h>
#include <base/sys.h>
#include <base/util.h>
#include <fcntl.h>	// for O_ constants
#include <sys/event.h>
#include <sys/mman.h>	 // for MAP and PROT constants
#include <sys/socket.h>	 // for struct sockaddr
#include <sys/types.h>	 // for ssize_t/size_t/off_t

#define SOCKET_FLAG_CLIENT 0x2000000000000000ULL
#define SOCKET_FLAG_UDP 0x4000000000000000ULL
#define SOCKET_FLAGS_ALL (SOCKET_FLAG_UDP | SOCKET_FLAG_CLIENT)
#define FD(handle) (handle & 0xFFFFFFFF)

// needed system calls:
int open(const char *pathname, int flags, ...);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
int shutdown(int sockfd, int how);
int accept(int sockfd, struct sockaddr *_Nullable restrict addr,
		   socklen_t *_Nullable restrict addrlen);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int setsockopt(int socket, int level, int option_name, const void *option_value,
			   socklen_t option_len);

int64 transmit(int64 fd, const byte *buf, uint64 len) {
	int64 ret = write(FD(fd), buf, len);
	if (ret == -1) SetErr(IO);
	return ret;
}

int64 receive(int64 fd, byte *buf, uint64 len) {
	int64 ret = read(FD(fd), buf, len);
	if (ret == -1) SetErr(IO);
	return ret;
}

uint16_t big_endian_port(uint16_t portshort) {
	return (portshort << 8) |
		   (portshort >> 8);  // Swap bytes (we are only on little endian)
}

int big_endian_addr(uint32 addr) {
	return ((addr & 0xFF) << 24) | ((addr & 0xFF00) << 8) |
		   ((addr & 0xFF0000) >> 8) | ((addr & 0xFF000000) >> 24);
}

int addr_to_uint32(const char *str, uint32 *out_addr) {
	byte bytes[4];
	int count = 0;

	while (*str) {
		int byte = 0;
		while (*str >= '0' && *str <= '9') {
			byte = byte * 10 + (*str - '0');
			str++;
		}

		if (count >= 4 || byte < 0 || byte > 255) {
			return -1;
		}

		bytes[count++] = byte;

		if (*str == '.') {
			str++;
		} else if (*str != '\0') {
			return -1;
		}
	}

	*out_addr =
		(bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
	return 0;
}

int64 server(int port, const char *addr, bool udp, int backlog) {
	if (backlog > 8191) {
		SetErr(IllegalArgument);
		return -1;
	}
	uint32 addr_bytes;
	if (addr_to_uint32(addr, &addr_bytes)) {
		SetErr(InvalidAddr);
		return -1;
	}
	int64 ret = big_endian_addr(addr_bytes) |
				((uint64)big_endian_port(port) << 32) | ((int64)backlog << 48);
	if (udp) ret |= SOCKET_FLAG_UDP;
	return ret;
}
int64 client(int port, const char *addr, bool udp) {
	uint32 addr_bytes;
	if (addr_to_uint32(addr, &addr_bytes)) {
		SetErr(InvalidAddr);
		return -1;
	}
	int64 ret = big_endian_addr(addr_bytes) |
				((uint64)big_endian_port((port)) << 32) | SOCKET_FLAG_CLIENT;
	if (udp) ret |= SOCKET_FLAG_UDP;
	return ret;
}

int64 accept_conn(int64 handle) {
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);
	return accept(FD(handle), (struct sockaddr *)&address, &addrlen);
}

int64 establish(int64 handle) {
	int backlog = ((handle & ~SOCKET_FLAG_UDP) >> 48) & 0xFFFF;
	bool udp = (handle & SOCKET_FLAG_UDP) != 0;
	uint32 host = handle & 0xFFFFFFFFULL;
	int port = (handle >> 32) & 0xFFFF;

	int type;
	if (udp)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

	int64 sock;

	if (handle & SOCKET_FLAG_CLIENT) {
		// client
		// println("establish client conn to host: %u,port=%i,udp=%i", host,
		// port, 		udp);

		sock = socket(AF_INET, type, 0);

		if (sock < 0) {
			SetErr(MaxFds);
			return -1;
		}

		struct sockaddr_in addr;
		set_bytes((byte *)&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = port;
		addr.sin_addr.s_addr = host;

		if (!udp) {
			if (connect(sock, (struct sockaddr *)&addr, sizeof(addr))) {
				close(sock);
				SetErr(ConnectionRefused);
				return -1;
			}
		}

		sock |= SOCKET_FLAG_CLIENT;
	} else {
		// server
		// println("establish server conn to host:
		// %u,port=%i,udp=%i,backlog=%i", 		host, port, udp, backlog);

		sock = socket(AF_INET, type, 0);

		if (sock < 0) {
			println("sock -1");
			SetErr(MaxFds);
			return -1;
		}

		int reuse = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
			0) {
			println("sock opt");
			SetErr(IO);
			close(sock);
			return -1;
		}

		struct sockaddr_in addr;
		set_bytes((byte *)&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = port;
		addr.sin_addr.s_addr = host;

		if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
			println("bind");
			close(sock);
			SetErr(IO);
			return -1;
		}

		if (!udp) {
			if (listen(sock, backlog)) {
				println("udp");
				close(sock);
				SetErr(IO);
				return -1;
			}
		}
	}
	if (udp)
		return sock | SOCKET_FLAG_UDP;
	else
		return sock;
}

int evh() {
#ifdef __linux__
	int ret = epoll_create1(0);
#elif __APPLE__
	int ret = kqueue();
#else
	int ret = -1;
#endif

	return ret;
}
int evh_register(int evh, int64 handle, int op) {
	int fd = FD(handle);
	struct kevent event;
	EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_CLEAR, NOTE_WRITE, 0, NULL);
	int ret = kevent(evh, &event, 1, NULL, 0, NULL);
	return ret;
}
int evh_unregister(int evh, int64 handle) {
	return 0;
}
int evh_wait(int evh, int64 timeout_millis, int64 max_events,
			 EvhEvent *events) {
	struct kevent res[max_events];
	int ret = kevent(evh, NULL, 0, res, max_events, NULL);
	for (int i = 0; i < ret; i++) {
		int64 fd = (int)res[i].ident;
		events[i].handle = fd | SOCKET_FLAG_CLIENT;
		events[i].events = 0;
		if (res[i].filter == EVFILT_READ) {
			events[i].events |= EVENT_READ;
		} else if (res[i].filter == EVFILT_WRITE) {
			events[i].events |= EVENT_WRITE;
		}
	}
	return ret;
}
