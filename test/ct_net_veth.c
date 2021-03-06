/*
 * Test that veth pair can be created
 */
#define _GNU_SOURCE
#include <sys/types.h>
#include <libct.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sched.h>
#include "test.h"

#define VETH_HOST_NAME	"hveth0"
#define VETH_CT_NAME	"cveth0"

struct ct_arg {
	int wait_pipe;
	int *mark;
};

static int check_ct_net(void *a)
{
	struct ct_arg *ca = a;
	char c;

	ca->mark[0] = 1;
	if (!system("ip link l " VETH_CT_NAME ""))
		ca->mark[2] = 1;

	read(ca->wait_pipe, &c, 1);
	return 0;
}

int main(int argc, char **argv)
{
	int p[2];
	struct ct_arg ca;
	libct_session_t s;
	ct_handler_t ct;
	struct ct_net_veth_arg va;

	ca.mark = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANON, 0, 0);
	pipe(p);

	ca.mark[0] = 0;
	ca.mark[1] = 0;
	ca.mark[2] = 0;
	ca.wait_pipe = p[0];

	va.host_name = VETH_HOST_NAME;
	va.ct_name = VETH_CT_NAME;

	s = libct_session_open_local();
	ct = libct_container_create(s, "test");
	libct_container_set_nsmask(ct, CLONE_NEWNET);

	if (libct_net_add(ct, CT_NET_VETH, &va))
		return err("Can't add hostnic");

	if (libct_container_spawn_cb(ct, check_ct_net, &ca))
		return err("Can't spawn CT");

	if (!system("ip link l " VETH_HOST_NAME ""))
		ca.mark[1] = 1;

	write(p[1], "a", 1);

	libct_container_wait(ct);
	libct_container_destroy(ct);
	libct_session_close(s);

	if (!ca.mark[0])
		return fail("CT is not alive");
	if (!ca.mark[1])
		return fail("VETH not created");
	if (!ca.mark[2])
		return fail("VETH not assigned");

	return pass("VETH works OK");
}
