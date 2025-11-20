<?php
$conn = new mysqli("localhost", "root", "", "smartbox");
if ($conn->connect_error) die("Kết nối thất bại: " . $conn->connect_error);

// Xử lý thêm lịch
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['add'])) {
    $compartment = (int)$_POST['compartment'];
    $hour = (int)$_POST['hour'];
    $minute = (int)$_POST['minute'];
    $pills = (int)$_POST['pills'];
    
    $sql = "INSERT INTO pill_schedule2 (compartment, hour, minute, pills, status) 
            VALUES ($compartment, $hour, $minute, $pills, 0)"; // status mặc định là 0 (chưa uống)
    $conn->query($sql);
    header("Location: index.php");
    exit;
}

// Xử lý xóa lịch
if (isset($_GET['delete'])) {
    $id = (int)$_GET['delete'];
    $conn->query("DELETE FROM pill_schedule2 WHERE id=$id");
    header("Location: index.php");
    exit;
}
?>
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <title>Hộp thuốc thông minh</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; background: #f5f7fa; }
    h2 { color: #2c3e50; }
    table { width: 100%; border-collapse: collapse; margin-bottom: 20px; background: #fff; }
    th, td { border: 1px solid #ddd; padding: 8px; text-align: center; }
    th { background: #3498db; color: white; }
    tr:nth-child(even) { background: #f9f9f9; }
    form { margin-bottom: 10px; }
    input { padding: 5px; margin: 0 5px; }
    button { padding: 5px 10px; border: none; border-radius: 4px; cursor: pointer; }
    button.add { background: #2ecc71; color: white; }
    a.delete { background: #e74c3c; color: white; padding: 5px 10px; border-radius: 4px; text-decoration: none; }
  </style>
</head>
<body>
<h1>💊 Hộp thuốc thông minh</h1>

<?php
for ($comp = 1; $comp <= 6; $comp++) {
    echo "<h2>Ngăn $comp</h2>";

    // Form thêm lịch
    echo "
    <form method='POST'>
        <input type='hidden' name='compartment' value='$comp'>
        Giờ: <input type='number' name='hour' min='0' max='23' required>
        Phút: <input type='number' name='minute' min='0' max='59' required>
        Số viên: <input type='number' name='pills' min='1' required>
        <button type='submit' name='add' class='add'>Thêm +</button>
    </form>
    ";

    // Lấy lịch
    $sql = "SELECT * FROM pill_schedule2 WHERE compartment=$comp ORDER BY hour, minute";
    $result = $conn->query($sql);

    if ($result->num_rows > 0) {
        echo "<table>
                <tr>
                  <th>Giờ</th>
                  <th>Phút</th>
                  <th>Số viên</th>
                  <th>Trạng thái</th>
                  <th>Hành động</th>
                </tr>";
        while ($row = $result->fetch_assoc()) {
            $statusText = $row['status'] ? "✅ Đã uống" : "❌ Chưa uống";
            echo "<tr>
                    <td>".sprintf("%02d", $row['hour'])."</td>
                    <td>".sprintf("%02d", $row['minute'])."</td>
                    <td>".$row['pills']."</td>
                    <td>$statusText</td>
                    <td><a class='delete' href='?delete=".$row['id']."' onclick='return confirm(\"Xóa lịch này?\")'>Xóa</a></td>
                  </tr>";
        }
        echo "</table>";
    } else {
        echo "<p><i>Chưa có lịch</i></p>";
    }
}
$conn->close();
?>
</body>
</html>
