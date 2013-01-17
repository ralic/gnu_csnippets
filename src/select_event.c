/*
 * Copyright (c) 2012 Ahmed Samy <f.fallen45@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifdef USE_SELECT

#include <internal/socket_compat.h>
#include <csnippets/io_poll.h>
#include <csnippets/socket.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/time.h>
#endif

#ifndef MAX_EVENTS
#define MAX_EVENTS 1024
#endif

struct pollev {
	size_t index;
	struct data {
		int fd;
		short events;
		short revents;
	} __packed fds[FD_SETSIZE];
};

static int
compute_revents(int fd, int sought, fd_set *rfds, fd_set *wfds, fd_set *efds)
{
	int happened = 0;
	if (FD_ISSET (fd, rfds))
		happened |= IO_READ & sought;

	if (FD_ISSET (fd, wfds))
		happened |= IO_WRITE & sought;

	if (FD_ISSET (fd, efds))
		happened |= IO_ERR & sought;

	return happened;
}

struct pollev *pollev_init(void)
{
	struct pollev *ev;

	xmalloc(ev, sizeof(struct pollev), return NULL);
	ev->index = 0;
	memset(ev->fds, 0, sizeof(ev->fds));
	return ev;
}

void pollev_deinit(struct pollev *p)
{
	free(p->fds);
	free(p);
}

void pollev_add(struct pollev *pev, int fd, int bits)
{
	size_t tmpidx = pev->index;
	if (unlikely(!pev || fd < 0))
		return;

	if (++tmpidx >= FD_SETSIZE) {
#ifdef _DEBUG_POLLEV
		dbg("can't add more file descriptors to this set, the size would exceed the maximum number of file descriptors\n");
#endif
		return;
	}

	if (bits & IO_READ)
		pev->fds[pev->index].events |= IO_READ;
	if (bits & IO_WRITE)
		pev->fds[pev->index].events |= IO_WRITE;

	pev->fds[pev->index].fd = fd;
	pev->fds[pev->index].revents = 0;
	++pev->index;
}

void pollev_del(struct pollev *pev, int fd)
{
	int i;
	if (unlikely(!pev))
		return;

	for (i = 0; i < FD_SETSIZE; ++i)
		if (pev->fds[i].fd == fd) {
			pev->fds[i].fd = -1;
			break;
		}
}

int pollev_poll(struct pollev *pev, int timeout)
{
	int fd, maxfd, rc, i;
	static fd_set rfds, wfds, efds;
	static struct timeval tv;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	memset(&tv, 0, sizeof(tv));
	tv.tv_sec  = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

	for (maxfd = 0, i = 0; i < pev->index; ++i) {
		fd = pev->fds[i].fd;
		if (fd <= 0)
			continue;
		uint32_t ev = pev->fds[i].events;
		if (ev & IO_READ)
			FD_SET(fd, &rfds);
		if (ev & IO_WRITE)
			FD_SET(fd, &wfds);

		if (ev & (IO_READ | IO_WRITE)) {
			FD_SET(fd, &efds);
			if (fd > maxfd && fd <= FD_SETSIZE)
				maxfd = fd;
		}
	}

	dbg("polling on %d fds.\n", maxfd + 1);
	do
		rc = select(maxfd + 1, &rfds, &wfds, &efds, &tv);
	while (rc < 0 && S_error == S_EINTR);
	if (rc <= 0)
		return -1;

	/* establish results  */
	for (rc = 0, i = 0; i <= maxfd; ++i)
		if (pev->fds[i].fd >= 0) {
			int happened = compute_revents(pev->fds[i].fd, pev->fds[i].events,
							&rfds, &wfds, &efds);
			if (happened) {
				pev->fds[i].revents = happened;
				++rc;
			}
		}

	dbg("Done.  %d fd(s) are ready.\n", rc);
	return rc;
}

__inline int pollev_active(struct pollev *pev, int index)
{
	if (unlikely(index < 0 || index > FD_SETSIZE))
		return -1;
	return pev->fds[index].fd;
}

__inline uint32_t pollev_revent(struct pollev *pev, int index)
{
	uint32_t r = 0;
	if (unlikely(index < 0 || index > FD_SETSIZE))
		return r;
	r = pev->fds[index].revents;
	pev->fds[index].revents = 0;
	return r;
}

#endif

