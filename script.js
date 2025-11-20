// Tạo giao diện cho 6 ngăn
document.addEventListener("DOMContentLoaded", () => {
  const container = document.getElementById("container");

  for (let i = 1; i <= 6; i++) {
    const div = document.createElement("div");
    div.className = "compartment";
    div.innerHTML = `
      <h2>Ngăn ${i}</h2>
      <form onsubmit="addSchedule(event, ${i})">
        Giờ: <input type="number" name="hour" min="0" max="23" required>
        Phút: <input type="number" name="minute" min="0" max="59" required>
        Số viên: <input type="number" name="pills" min="1" required>
        <button type="submit">Thêm</button>
      </form>
      <table id="table-${i}">
        <thead>
          <tr>
            <th>Giờ</th>
            <th>Phút</th>
            <th>Số viên</th>
            <th>Trạng thái</th>
            <th>Xóa</th>
          </tr>
        </thead>
        <tbody></tbody>
      </table>
    `;
    container.appendChild(div);
  }

  loadData();
});

// Hàm load dữ liệu từ data.php
// Hàm load dữ liệu từ data.php
function loadData() {
  fetch("data.php")
    .then(res => res.json())
    .then(data => {
      // Xóa bảng cũ
      for (let i = 1; i <= 6; i++) {
        document.querySelector(`#table-${i} tbody`).innerHTML = "";
      }

      data.forEach(item => {
        const tbody = document.querySelector(`#table-${item.compartment} tbody`);
        if (tbody) {
          const tr = document.createElement("tr");
          tr.innerHTML = `
            <td>${item.hour}</td>
            <td>${item.minute}</td>
            <td>${item.pills}</td>
            <td>${item.status == 1 ? "Đã uống" : "Chưa uống"}</td>
            <td><button class="delete-btn" onclick="deleteSchedule(${item.id})">Xóa</button></td>
          `;
          tbody.appendChild(tr);
        }
      });
    });
}


// Hàm thêm lịch
function addSchedule(event, compartment) {
  event.preventDefault();
  const form = event.target;
  const hour = form.hour.value;
  const minute = form.minute.value;
  const pills = form.pills.value;

  fetch(`add.php?compartment=${compartment}&hour=${hour}&minute=${minute}&pills=${pills}&status=0`)
    .then(res => res.json())
    .then(() => loadData());

  form.reset();
}

// Hàm xóa lịch
function deleteSchedule(id) {
  if (confirm("Bạn có chắc chắn muốn xóa lịch này?")) {
    fetch(`delete.php?id=${id}`)
      .then(res => res.json())
      .then(() => loadData());
  }
}
