all: cache_sim addr_maker

cache_sim: cache_sim.c
	gcc -Wall cache_sim.c -o cache_sim

addr_maker: addr_maker.c
	gcc -Wall addr_maker.c -o addr_maker

clean:
	rm -f cache_sim addr_maker
