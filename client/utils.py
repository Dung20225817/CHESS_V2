# Các hàm hỗ trợ parse FEN, Unicode quân cờ
# Map ký tự sang Emoji Unicode cho đẹp
PIECE_UNICODE = {
    'K': "♔", 'Q': "♕", 'R': "♖", 'B': "♗", 'N': "♘", 'P': "♙", # Trắng
    'k': "♚", 'q': "♛", 'r': "♜", 'b': "♝", 'n': "♞", 'p': "♟", # Đen
    '.': ""
}

def parse_fen(fen):
    """Chuyển chuỗi FEN thành mảng 2 chiều 8x8"""
    if not fen or fen == "................................................................":
        # Bàn cờ rỗng hoặc khởi tạo
        return [['.' for _ in range(8)] for _ in range(8)]
    
    parts = fen.split('/')
    if len(parts) == 8:
        return [list(row) for row in parts]
    
    # Fallback cho chuỗi raw 64 ký tự
    if len(fen) >= 64:
        return [list(fen[i*8 : (i+1)*8]) for i in range(8)]
    
    return [['.' for _ in range(8)] for _ in range(8)]

def get_piece_color(piece):
    if piece == '.': return None
    return 'white' if piece.isupper() else 'black'