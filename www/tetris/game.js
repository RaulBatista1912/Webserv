const COLS = 10;
const ROWS = 20;
const SIZE = 20;

const canvas = document.getElementById("game");
const ctx = canvas.getContext("2d");

let board = Array.from({ length: ROWS }, () => Array(COLS).fill(0));

let piece = { x: 4, y: 0 };

function draw() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    ctx.fillStyle = "red";
    ctx.fillRect(piece.x * SIZE, piece.y * SIZE, SIZE, SIZE);
}

function update() {
    piece.y++;
}

document.addEventListener("keydown", e => {
    if (e.key === "ArrowLeft") piece.x--;
    if (e.key === "ArrowRight") piece.x++;
    if (e.key === "ArrowDown") piece.y++;
});

setInterval(() => {
    update();
    draw();
}, 500);