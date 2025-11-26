const COLORS = {
  cross: "#E94138",
  circle: "#FFB520",
  outline: "#B8B8D1",
};
const ANIMATION_STEP = 0.003;
const canv = document.getElementById("canv");
const ctx = canv.getContext("2d");
const statusDiv = document.getElementById("status");
const modalExit = document.getElementById("modal-exit");
const exitButton = document.getElementById("exit-button");
const splashElem = document.getElementById("splash");
const playersElem = document.getElementById("players");
const spinner = document.getElementById("spinner");
const params = new URLSearchParams(window.location.search);
const id = params.get("id");
const gameType = params.get("gameType");
let state = { active_player_index: 0, cells: Array.from({ length: 9 }).map((_) => " ") };

function showThisGuide() {
  showModalGuide("/static/" + gameType + "-guide.html");
}

async function disconnectFollowLink(url) {
  if (url == undefined) {
    url = "/"
  }
  modalBg.classList.remove("hidden");
  modalExit.classList.remove("hidden");
  setTimeout(
    () => {
      modalBg.classList.remove("modal-hidden");
      modalExit.classList.remove("modal-hidden");
    },
    20
  );
  exitButton.onclick = () => {
    fetch("/disconnect?id=" + id).then(() =>
      window.open(url + "?id=" + id, "_self")
    )
  };
}

function easingFunction(x) {
  if (x > 1) {
    return 1;
  }
  if (x < 0) {
    return 0;
  }
  return (1 - Math.cos(Math.PI * x)) / 2;
}

class Sprite {
  constructor(type) {
    this.type = type;
    this.progress = 0;
  }

  update(deltaTime) {
    this.progress += ANIMATION_STEP * deltaTime;
  }
}

class Drawer {
  constructor(canv, ctx, statusDiv, splashElem, playersElem, spinner) {
    this.canv = canv;
    this.ctx = ctx;
    this.statusDiv = statusDiv;
    this.splashElem = splashElem;
    this.playersElem = playersElem;
    this.spinner = spinner;
    this.playerIndex = -1;
    this.cells = Array.from({ length: 9 }).map((_) => " ");
    this.sprites = Array.from({ length: 9 }).map((_) => null);
    this.lastTime = null;
    this.finished = false;
  }

  splash(text) {
    this.splashElem.innerText = text;
    this.splashElem.classList.remove("hidden");
    const keyframes = [
      { transform: 'scale(0)' },
      { transform: 'scale(1)' },
    ];
    const options = {
      duration: 300,
      easing: "cubic-bezier(0.68, -0.55, 0.265, 1.55)",
      iterations: 1,
    };
    this.splashElem.animate(keyframes, options);
  }

  update(response) {
    if (this.finished) {
      return;
    }
    this.playersElem.innerHTML = response.player_names.map((x) => {
      if (x == null) {
        return "<div class='unknown'>???</div>";
      }
      return "<div>" + x + "</div>";
    }).join("");
    const state = response.state;
    this.playerIndex = response.your_index;
    if (response.status == "game_active") {
      this.spinner.classList.add("hidden");
      this.statusDiv.innerText = state.active_player_index == this.playerIndex ? "Your turn" : "";
    }
    if (response.status == "game_finished") {
      this.finished = true;
      setTimeout(() => {
        window.open("/?id=" + id, "_self")
      }, 2000);
      if (state.winner_index == this.playerIndex) {
        this.splash("You win!");
      } else if (state.winner_index == 1 - this.playerIndex) {
        this.splash("You lost!");
      } else {
        this.splash("Draw!");
      }
    }
    const cells = state.cells;
    for (let i = 0; i < 9; i++) {
      if (cells[i] != this.cells[i]) {
        this.sprites[i] = new Sprite(cells[i]);
        this.cells[i] = cells[i];
      }
    }
  }

  drawGrid(n) {
    const cellWidth = this.canv.width / n;
    this.ctx.strokeStyle = COLORS.outline;
    this.ctx.lineCap = "round";
    this.ctx.lineWidth = 5.0;
    for (let i = 1; i < n; i++) {
      this.ctx.beginPath();
      this.ctx.moveTo(i * cellWidth, 5);
      this.ctx.lineTo(i * cellWidth, n * cellWidth - 5);
      this.ctx.stroke();
      this.ctx.beginPath();
      this.ctx.moveTo(5, i * cellWidth);
      this.ctx.lineTo(n * cellWidth - 5, i * cellWidth);
      this.ctx.stroke();
    }
  }

  drawSprite(sprite, x, y, size) {
    if (sprite == null) {
      return;
    }
    size *= 0.2;
    this.ctx.save();
    this.ctx.translate(x, y);
    this.ctx.lineCap = "round";
    this.ctx.lineWidth = 5.0;
    if (sprite.type == "X") {
      const p1 = easingFunction(sprite.progress / 0.8);
      if (p1 > 0) {
        this.ctx.strokeStyle = COLORS.cross;
        this.ctx.beginPath();
        this.ctx.moveTo(-size, -size);
        this.ctx.lineTo(-size + 2 * p1 * size, -size + 2 * p1 * size);
        this.ctx.stroke();
      }
      const p2 = easingFunction((sprite.progress - 0.2) / 0.8);
      if (p2 > 0) {
        this.ctx.beginPath();
        this.ctx.moveTo(size, -size);
        this.ctx.lineTo(size - 2 * p2 * size, -size + 2 * p2 * size);
        this.ctx.stroke();
      }
    } else if (sprite.type == "O") {
      this.ctx.strokeStyle = COLORS.circle;
      this.ctx.rotate(-1);
      this.ctx.beginPath();
      ctx.arc(0, 0, size, 0, easingFunction(sprite.progress) * Math.PI * 2);
      this.ctx.stroke();
    }
    this.ctx.restore();
  }

  draw(time) {
    let deltaTime = null;
    if (this.lastTime == null) {
      deltaTime = 0;
    } else {
      deltaTime = time - this.lastTime;
    }
    this.lastTime = time;
    this.ctx.clearRect(0, 0, this.canv.width, this.canv.height);
    this.drawGrid(3);
    const cellWidth = this.canv.width / 3;
    for (let i = 0; i < 9; i++) {
      const x = (i % 3);
      const y = Math.floor(i / 3);
      this.drawSprite(this.sprites[i], (x + 0.5) * cellWidth, (y + 0.5) * cellWidth, cellWidth);
      if (this.sprites[i] != null) {
        this.sprites[i].update(deltaTime);
      }
    }
  }
}

const drawer = new Drawer(canv, ctx, statusDiv, splashElem, playersElem, spinner);

function draw(time) {
  drawer.draw(time);
  requestAnimationFrame(draw);
}

draw(0);

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
  // statusDiv.innerText = result.status;
  drawer.update(result);
}

update();
setInterval(() => update(), 500);

canv.addEventListener("click", (e) => {
  const x = Math.floor(3 * (e.clientX - canv.getBoundingClientRect().left) / canv.width);
  const y = Math.floor(3 * (e.clientY - canv.getBoundingClientRect().top) / canv.height);
  const cell = x + 3 * y;
  update(cell);
});
