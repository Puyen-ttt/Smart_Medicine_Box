<?php
header('Content-Type: application/json');

$conn = new mysqli("localhost", "root", "", "smartbox");

if ($conn->connect_error) {
    echo json_encode([
        'error' => 'Kết nối thất bại: ' . $conn->connect_error
    ]);
    exit;
}

// Truy vấn tất cả dữ liệu, sắp xếp theo ngăn, giờ, phút
$sql = "
    SELECT 
        id, 
        compartment, 
        hour, 
        minute, 
        pills, 
        status 
    FROM pill_schedule2
    ORDER BY compartment, hour, minute
";

$result = $conn->query($sql);

$data = [];

if ($result->num_rows > 0) {
    while ($row = $result->fetch_assoc()) {
        $data[] = [
            'id'         => (int)$row['id'],
            'compartment'=> (int)$row['compartment'],
            'hour'       => (int)$row['hour'],
            'minute'     => (int)$row['minute'],
            'pills'      => (int)$row['pills'],
            'status'     => (int)$row['status']
        ];
    }
}

// Xuất JSON
echo json_encode($data, JSON_PRETTY_PRINT);

$conn->close();
?>
