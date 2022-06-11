CC = gcc
BIN = binary/
INC = include/
OBJ = object/
SRC = source/
LIB = lib/
PROTO_BUF = /usr/local/
CFLAGS = -g -Wall -I $(INC)
LDFLAGS = ${PROTO_BUF}lib/ -lprotobuf-c.a
CLIENTREC = table.o list.o data.o entry.o sdmessage.pb-c.o message.o network_client.o client_stub.o table-client.o
SERVERREC = table.o list.o data.o entry.o sdmessage.pb-c.o message.o table_skel.o network_server.o table-server.o fifo.o
CLIENTLIB = table.o list.o data.o entry.o sdmessage.pb-c.o message.o client_stub.o network_client.o

exec: table-server table-client client-lib.o

client-lib.o: $(CLIENTLIB)
	ld -r $(addprefix $(OBJ), $(CLIENTLIB)) -o $(LIB)client-lib.o

table-server: $(SERVERREC)
	$(CC) $(addprefix $(OBJ), $(SERVERREC)) -L/usr/local/lib -lprotobuf-c -lrt -lpthread -lm -lzookeeper_mt -o $(BIN)table-server

table-client: $(CLIENTREC)
	$(CC) $(addprefix $(OBJ), $(CLIENTREC)) -L/usr/local/lib -lprotobuf-c -lzookeeper_mt -o $(BIN)table-client

sdmessage.pb-c.o:
	$(PROTO_BUF)bin/protoc-c --c_out=. sdmessage.proto
	mv -f sdmessage.pb-c.h $(INC)sdmessage.pb-c.h
	mv -f sdmessage.pb-c.c $(SRC)sdmessage.pb-c.c
	gcc -c -I $(INC) $(SRC)sdmessage.pb-c.c
	mv -f sdmessage.pb-c.o $(OBJ)sdmessage.pb-c.o


%.o: $(SRC)%.c $($@)
	gcc $(CFLAGS) -lrt -lpthread -lm -lzookeeper_mt -c -o $(OBJ)$@ $<

clean:
	rm -f $(BIN)*
	rm -f $(OBJ)*
	rm -f $(LIB)*
