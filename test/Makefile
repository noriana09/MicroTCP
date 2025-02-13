all: server client

server: test_microtcp_server.c ../lib/microtcp.c
	gcc -o server test_microtcp_server.c ../lib/microtcp.c

client: test_microtcp_client.c ../lib/microtcp.c
	gcc -o client test_microtcp_client.c ../lib/microtcp.c

test: bandwidth_test.c ../lib/microtcp.c
	gcc -o test bandwidth_test.c ../lib/microtcp.c

clean:
	rm -rf server client test save.txt
