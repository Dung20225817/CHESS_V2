# Component vẽ bàn cờ
import tkinter as tk
from utils import PIECE_UNICODE
from chess_rules import ChessRules

class BoardGUI(tk.Frame):
    def __init__(self, parent, on_move_callback):
        super().__init__(parent)
        self.on_move = on_move_callback
        
        self.cells = {} # Map "e2" -> Button Widget
        self.board_data = [['.' for _ in range(8)] for _ in range(8)]
        self.selected_sq = None
        self.valid_moves = []
        self.player_color = None
        self.is_my_turn = False
        
        self.init_ui()

    def init_ui(self):
        # Màu sắc giống BoardGrid.tsx
        self.color_light = "#f0d9b5"
        self.color_dark = "#b58863"
        self.color_select = "#7fc97f" # Xanh lá
        self.color_valid = "#fdc689"  # Vàng cam

        # Labels cột (a-h)
        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        for c, f in enumerate(files):
            tk.Label(self, text=f, font=("Arial", 10, "bold")).grid(row=8, column=c+1)

        # Labels hàng (1-8)
        for r in range(8):
            rank_str = str(8 - r)
            tk.Label(self, text=rank_str, font=("Arial", 10, "bold")).grid(row=r, column=0)

        # Lưới bàn cờ 8x8
        for r in range(8):
            for c in range(8):
                sq_name = ChessRules.pos_to_notation(r, c)
                bg = self.color_light if (r + c) % 2 == 0 else self.color_dark
                
                btn = tk.Button(self, text="", font=("Segoe UI Symbol", 24), 
                                width=4, height=1, # Kích thước tương đối
                                relief="flat", borderwidth=0,
                                command=lambda s=sq_name: self.handle_click(s))
                btn.grid(row=r, column=c+1, padx=0, pady=0)
                
                # Lưu thông tin để update sau
                self.cells[sq_name] = {
                    'widget': btn,
                    'base_bg': bg,
                    'row': r, 'col': c
                }

    def update_board(self, board_matrix, player_color, is_turn):
        self.board_data = board_matrix
        self.player_color = player_color
        self.is_my_turn = is_turn
        
        # Reset selection nếu bàn cờ cập nhật (ví dụ sau khi đi)
        self.selected_sq = None
        self.valid_moves = []

        self.redraw()

    def redraw(self):
        for r in range(8):
            for c in range(8):
                sq = ChessRules.pos_to_notation(r, c)
                cell = self.cells[sq]
                piece = self.board_data[r][c]
                btn = cell['widget']
                
                # Set text quân cờ
                symbol = PIECE_UNICODE.get(piece, "")
                btn.config(text=symbol)
                
                # Set màu chữ (Trắng/Đen)
                fg_color = "white" if piece.isupper() else "black"
                if piece == '.': fg_color = "black"
                
                # Xử lý màu nền (Highlight)
                bg_color = cell['base_bg']
                
                if sq == self.selected_sq:
                    bg_color = self.color_select
                elif sq in self.valid_moves:
                    bg_color = self.color_valid
                
                btn.config(bg=bg_color, fg=fg_color)

    def handle_click(self, sq):
        if not self.player_color: return # Chưa vào game
        if not self.is_my_turn:
            print("Chưa đến lượt bạn!")
            return

        # Logic chọn quân / đi quân
        if self.selected_sq == sq:
            # Bỏ chọn
            self.selected_sq = None
            self.valid_moves = []
            self.redraw()
            return

        if self.selected_sq:
            # Đã chọn 1 ô -> Đây là ô đích -> Gửi move
            # Kiểm tra luật client trước khi gửi
            is_valid, reason = ChessRules.is_valid_move(
                self.board_data, self.selected_sq, sq, self.player_color
            )
            
            if is_valid:
                self.on_move(self.selected_sq, sq)
                self.selected_sq = None
                self.valid_moves = []
            else:
                print(f"Nước đi lỗi: {reason}")
                # Nếu click vào quân mình thì đổi chọn
                clicked_row, clicked_col = ChessRules.notation_to_pos(sq)
                piece = self.board_data[clicked_row][clicked_col]
                if ChessRules.is_player_piece(piece, self.player_color):
                     self.selected_sq = sq
                     self.calc_valid_moves(sq)
            self.redraw()
        else:
            # Chưa chọn -> Chọn quân nguồn
            row, col = ChessRules.notation_to_pos(sq)
            piece = self.board_data[row][col]
            if ChessRules.is_player_piece(piece, self.player_color):
                self.selected_sq = sq
                self.calc_valid_moves(sq)
                self.redraw()

    def calc_valid_moves(self, from_sq):
        # Tính toán sơ bộ các nước đi có thể để highlight
        self.valid_moves = []
        for r in range(8):
            for c in range(8):
                to_sq = ChessRules.pos_to_notation(r, c)
                valid, _ = ChessRules.is_valid_move(
                    self.board_data, from_sq, to_sq, self.player_color
                )
                if valid:
                    self.valid_moves.append(to_sq)