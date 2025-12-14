CC = gcc
CFLAGS = -Wall -O2 -fPIC 
# -fPIC là bắt buộc để tạo thư viện chia sẻ
# -shared: tạo file .so
LDFLAGS = -shared -lpthread

SRC = src/client_lib.c
OUT = client_lib.so

all:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(OUT) $(SRC)

clean:
	rm -f $(OUT)