<?php
// Kết nối database
$servername = "localhost";
$username   = "root";       // đổi nếu MySQL của bạn có user khác
$password   = "";           // mật khẩu MySQL (XAMPP thường để trống)
$dbname     = "smartbox";   // tên database của bạn

$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Lấy dữ liệu từ URL
$compartment = isset($_GET['compartment']) ? intval($_GET['compartment']) : 0;
$hour        = isset($_GET['hour']) ? intval($_GET['hour']) : 0;
$minute      = isset($_GET['minute']) ? intval($_GET['minute']) : 0;
$pills       = isset($_GET['pills']) ? intval($_GET['pills']) : 0;
$status      = isset($_GET['status']) ? intval($_GET['status']) : 0;

// Kiểm tra dữ liệu
if ($compartment > 0 && $hour >= 0 && $minute >= 0 && $pills > 0) {
    $sql = "INSERT INTO pill_schedule2 (compartment, hour, minute, pills, status) 
            VALUES ('$compartment', '$hour', '$minute', '$pills', '$status')";
    
    if ($conn->query($sql) === TRUE) {
        $last_id = $conn->insert_id; // lấy id vừa thêm
        echo json_encode([
            "success" => true,
            "message" => "Thêm dữ liệu thành công",
            "id" => $last_id
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
        "message" => "Thiếu dữ liệu"
    ]);
}

$conn->close();
?>
