const canvas = document.getElementById("background-canv");
const ctx = canvas.getContext("2d");

const CONFIG = {
  particleCount: 50,
  minSize: 8,
  maxSize: 32,
  minSpeed: 0.01,
  maxSpeed: 0.15,
  colors: [
    "#E94138", "#FFB520", "#FFFFFB"
  ],
  lineWidth: 4,
  background: "#424CD4",
};

function rand(min, max) {
  return Math.random() * (max - min) + min;
}
function randInt(min, max) {
  return Math.floor(Math.random() * (max - min + 1)) + min;
}
class Particle {
  constructor(width, height) {
    this.reset(width, height, true);
  }
  reset(width, height, init = false) {
    this.type = Math.random() < 0.5 ? "cross" : "circle";
    this.radius = rand(CONFIG.minSize, CONFIG.maxSize);
    this.x = rand(0, width);
    this.y = rand(0, height);
    const speed = rand(CONFIG.minSpeed, CONFIG.maxSpeed);
    const angle = rand(0, Math.PI * 2);
    this.vx = Math.cos(angle) * speed;
    this.vy = Math.sin(angle) * speed;
    this.color = CONFIG.colors[randInt(0, CONFIG.colors.length - 1)];
    this.rotation = rand(0, Math.PI * 2);
    this.rotationSpeed = rand(-0.003, 0.003);
  }
  update(width, height) {
    this.x += this.vx;
    this.y += this.vy;
    this.rotation += this.rotationSpeed;
    if (this.x < -2 * this.radius) {
      this.x = width + 2 * this.radius;
    }
    if (this.x > width + 2 * this.radius) {
      this.x = -2 * this.radius;
    }
    if (this.y < -2 * this.radius) {
      this.y = height + 2 * this.radius;
    }
    if (this.y > height + 2 * this.radius) {
      this.y = -2 * this.radius;
    }
  }
  draw(ctx) {
    ctx.save();
    ctx.lineWidth = Math.max(1, CONFIG.lineWidth * (this.radius / 8));
    if (this.type === "circle") {
      ctx.beginPath();
      ctx.strokeStyle = this.color;
      ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
      ctx.stroke();
    } else {
      ctx.translate(this.x, this.y);
      ctx.rotate(this.rotation);
      ctx.lineCap = "round";
      ctx.strokeStyle = this.color;
      const r = this.radius;
      ctx.beginPath();
      ctx.moveTo(-r, 0);
      ctx.lineTo(r, 0);
      ctx.moveTo(0, -r);
      ctx.lineTo(0, r);
      ctx.stroke();
    }
    ctx.restore();
  }
}
class ParticleSystem {
  constructor(canvas, ctx, count) {
    this.canvas = canvas;
    this.ctx = ctx;
    this.width = window.innerWidth;
    this.height = window.innerHeight;
    this.particles = [];
    this.count = count;
    this._createParticles();
  }
  _createParticles() {
    this.particles.length = 0;
    for (let i = 0; i < this.count; i++) {
      this.particles.push(new Particle(this.width, this.height));
    }
  }
  resize(width, height) {
    this.width = width;
    this.height = height;
    for (const p of this.particles) {
      p.x = Math.min(Math.max(p.x, -p.radius), width + p.radius);
      p.y = Math.min(Math.max(p.y, -p.radius), height + p.radius);
    }
  }
  updateAndDraw() {
    this.ctx.fillStyle = CONFIG.background;
    this.ctx.fillRect(0, 0, this.width, this.height);
    for (const p of this.particles) {
      p.update(this.width, this.height);
      p.draw(this.ctx);
    }
  }
}
const system = new ParticleSystem(canvas, ctx, CONFIG.particleCount);
function animate() {
  system.updateAndDraw();
  requestAnimationFrame(animate);
}
function setCanvasSizeToDisplaySize() {
  const w = window.innerWidth;
  const h = window.innerHeight;
  if (canvas.width !== w || canvas.height !== h) {
    canvas.width = w;
    canvas.height = h;
    system.resize(w, h);
  }
}
window.addEventListener("resize", setCanvasSizeToDisplaySize);
setCanvasSizeToDisplaySize();
animate();
