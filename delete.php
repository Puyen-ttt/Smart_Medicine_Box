<?php
// Kết nối database
$servername = "localhost";
$username   = "root";
$password   = "";
$dbname     = "smartbox";   // database của bạn

$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Lấy id từ URL
$id = isset($_GET['id']) ? intval($_GET['id']) : 0;

if ($id > 0) {
    $sql = "DELETE FROM pill_schedule2 WHERE id = $id";
    if ($conn->query($sql) === TRUE) {
        echo json_encode([
            "success" => true,
            "message" => "Xóa lịch thành công",
            "id" => $id
        ]);
    } else {
        echo json_encode([
            "success" => false,
            "message" => "Lỗi: " . $conn->error
        ]);
    }
} else {
    echo json_encode([
        "success" => false,
        "message" => "Thiếu id"
    ]);
}

$conn->close();
?>
