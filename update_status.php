<?php
// Kết nối database
$mysqli = new mysqli("localhost", "root", "", "smartbox");

// Kiểm tra kết nối
if ($mysqli->connect_errno) {
    http_response_code(500);
    echo "Lỗi kết nối MySQL: " . $mysqli->connect_error;
    exit();
}

// Lấy dữ liệu từ request (ESP sẽ gửi qua URL hoặc HTTP GET/POST)
$id = isset($_REQUEST['id']) ? intval($_REQUEST['id']) : 0;
$status = isset($_REQUEST['status']) ? intval($_REQUEST['status']) : -1;

if ($id > 0 && ($status == 0 || $status == 1)) {
    // Cập nhật trạng thái
    $stmt = $mysqli->prepare("UPDATE pill_schedule2 SET status = ? WHERE id = ?");
    $stmt->bind_param("ii", $status, $id);
    
    if ($stmt->execute()) {
        echo "OK";
    } else {
        http_response_code(500);
        echo "Lỗi cập nhật dữ liệu!";
    }

    $stmt->close();
} else {
    http_response_code(400);
    echo "Thiếu hoặc sai tham số!";
}

$mysqli->close();
?>
