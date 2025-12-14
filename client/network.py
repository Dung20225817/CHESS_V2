# Xử lý giao tiếp với thư viện C - ctypes
import ctypes
import json
import os

# Định nghĩa Callback type
CALLBACK_TYPE = ctypes.CFUNCTYPE(None, ctypes.c_char_p)

class NetworkClient:
    def __init__(self, lib_path="./client_lib.so"):
        if not os.path.exists(lib_path):
            raise FileNotFoundError(f"Không tìm thấy {lib_path}. Hãy chạy 'make' trước.")
        
        try:
            self.lib = ctypes.CDLL(lib_path)
        except OSError as e:
            raise RuntimeError(f"Không thể load thư viện C: {e}")

        # Cấu hình input/output cho hàm C
        self.lib.connect_server.argtypes = [ctypes.c_char_p, ctypes.c_int, CALLBACK_TYPE]
        self.lib.connect_server.restype = ctypes.c_int

        self.lib.send_msg.argtypes = [ctypes.c_char_p]
        self.lib.send_msg.restype = None

        self.lib.close_connection.argtypes = []
        self.lib.close_connection.restype = None

        # Giữ tham chiếu callback để không bị Garbage Collector xóa
        self.cb_ref = None

    def connect(self, ip, port, on_receive_func):
        """
        ip: str
        port: int
        on_receive_func: function(json_dict) -> void
        """
        def internal_callback(raw_data):
            if raw_data:
                try:
                    str_data = raw_data.decode('utf-8')
                    # Xử lý trường hợp server gửi nhiều JSON dính nhau (stream)
                    # Ở đây ta giả định server gửi từng gói hoặc split theo newline nếu cần
                    # Để an toàn, ta split theo \n và parse từng cái
                    parts = str_data.split('\n')
                    for part in parts:
                        if not part.strip(): continue
                        data = json.loads(part)
                        on_receive_func(data)
                except json.JSONDecodeError:
                    print(f"[Network Error] Bad JSON: {raw_data}")
                except Exception as e:
                    print(f"[Network Error] {e}")

        self.cb_ref = CALLBACK_TYPE(internal_callback)
        ip_bytes = ip.encode('utf-8')
        return self.lib.connect_server(ip_bytes, port, self.cb_ref)

    def send(self, data_dict):
        try:
            json_str = json.dumps(data_dict, separators=(',', ':'))
            self.lib.send_msg(json_str.encode('utf-8'))
        except Exception as e:
            print(f"[Send Error] {e}")

    def close(self):
        self.lib.close_connection()