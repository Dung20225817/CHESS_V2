import tkinter as tk
from tkinter import messagebox, ttk, Toplevel
from network import NetworkClient
from gui_board import BoardGUI
from utils import parse_fen
import socket

class ChessApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Chess Client - Social Edition")
        self.root.geometry("1000x700")

        self.net = NetworkClient()
        
        # --- STATE ---
        self.user_id = None
        self.username = "Unknown"
        self.player_color = None
        self.turn_color = "white"
        self.board_state = parse_fen("")
        
        # Lists data
        self.friends_list = []      # Bạn bè
        self.search_results = []    # Tìm kiếm
        self.pending_requests = []  # Lời mời kết bạn
        self.challenges_list = []   # Lời thách đấu nhận được

        # Challenge Polling State
        self.is_polling_challenge = False
        self.challenge_target_id = None
        self.wait_window = None

        # --- MAIN CONTAINER ---
        self.container = tk.Frame(root)
        self.container.pack(fill="both", expand=True, padx=20, pady=20)
        
        self.show_login()

    def clear_container(self):
        for w in self.container.winfo_children():
            w.destroy()

    # ==========================================
    # 1. MÀN HÌNH ĐĂNG NHẬP
    # ==========================================
    def show_login(self):
        self.clear_container()
        tk.Label(self.container, text="CHESS ONLINE", font=("Arial", 28, "bold"), fg="#8b4513").pack(pady=30)
        
        frame = tk.Frame(self.container, bg="#f0f0f0", padx=20, pady=20, bd=1, relief="solid")
        frame.pack()

        # Dòng nhập IP
        tk.Label(frame, text="Server IP:", font=("Arial", 12)).grid(row=0, column=0, pady=10, sticky='e')
        
        ip_frame = tk.Frame(frame) # Frame con để chứa ô nhập + nút scan
        ip_frame.grid(row=0, column=1, pady=10, padx=10, sticky="w")
        
        self.ent_ip = tk.Entry(ip_frame, font=("Arial", 12), width=15)
        self.ent_ip.pack(side="left")
        self.ent_ip.insert(0, "127.0.0.1") 

        # NÚT QUÉT TỰ ĐỘNG (MỚI)
        tk.Button(ip_frame, text="📡 Quét", command=self.scan_server, 
                  bg="#FF9800", fg="white", font=("Arial", 9)).pack(side="left", padx=5)

        # ... (Phần Username/Pass giữ nguyên) ...
        tk.Label(frame, text="Username:", font=("Arial", 12)).grid(row=1, column=0, pady=10, sticky='e')
        self.ent_user = tk.Entry(frame, font=("Arial", 12))
        self.ent_user.grid(row=1, column=1, pady=10, padx=10)

        tk.Label(frame, text="Password:", font=("Arial", 12)).grid(row=2, column=0, pady=10, sticky='e')
        self.ent_pass = tk.Entry(frame, show="*", font=("Arial", 12))
        self.ent_pass.grid(row=2, column=1, pady=10, padx=10)

        btn_frame = tk.Frame(self.container)
        btn_frame.pack(pady=30)
        
        tk.Button(btn_frame, text="Đăng Nhập", command=lambda: self.connect_and_auth("LOGIN"), 
                  bg="#4CAF50", fg="white", width=15, pady=5).pack(side="left", padx=10)
        tk.Button(btn_frame, text="Đăng Ký", command=lambda: self.connect_and_auth("REGISTER"), 
                  bg="#2196F3", fg="white", width=15, pady=5).pack(side="left", padx=10)

    # --- HÀM QUÉT SERVER (MỚI) ---
    def scan_server(self):
        self.ent_ip.delete(0, tk.END)
        self.ent_ip.insert(0, "Đang quét...")
        self.root.update()

        try:
            # Tạo socket UDP để lắng nghe
            udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            udp_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            udp_sock.settimeout(3.0) # Chỉ quét trong 3 giây
            udp_sock.bind(("", 6001)) # Lắng nghe cổng 6001

            print("Đang lắng nghe tín hiệu từ server...")
            msg, addr = udp_sock.recvfrom(1024)
            server_ip = addr[0]
            message = msg.decode('utf-8')

            if message == "CHESS_SERVER_HERE":
                self.ent_ip.delete(0, tk.END)
                self.ent_ip.insert(0, server_ip)
                messagebox.showinfo("Thành công", f"Tìm thấy Server tại: {server_ip}")
            else:
                self.ent_ip.delete(0, tk.END)
                messagebox.showwarning("Thất bại", "Nhận được tín hiệu lạ.")
            
            udp_sock.close()

        except socket.timeout:
            self.ent_ip.delete(0, tk.END)
            messagebox.showwarning("Không tìm thấy", "Không thấy Server nào trong mạng Wifi/LAN này.\nHãy chắc chắn Server đã bật và tường lửa cho phép.")
        except Exception as e:
            self.ent_ip.delete(0, tk.END)
            messagebox.showerror("Lỗi", str(e))

    # ==========================================
    # 2. HOMEPAGE (LOBBY)
    # ==========================================
    # --- NETWORK REQUEST MỚI ---
    def req_get_rooms(self):
        self.net.send({"type": "GET_ROOMS"})

    def show_lobby(self):
        self.clear_container()
        
        # --- HEADER ---
        header = tk.Frame(self.container)
        header.pack(fill="x", pady=(0, 20))
        tk.Label(header, text=f"👤 {self.username} (ID: {self.user_id})", 
                 font=("Arial", 16, "bold"), fg="#2E7D32").pack(side="left")
        tk.Button(header, text="🚪 Đăng xuất", command=self.do_logout, bg="#757575", fg="white").pack(side="right")

        # --- LAYOUT 2 CỘT ---
        content = tk.Frame(self.container)
        content.pack(fill="both", expand=True)

        # >>> CỘT TRÁI: DANH SÁCH PHÒNG (SỬA LẠI) <<<
        left_panel = tk.LabelFrame(content, text="🎮 Danh Sách Phòng", font=("Arial", 12, "bold"), padx=10, pady=10)
        left_panel.pack(side="left", fill="both", expand=True, padx=(0, 10))

        # 1. Khu vực nhập thủ công
        manual_frame = tk.Frame(left_panel)
        manual_frame.pack(fill="x", pady=(0, 10))
        tk.Label(manual_frame, text="Tìm/Tạo:", font=("Arial", 10)).pack(side="left")
        self.ent_room = tk.Entry(manual_frame, font=("Arial", 11), width=12)
        self.ent_room.pack(side="left", padx=5)
        tk.Button(manual_frame, text="Go", command=self.do_join_room_manual, 
                  bg="#FF9800", fg="white", width=3).pack(side="left")

        # 2. Toolbar (Nút làm mới)
        room_toolbar = tk.Frame(left_panel)
        room_toolbar.pack(fill="x")
        tk.Label(room_toolbar, text="Phòng đang online:", fg="gray").pack(side="left")
        tk.Button(room_toolbar, text="🔄", command=self.req_get_rooms, bd=1).pack(side="right")

        # 3. Vùng hiển thị danh sách phòng (QUAN TRỌNG: Đây là cái bạn đang thiếu)
        self.room_scroll_frame = self.create_scrollable_area(left_panel)

        # [MỚI] Tự động cập nhật danh sách phòng mỗi 3 giây
        # self.lobby_update_job = self.root.after(3000, self.loop_get_rooms)

        # >>> CỘT PHẢI: Hệ thống bạn bè (Tabs) <<<
        right_panel = tk.LabelFrame(content, text="🌐 Cộng Đồng", font=("Arial", 12, "bold"), padx=5, pady=5)
        right_panel.pack(side="right", fill="both", expand=True, padx=(10, 0))

        self.notebook = ttk.Notebook(right_panel)
        self.notebook.pack(fill="both", expand=True)

        # Tab 1: Bạn Bè
        self.tab_friends = tk.Frame(self.notebook, bg="white")
        self.notebook.add(self.tab_friends, text="   Bạn Bè   ")
        self.setup_friends_tab()

        # Tab 2: Thách Đấu (Mới)
        self.tab_challenges = tk.Frame(self.notebook, bg="white")
        self.notebook.add(self.tab_challenges, text=" ⚔️ Thách Đấu ")
        self.setup_challenges_tab()

        # Tab 3: Tìm Kiếm
        self.tab_search = tk.Frame(self.notebook, bg="white")
        self.notebook.add(self.tab_search, text="   Tìm Kiếm   ")
        self.setup_search_tab()

        # Tab 4: Lời Mời KB
        self.tab_requests = tk.Frame(self.notebook, bg="white")
        self.notebook.add(self.tab_requests, text="   Kết Bạn 🔔   ")
        self.setup_requests_tab()

        self.notebook.bind("<<NotebookTabChanged>>", self.on_tab_change)
        self.req_refresh_friends()
        self.req_get_rooms() # <--- Gọi hàm lấy phòng ngay khi vào Lobby

    # --- HÀM CẬP NHẬT GIAO DIỆN PHÒNG (MỚI) ---
    def update_rooms_ui(self, rooms_data):
        # Xóa danh sách cũ
        for w in self.room_scroll_frame.winfo_children(): w.destroy()

        if not rooms_data:
            tk.Label(self.room_scroll_frame, text="Chưa có phòng nào.", fg="gray").pack(pady=10)
            return

        for r in rooms_data:
            r_name = r.get("name")
            r_count = r.get("count")
            
            # Tạo dòng hiển thị phòng
            row = tk.Frame(self.room_scroll_frame, bg="white", pady=5, bd=1, relief="ridge")
            row.pack(fill="x", padx=2, pady=2)
            
            # Icon trạng thái (Xanh = Trống, Đỏ = Đầy)
            status_color = "green" if r_count < 2 else "red"
            tk.Label(row, text="●", fg=status_color, bg="white").pack(side="left", padx=5)
            
            tk.Label(row, text=f"{r_name} ({r_count}/2)", bg="white", font=("Arial", 11, "bold")).pack(side="left")
            
            # Nút Join
            state = "normal" if r_count < 2 else "disabled"
            btn_text = "Vào" if r_count < 2 else "Full"
            tk.Button(row, text=btn_text, state=state, bg="#2196F3", fg="white", font=("Arial", 9),
                      command=lambda rn=r_name: self.do_join_specific_room(rn)).pack(side="right", padx=5)

    # --- UI TAB: BẠN BÈ ---
    def setup_friends_tab(self):
        tb = tk.Frame(self.tab_friends, bg="white", pady=5)
        tb.pack(fill="x")
        tk.Button(tb, text="🔄 Refresh", command=self.req_refresh_friends, bg="#eee").pack(side="right", padx=5)
        self.friends_scroll_frame = self.create_scrollable_area(self.tab_friends)

    def update_friends_ui(self):
        for w in self.friends_scroll_frame.winfo_children(): w.destroy()
        if not self.friends_list:
            tk.Label(self.friends_scroll_frame, text="Chưa có bạn bè.", bg="white", fg="gray").pack(pady=20)
            return

        for friend in self.friends_list:
            fid = friend.get("id")
            fname = friend.get("username")
            row = tk.Frame(self.friends_scroll_frame, bg="white", pady=5, bd=1, relief="groove")
            row.pack(fill="x", padx=5, pady=2)
            
            tk.Label(row, text="👤", bg="white", font=("Arial", 12)).pack(side="left", padx=5)
            tk.Label(row, text=f"{fname} (ID:{fid})", bg="white", font=("Arial", 11, "bold")).pack(side="left")
            
            # Nút Thách đấu mới: Gửi request thay vì join thẳng
            tk.Button(row, text="⚔️ Thách đấu", bg="#e53935", fg="white", 
                      command=lambda ti=fid, tn=fname: self.initiate_challenge(ti, tn)).pack(side="right", padx=5)

    # --- UI TAB: THÁCH ĐẤU (CHALLENGES) ---
    def setup_challenges_tab(self):
        tb = tk.Frame(self.tab_challenges, bg="white", pady=5)
        tb.pack(fill="x")
        tk.Button(tb, text="🔄 Tải lại", command=self.req_get_challenges, bg="#eee").pack(side="right", padx=5)
        self.challenges_scroll_frame = self.create_scrollable_area(self.tab_challenges)

    def update_challenges_ui(self):
        for w in self.challenges_scroll_frame.winfo_children(): w.destroy()
        if not self.challenges_list:
            tk.Label(self.challenges_scroll_frame, text="Không có lời thách đấu nào.", bg="white", fg="gray").pack(pady=20)
            return

        for chall in self.challenges_list:
            msg_id = chall.get("msg_id")
            from_name = chall.get("username")
            from_id = chall.get("from_id")

            row = tk.Frame(self.challenges_scroll_frame, bg="#FFEBEE", pady=5, bd=1, relief="raised")
            row.pack(fill="x", padx=5, pady=2)
            
            tk.Label(row, text=f"⚔️ {from_name} mời solo!", bg="#FFEBEE", font=("Arial", 10, "bold"), fg="#D32F2F").pack(side="left", padx=10)
            tk.Button(row, text="✅ Chấp nhận", bg="#D32F2F", fg="white",
                      command=lambda mid=msg_id, fid=from_id: self.req_accept_challenge(mid, fid)).pack(side="right", padx=5)

    # --- UI TAB: TÌM KIẾM ---
    def setup_search_tab(self):
        bar = tk.Frame(self.tab_search, bg="white", pady=10)
        bar.pack(fill="x", padx=10)
        self.ent_search = tk.Entry(bar, font=("Arial", 12))
        self.ent_search.pack(side="left", fill="x", expand=True)
        tk.Button(bar, text="🔍 Tìm", command=self.req_search_user, bg="#2196F3", fg="white").pack(side="left", padx=5)
        self.search_scroll_frame = self.create_scrollable_area(self.tab_search)

    def update_search_ui(self):
        for w in self.search_scroll_frame.winfo_children(): w.destroy()
        if not self.search_results:
            tk.Label(self.search_scroll_frame, text="Không tìm thấy kết quả.", bg="white", fg="gray").pack(pady=20)
            return
        for user in self.search_results:
            uid = user.get("id")
            uname = user.get("username")
            if str(uid) == str(self.user_id): continue
            row = tk.Frame(self.search_scroll_frame, bg="white", pady=5, bd=1, relief="groove")
            row.pack(fill="x", padx=5, pady=2)
            tk.Label(row, text=f"{uname}", bg="white", font=("Arial", 11)).pack(side="left", padx=10)
            tk.Button(row, text="➕ Kết bạn", bg="#4CAF50", fg="white",
                      command=lambda ti=uid: self.req_send_friend_request(ti)).pack(side="right", padx=5)

    # --- UI TAB: LỜI MỜI KB ---
    def setup_requests_tab(self):
        tb = tk.Frame(self.tab_requests, bg="white", pady=5)
        tb.pack(fill="x")
        tk.Button(tb, text="🔄 Tải lại", command=self.req_get_requests, bg="#eee").pack(side="right", padx=5)
        self.req_scroll_frame = self.create_scrollable_area(self.tab_requests)

    def update_requests_ui(self):
        for w in self.req_scroll_frame.winfo_children(): w.destroy()
        if not self.pending_requests:
            tk.Label(self.req_scroll_frame, text="Không có lời mời nào.", bg="white", fg="gray").pack(pady=20)
            return
        for req in self.pending_requests:
            msg_id = req.get("msg_id")
            from_name = req.get("username")
            row = tk.Frame(self.req_scroll_frame, bg="#FFF8E1", pady=5, bd=1, relief="raised")
            row.pack(fill="x", padx=5, pady=2)
            tk.Label(row, text=f"📩 {from_name} muốn kết bạn", bg="#FFF8E1", font=("Arial", 10)).pack(side="left", padx=10)
            tk.Button(row, text="✅ Chấp nhận", bg="#4CAF50", fg="white",
                      command=lambda mid=msg_id: self.req_accept_request(mid)).pack(side="right", padx=5)

    def create_scrollable_area(self, parent):
        canvas = tk.Canvas(parent, bg="white")
        scrollbar = ttk.Scrollbar(parent, orient="vertical", command=canvas.yview)
        scroll_frame = tk.Frame(canvas, bg="white")
        scroll_frame.bind("<Configure>", lambda e: canvas.configure(scrollregion=canvas.bbox("all")))
        canvas.create_window((0, 0), window=scroll_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)
        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")
        return scroll_frame

    def on_tab_change(self, event):
        idx = self.notebook.index(self.notebook.select())
        if idx == 0: self.req_refresh_friends()
        elif idx == 1: self.req_get_challenges()
        elif idx == 3: self.req_get_requests()

    # ==========================================
    # 3. MÀN HÌNH GAME
    # ==========================================
    def show_game(self, room_name):
        # Nếu đang có popup chờ, tắt nó đi
        if self.wait_window:
            self.wait_window.destroy()
            self.wait_window = None
            self.is_polling_challenge = False

        self.clear_container()
        
        # --- Cột bên phải (Thông tin & Log) ---
        info_panel = tk.Frame(self.container, width=250)
        info_panel.pack(side="right", fill="y", padx=10, pady=10)
        info_panel.pack_propagate(False) # Giữ chiều rộng cố định
        
        tk.Label(info_panel, text=f"Phòng: {room_name}", font=("Arial", 11, "bold")).pack(pady=(0, 10))
        
        self.lbl_status = tk.Label(info_panel, text="Đang kết nối...", fg="#f57f17", font=("Arial", 11))
        self.lbl_status.pack(pady=5)
        
        self.lbl_me = tk.Label(info_panel, text="Bạn là: ...", font=("Arial", 11))
        self.lbl_me.pack(pady=5)
        
        self.lbl_turn = tk.Label(info_panel, text="Lượt: Trắng", font=("Arial", 12, "bold"))
        self.lbl_turn.pack(pady=(5, 10))
        
        # === [ĐOẠN CẦN THÊM MỚI] ===
        tk.Label(info_panel, text="📜 Lịch sử đấu", font=("Arial", 10, "bold")).pack(anchor="w")
        
        log_frame = tk.Frame(info_panel, bg="white", bd=1, relief="sunken")
        log_frame.pack(fill="both", expand=True, pady=(5, 20))
        
        # Widget Text để hiển thị log
        self.log_text = tk.Text(log_frame, height=10, width=20, state="disabled", font=("Courier New", 10))
        log_scroll = ttk.Scrollbar(log_frame, orient="vertical", command=self.log_text.yview)
        self.log_text.configure(yscrollcommand=log_scroll.set)
        
        self.log_text.pack(side="left", fill="both", expand=True)
        log_scroll.pack(side="right", fill="y")
        # ============================

        tk.Button(info_panel, text="🚪 Rời Phòng", command=self.do_leave_room, 
                  bg="#f44336", fg="white", font=("Arial", 10, "bold")).pack(side="bottom", fill="x")

        # --- Cột bên trái (Bàn cờ) ---
        board_panel = tk.Frame(self.container)
        board_panel.pack(side="left", expand=True, fill="both", padx=10, pady=10)
        
        board_center_frame = tk.Frame(board_panel)
        board_center_frame.pack(expand=True)
        self.board_gui = BoardGUI(board_center_frame, self.send_move)
        self.board_gui.pack()
    
    # --- [MỚI] HÀM HỖ TRỢ GHI LOG (Thêm hàm này vào class ChessApp) ---
    def append_move_log(self, text):
        """Thêm một dòng vào khung lịch sử nước đi"""
        # Kiểm tra xem khung log có tồn tại không trước khi ghi
        if hasattr(self, 'log_text') and self.log_text.winfo_exists():
            self.log_text.config(state="normal")       # Mở khóa để ghi
            self.log_text.insert(tk.END, text + "\n")  # Thêm nội dung
            self.log_text.see(tk.END)                  # Tự động cuộn xuống
            self.log_text.config(state="disabled")     # Khóa lại

    # ==========================================
    # 4. NETWORK & LOGIC
    # ==========================================
    def connect_and_auth(self, auth_type):
        # 1. Lấy IP từ ô nhập liệu
        ip_address = self.ent_ip.get() 
        
        user = self.ent_user.get()
        pwd = self.ent_pass.get()
        
        if not ip_address or not user or not pwd: 
            messagebox.showwarning("Thiếu thông tin", "Vui lòng nhập IP, User và Pass")
            return

        self.username = user
        if not self.net.cb_ref:
            # 2. Truyền biến ip_address vào hàm connect thay vì chuỗi cứng
            if self.net.connect(ip_address, 6000, self.on_server_message) == 0:
                messagebox.showerror("Lỗi", f"Không kết nối được đến {ip_address}")
                return
        
        self.net.send({"type": auth_type, "username": user, "password": pwd})

    # --- SOCIAL ---
    def req_refresh_friends(self):
        if self.user_id: self.net.send({"type": "GET_FRIENDS", "user_id": int(self.user_id)})

    def req_search_user(self):
        kw = self.ent_search.get()
        if kw: self.net.send({"type": "SEARCH_USER", "keyword": kw})

    def req_send_friend_request(self, to_id):
        if self.user_id: 
            self.net.send({"type": "SEND_REQUEST", "from_id": int(self.user_id), "to_id": int(to_id)})

    def req_get_requests(self):
        if self.user_id: self.net.send({"type": "GET_REQUESTS", "user_id": int(self.user_id)})

    def req_accept_request(self, msg_id):
        if self.user_id:
            self.net.send({"type": "ACCEPT_REQUEST", "msg_id": int(msg_id), "user_id": int(self.user_id)})

    # --- CHALLENGE LOGIC (SENDER) ---
    def initiate_challenge(self, to_id, to_name):
        if not self.user_id: return
        self.challenge_target_id = to_id
        
        # 1. Gửi lệnh
        self.net.send({"type": "SEND_CHALLENGE", "from_id": int(self.user_id), "to_id": int(to_id)})
        
        # 2. Hiện popup chờ
        self.wait_window = Toplevel(self.root)
        self.wait_window.title("Đang thách đấu...")
        self.wait_window.geometry("300x150")
        tk.Label(self.wait_window, text=f"Đang chờ {to_name} chấp nhận...", font=("Arial", 11)).pack(pady=30)
        tk.Button(self.wait_window, text="Hủy", command=self.cancel_challenge_wait).pack()

        # 3. Bắt đầu polling
        self.is_polling_challenge = True
        self.root.after(2000, self.poll_challenge_status)

    def poll_challenge_status(self):
        if not self.is_polling_challenge or not self.wait_window:
            return
        
        # Gửi lệnh check
        self.net.send({"type": "CHECK_CHALLENGE", "from_id": int(self.user_id), "to_id": int(self.challenge_target_id)})
        
        # Lặp lại sau 2s
        self.root.after(2000, self.poll_challenge_status)

    def cancel_challenge_wait(self):
        self.is_polling_challenge = False
        if self.wait_window: self.wait_window.destroy()
        self.wait_window = None

    # --- CHALLENGE LOGIC (RECEIVER) ---
    def req_get_challenges(self):
        if self.user_id: self.net.send({"type": "GET_CHALLENGES", "user_id": int(self.user_id)})

    def req_accept_challenge(self, msg_id, from_id):
        # Chấp nhận -> Server sẽ trả về room name
        self.net.send({
            "type": "ACCEPT_CHALLENGE", 
            "msg_id": int(msg_id), 
            "user_id": int(self.user_id),
            "from_id": int(from_id)
        })

    # --- GAME JOIN ---
    def do_join_room_manual(self):
        room = self.ent_room.get()
        if room: self.do_join_specific_room(room)

    def do_join_specific_room(self, room_name):
        self.net.send({"type": "join", "room": room_name})
        self.show_game(room_name)

    def send_move(self, from_sq, to_sq):
        self.net.send({"type": "move", "move": f"{from_sq}{to_sq}"})

    def do_leave_room(self):
        self.net.close()
        self.net.cb_ref = None
        self.show_login()

    def do_logout(self):
        self.do_leave_room()

    # ==========================================
    # 5. XỬ LÝ DỮ LIỆU TỪ SERVER
    # ==========================================
    def on_server_message(self, data):
        self.root.after(0, lambda: self.process_data(data))

    def process_data(self, data):
        print(f"[RECV] {data}")
        msg_type = data.get("type")

        # AUTH
        if msg_type in ["LOGIN", "REGISTER"]:
            if data.get("success"):
                self.user_id = data.get("user_id")
                self.show_lobby()
            else:
                messagebox.showerror("Lỗi", "Thất bại!")
                self.net.close()
                self.net.cb_ref = None

        # SOCIAL FRIENDS
        elif msg_type == "GET_FRIENDS":
            self.friends_list = data.get("friends", [])
            if hasattr(self, 'friends_scroll_frame'): self.update_friends_ui()

        elif msg_type == "SEARCH_RESULT":
            self.search_results = data.get("users", [])
            if hasattr(self, 'search_scroll_frame'): self.update_search_ui()

        elif msg_type == "SEND_REQUEST":
            if data.get("success"): messagebox.showinfo("Info", "Đã gửi lời mời!")
            else: messagebox.showwarning("Info", data.get("msg", "Lỗi"))

        elif msg_type == "GET_REQUESTS":
            self.pending_requests = data.get("requests", [])
            if hasattr(self, 'req_scroll_frame'): self.update_requests_ui()

        elif msg_type == "ACCEPT_REQUEST":
            if data.get("success"):
                messagebox.showinfo("Info", "Đã kết bạn!")
                self.req_refresh_friends()
                self.req_get_requests()

        # SOCIAL CHALLENGES
        elif msg_type == "SEND_CHALLENGE":
            if not data.get("success"):
                messagebox.showerror("Lỗi", "Không thể gửi lời mời (đã gửi hoặc lỗi)")
                self.cancel_challenge_wait()

        elif msg_type == "GET_CHALLENGES":
            self.challenges_list = data.get("challenges", [])
            if hasattr(self, 'challenges_scroll_frame'): self.update_challenges_ui()

        elif msg_type == "CHALLENGE_ACCEPTED":
            # Xảy ra khi:
            # 1. Mình là người nhận bấm "Accept" -> Server trả về room
            # 2. Mình là người gửi đang Poll -> Server trả về room
            if data.get("success"):
                room_name = data.get("room")
                # Dừng polling (nếu có) và vào game
                self.cancel_challenge_wait()
                self.do_join_specific_room(room_name)

        elif msg_type == "CHALLENGE_STATUS":
            # Phản hồi của lệnh CHECK_CHALLENGE
            status = data.get("status")
            if status == "pending":
                pass # Tiếp tục chờ
            elif status == "none":
                # Có thể đối thủ đã từ chối hoặc hết hạn (chưa implement reject)
                # Hoặc lỗi mạng
                pass 

        elif msg_type == "ROOM_LIST":
            rooms = data.get("rooms", [])
            if hasattr(self, 'room_scroll_frame'):
                self.update_rooms_ui(rooms)

        # GAMEPLAY
        elif msg_type == "assignColor":
            self.player_color = data.get("color")
            if hasattr(self, 'lbl_me') and self.lbl_me:
                c_txt = "⚪ TRẮNG" if self.player_color == "white" else "⚫ ĐEN"
                self.lbl_me.config(text=f"Bạn cầm quân: {c_txt}", fg="blue")
            if hasattr(self, 'board_gui'): self.board_gui.player_color = self.player_color
            if hasattr(self, 'lbl_status'): self.lbl_status.config(text="Đã vào game!", fg="green")

        elif msg_type == "move_notify":
            move_str = data.get("move")
            next_turn = data.get("turn")
            
            # Logic hiển thị log
            # Nếu lượt đi tiếp theo LÀ của mình -> Tức là đối thủ vừa đi
            if next_turn == self.player_color:
                self.append_move_log(f"Đối thủ: {move_str}")
            
            # Cập nhật lượt đi hiển thị (dự phòng nếu gói state đến chậm)
            self.turn_color = next_turn
            if hasattr(self, 'lbl_turn'):
                t_txt = "⚪ Trắng" if next_turn == "white" else "⚫ Đen"
                self.lbl_turn.config(text=f"Lượt: {t_txt}")
    
        elif msg_type == "state":
            fen = data.get("fen")
            turn = data.get("turn")
            self.board_state = parse_fen(fen)
            self.turn_color = turn
            if hasattr(self, 'lbl_turn'):
                t_txt = "⚪ Trắng" if turn == "white" else "⚫ Đen"
                self.lbl_turn.config(text=f"Lượt: {t_txt}")
            if hasattr(self, 'board_gui'):
                is_my_turn = (self.player_color == self.turn_color)
                self.board_gui.update_board(self.board_state, self.player_color, is_my_turn)

        elif msg_type == "gameOver":
            winner = data.get("winner")
            
            # [MỚI] Xử lý trường hợp đối thủ thoát
            if winner == "opponent_disconnect":
                reason = data.get("reason", "")
                msg = f"Đối thủ đã thoát hoặc mất kết nối ({reason}). Bạn thắng!"
                messagebox.showinfo("Kết thúc", msg)
                self.append_move_log(f"--- {msg} ---")
            else:
                result_text = f"Kết thúc! Người thắng: {winner}"
                messagebox.showinfo("Game Over", result_text)
                self.append_move_log(f"--- {result_text} ---")
            
            # Reset trạng thái
            if hasattr(self, 'lbl_status'):
                self.lbl_status.config(text="Ván đấu kết thúc", fg="red")

    def on_close(self):
        self.is_polling_challenge = False
        try: self.net.close()
        except: pass
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = ChessApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()