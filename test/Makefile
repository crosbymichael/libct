TESTS =	ct_create ct_enter ct_proc ct_root ct_root_enter \
	ct_create_exec ct_cgroup_basic ct_net_host \
	ct_net_veth ct_create_exec_d ct_private_subdir \
	ct_ext_mount ct_mhandle_exec_d ct_private_subdir_ns \
	ct_cgroup_sub ct_service ct_kill_nons

PIGS  = file_piggy

CC = gcc
CFLAGS = -I../src/include/uapi/ -g
LDFLAGS = -Wl,--no-as-needed -lct -L../src/ -lprotobuf-c
OUTS = $(TESTS:%=%.out)

all: $(PIGS) $(TESTS)

cleanouts:
	rm -f $(OUTS)

clean: cleanouts
	rm -f $(TESTS)
	rm -f $(PIGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@

define gen-out
$(1): $(1).o
	$$(CC) -o $$@ $$^ $$(LDFLAGS)

$(1).out: $(1)
	./$$^ > $$@
	@ tail -n1 $$@
endef
$(foreach t, $(TESTS), $(eval $(call gen-out,$(t))))

run: cleanouts $(PIGS) $(OUTS)

.PHONY: all clean run
