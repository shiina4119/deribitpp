CPPFLAGS=-g
LDFLAGS=-g
LDLIBS=-lssl -lcrypto -lfmt
OUT_DIR=dist
MKDIR_P = mkdir -p

all: main receiver
        	
main: main.o trading_client.o shared_state.o broadcast_session.o listener.o helper.o
	g++ $(LDFLAGS) -o $(OUT_DIR)/main $(OUT_DIR)/main.o $(OUT_DIR)/trading_client.o $(OUT_DIR)/shared_state.o $(OUT_DIR)/broadcast_session.o $(OUT_DIR)/listener.o $(OUT_DIR)/helper.o $(LDLIBS)

main.o: main.cpp trading_client.hpp
	g++ $(CPPFLAGS) -c main.cpp -o $(OUT_DIR)/main.o

trading_client.o: trading_client.cpp trading_client.hpp
	g++ $(CPPFLAGS) -c trading_client.cpp -o $(OUT_DIR)/trading_client.o
	
shared_state.o: shared_state.cpp
	g++ $(CPPFLAGS) -c shared_state.cpp -o $(OUT_DIR)/shared_state.o

broadcast_session.o: broadcast_session.cpp
	g++ $(CPPFLAGS) -c broadcast_session.cpp -o $(OUT_DIR)/broadcast_session.o

listener.o: listener.cpp
	g++ $(CPPFLAGS) -c listener.cpp -o $(OUT_DIR)/listener.o

helper.o: helper.cpp
	g++ $(CPPFLAGS) -c helper.cpp -o $(OUT_DIR)/helper.o

receiver: receiver.cpp
	g++ -g receiver.cpp -o $(OUT_DIR)/receiver
