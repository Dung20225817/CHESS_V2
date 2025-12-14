# Trình biên dịch
CC = gcc

# Cờ biên dịch: -Iinclude để tìm các file .h trong thư mục include
# -Wall để hiển thị tất cả cảnh báo, -O2 để tối ưu hóa mã nguồn
CFLAGS = -Iinclude -Wall -O2

# Danh sách các file nguồn
SRC = src/main.c src/server.c src/client_handler.c src/room.c src/board.c src/utils.c src/auth.c src/friend.c

# Tên file thực thi đầu ra (bỏ đuôi .exe)
OUT = c_server

# Quy tắc mặc định
all:
	$(CC) $(SRC) $(CFLAGS) -o $(OUT) -lpthread

# Quy tắc dọn dẹp để biên dịch lại từ đầu
clean:
	rm -f $(OUT)