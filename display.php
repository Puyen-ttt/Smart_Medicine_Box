<?php
// Kết nối database
$servername = "localhost";
$username   = "root";
$password   = "";
$dbname     = "smartbox";

$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Lấy dữ liệu
$sql = "SELECT id, compartment, hour, minute, pills, status FROM pill_schedule2 ORDER BY compartment, hour, minute";
$result = $conn->query($sql);

// Hiển thị HTML
echo "<!DOCTYPE html>
<html lang='vi'>
<head>
    <meta charset='UTF-8'>
    <title>Lịch uống thuốc</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h2 { text-align: center; }
        table { border-collapse: collapse; width: 100%; margin-top: 20px; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: center; }
        th { background-color: #4CAF50; color: white; }
        tr:nth-child(even) { background-color: #f2f2f2; }
    </style>
</head>
<body>
    <h2>Bảng lịch uống thuốc</h2>
    <table>
        <tr>
            <th>ID</th>
            <th>Ngăn</th>
            <th>Giờ</th>
            <th>Phút</th>
            <th>Số viên</th>
            <th>Trạng thái</th>
        </tr>";

if ($result->num_rows > 0) {
    while($row = $result->fetch_assoc()) {
        // Hiển thị status dạng chữ
        $status_text = ($row['status'] == 1) ? "Đã uống" : "Chưa uống";
        echo "<tr>
                <td>{$row['id']}</td>
                <td>{$row['compartment']}</td>
                <td>{$row['hour']}</td>
                <td>{$row['minute']}</td>
                <td>{$row['pills']}</td>
                <td>{$status_text}</td>
              </tr>";
    }
} else {
    echo "<tr><td colspan='6'>Không có dữ liệu</td></tr>";
}

echo "  </table>
</body>
</html>";

$conn->close();
?>
