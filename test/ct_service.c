/*
 * Test that service cgroup works
 */
#include <libct.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>
#include "test.h"

struct ct_arg {
	int start_fd;
	int wait_fd;
	int *mark;
};

static int set_ct_alive(void *a)
{
	struct ct_arg *cta = a;
	char c = 'a';

	cta->mark[0] = getpid();
	write(cta->start_fd, &c, 1);
	read(cta->wait_fd, &c, 1);
	return 0;
}

static int check_service_cg(int pid)
{
	FILE *f;
	char buf[32];

	f = fopen("/sys/fs/cgroup/.libct/test-s/tasks", "r");
	if (!f) {
		perror("No file\n");
		return 0;
	}

	memset(buf, 0, sizeof(buf));
	if (!fgets(buf, sizeof(buf), f)) {
		fclose(f);
		return 0;
	}

	fclose(f);

	return atoi(buf) == pid;
}

int main(int argc, char **argv)
{
	struct ct_arg cta;
	int pid, p[2], p2[2];
	libct_session_t s;
	ct_handler_t ct;
	int cg_ok = 0;
	char c;

	pipe(p);
	pipe(p2);
	cta.mark = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANON, 0, 0);
	cta.mark[0] = 0;
	cta.start_fd = p2[1];
	cta.wait_fd = p[0];

	s = libct_session_open_local();
	ct = libct_container_create(s, "test-s");
	if (libct_container_set_option(ct, LIBCT_OPT_KILLABLE))
		return err("can't set killable");

	if (libct_container_spawn_cb(ct, set_ct_alive, &cta))
		return err("can't start CT");

	read(p2[0], &c, 1);
	if (cta.mark[0])
		cg_ok = check_service_cg(cta.mark[0]);
	write(p[1], &c, 1);

	libct_container_wait(ct);
	libct_container_destroy(ct);
	libct_session_close(s);

	if (!cta.mark[0])
		return fail("CT is not alive");

	if (!cg_ok)
		return fail("Service CG is not there");

	return pass("service CG works OK");
}
