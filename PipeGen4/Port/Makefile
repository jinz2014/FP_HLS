port_order : all_perm_rep.c port_order.c 
	gcc -g -DTEST_PORT_ORDERING -o $@ $^ -lm; \
  ./port_order > log;\
  vi log

all_perm_rep : all_perm_rep.c port_order.c 
	gcc -g -DTEST_PERM_REP -o $@ $^
	./all_perm_rep

all_perm: all_perm.c
	gcc -g -DTEST_PERMUTATION -o $@ $^
	./all_perm
