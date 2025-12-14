#ifndef FRIEND_H
#define FRIEND_H

// Định nghĩa struct Friend để dùng chung
typedef struct Friend {
    int id;
    char username[32];
} Friend;

// Định nghĩa struct FriendRequest để trả về danh sách lời mời
typedef struct FriendRequest {
    int msg_id;
    int from_id;
    char from_username[32];
} FriendRequest;

// Struct cho thách đấu
typedef struct Challenge {
    int msg_id;
    int from_id;
    char from_username[32];
} Challenge;

// --- Các hàm tiện ích ---
int get_username_from_id(int user_id, char *username);

// --- Các hàm tính năng ---
// 1. Tìm kiếm người dùng theo tên (gần đúng)
int search_users(const char *keyword, Friend *results, int max_size);

// 2. Lấy danh sách bạn bè
int get_friends(int user_id, Friend *friends, int max_size);

// 3. Gửi yêu cầu kết bạn
int send_friend_request(int from_id, int to_id);

// 4. Lấy danh sách lời mời kết bạn đang chờ (Pending)
int get_friend_requests(int user_id, FriendRequest *requests, int max_size);

// 5. Chấp nhận yêu cầu kết bạn
int accept_friend_request(int msg_id, int user_id);

// --- THÁCH ĐẤU ---
// Gửi lời thách đấu
int send_challenge(int from_id, int to_id);

// Lấy danh sách lời thách đấu đang chờ (cho người nhận)
int get_incoming_challenges(int user_id, Challenge *challenges, int max_size);

// Chấp nhận lời thách đấu (cập nhật status -> accepted)
int accept_challenge(int msg_id, int user_id);

// Kiểm tra trạng thái lời thách đấu mình đã gửi (cho người gửi polling)
// Trả về: 0 (pending), 1 (accepted - vào game), -1 (rejected/none)
int check_challenge_status(int from_id, int to_id);

#endif // FRIEND_H