# Logic kiểm tra nước đi - port từ chessRules.ts
class ChessRules:
    @staticmethod
    def is_player_piece(piece, player_color):
        if piece == '.': return False
        is_white = piece.isupper()
        return is_white if player_color == 'white' else not is_white

    @staticmethod
    def notation_to_pos(notation):
        if len(notation) != 2: return None
        col = ord(notation[0]) - ord('a')
        row = 8 - int(notation[1])
        if not (0 <= col <= 7 and 0 <= row <= 7): return None
        return (row, col)

    @staticmethod
    def pos_to_notation(row, col):
        file = chr(ord('a') + col)
        rank = str(8 - row)
        return f"{file}{rank}"

    @staticmethod
    def get_piece_at(board, row, col):
        if not (0 <= row <= 7 and 0 <= col <= 7): return ''
        return board[row][col]

    @staticmethod
    def is_path_clear(board, r1, c1, r2, c2):
        row_dir = 0 if r2 == r1 else (1 if r2 > r1 else -1)
        col_dir = 0 if c2 == c1 else (1 if c2 > c1 else -1)
        
        curr_r, curr_c = r1 + row_dir, c1 + col_dir
        while curr_r != r2 or curr_c != c2:
            if board[curr_r][curr_c] != '.': return False
            curr_r += row_dir
            curr_c += col_dir
        return True

    @staticmethod
    def is_valid_move(board, move_from_str, move_to_str, player_color):
        """
        Trả về (bool, reason)
        """
        p_from = ChessRules.notation_to_pos(move_from_str)
        p_to = ChessRules.notation_to_pos(move_to_str)
        if not p_from or not p_to: return False, "Invalid position"
        
        r1, c1 = p_from
        r2, c2 = p_to
        
        piece = board[r1][c1]
        target = board[r2][c2]

        if not ChessRules.is_player_piece(piece, player_color):
            return False, "Không phải quân của bạn"
        
        # Không được ăn quân mình
        if target != '.' and ChessRules.is_player_piece(target, player_color):
            return False, "Không thể ăn quân mình"

        piece_type = piece.lower()
        valid = False
        
        dx = abs(c2 - c1)
        dy = abs(r2 - r1)

        if piece_type == 'p': # Pawn
            direction = -1 if player_color == 'white' else 1
            start_row = 6 if player_color == 'white' else 1
            row_diff = r2 - r1
            
            # Đi thẳng
            if dx == 0:
                if row_diff == direction and target == '.':
                    valid = True
                elif r1 == start_row and row_diff == 2 * direction:
                    mid_row = r1 + direction
                    if board[mid_row][c1] == '.' and target == '.':
                        valid = True
            # Ăn chéo
            elif dx == 1 and row_diff == direction and target != '.':
                valid = True

        elif piece_type == 'r': # Rook
            if dx == 0 or dy == 0:
                valid = ChessRules.is_path_clear(board, r1, c1, r2, c2)

        elif piece_type == 'n': # Knight
            if (dy == 2 and dx == 1) or (dy == 1 and dx == 2):
                valid = True

        elif piece_type == 'b': # Bishop
            if dx == dy:
                valid = ChessRules.is_path_clear(board, r1, c1, r2, c2)

        elif piece_type == 'q': # Queen
            if dx == dy or dx == 0 or dy == 0:
                valid = ChessRules.is_path_clear(board, r1, c1, r2, c2)

        elif piece_type == 'k': # King
            if dx <= 1 and dy <= 1:
                valid = True

        if valid:
            return True, "OK"
        return False, "Nước đi không hợp lệ theo luật"