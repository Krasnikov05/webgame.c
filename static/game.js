const canv = document.getElementById("canv");
const ctx = canv.getContext("2d");
const statusDiv = document.getElementById("status");
const params = new URLSearchParams(window.location.search);
const id = params.get("id");
let state = { active_player_index: 0, cells: [" ", " ", " ", " ", " ", " ", " ", " ", " "] };

function draw() {
  ctx.clearRect(0, 0, canv.width, canv.height);
  ctx.strokeStyle = "rgb(0, 0, 0)";
  ctx.lineWidth = 5.0;
  ctx.font = "48px sans-serif"
  ctx.textAlign = "center";
  ctx.textBaseline = "middle";
  const cellWidth = canv.width / 3;
  for (let i = 0; i < 9; i++) {
    const x = (i % 3);
    const y = Math.floor(i / 3);
    ctx.strokeRect(x * cellWidth, y * cellWidth, cellWidth, cellWidth);
    ctx.fillText(state.cells[i], (x + 0.5) * cellWidth, (y + 0.5) * cellWidth);
  }
}

draw();

async function update(cell) {
  console.log(cell);
  let path = "/gameRequest?id=" + id;
  if (cell != null) {
    path += "&cell=" + cell
  }
  const response = await fetch(path);
  if (!response.ok) {
    return;
  }
  const result = await response.json();
  console.log(result);
  if (result.state) {
    state = result.state;
  }
  statusDiv.innerText = result.status;
  draw();
}

setInterval(() => update(), 500);

canv.addEventListener("click", (e) => {
  const x = Math.floor(3 * (e.clientX - canv.getBoundingClientRect().left) / canv.width);
  const y = Math.floor(3 * (e.clientY - canv.getBoundingClientRect().top) / canv.height);
  const cell = x + 3 * y;
  update(cell);
});
