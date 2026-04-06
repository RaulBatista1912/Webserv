/*
    ============================================================
    TETRIS - VERSION DEBUTANT (JS simple + commentaires)
    ============================================================

    Objectif:
    - Comprendre la logique du jeu pas a pas
    - Eviter les constructions JS trop avancees
    - Expliquer les fonctions "externes" du navigateur
      (addEventListener, setInterval, fetch, etc.)
*/

// Configuration de la grille
const COLS = 10;   // largeur en cases
const ROWS = 20;   // hauteur en cases
const SIZE = 20;   // taille d'une case en pixels

/*
    Fonctions externes (DOM):
    - document.getElementById("id") est une fonction du navigateur.
      Elle recupere un element HTML par son id.
*/

const canvas = document.getElementById("game");
const ctx = canvas.getContext("2d"); // API Canvas 2D du navigateur

const scoreEl = document.getElementById("score");
const linesEl = document.getElementById("lines");
const levelEl = document.getElementById("level");
const statusEl = document.getElementById("status");
const restartBtn = document.getElementById("restart");

// Couleurs associees aux formes
const COLORS = ["#58f1ff", "#ffe380", "#c88cff", "#ffb266", "#7fa7ff"];

/*
    Formes des pieces (matrices):
    - 1 = bloc present
    - 0 = vide
*/
const PIECES = [
    [[1, 1, 1, 1]],
    [[1, 1], [1, 1]],
    [[0, 1, 0], [1, 1, 1]],
    [[1, 0, 0], [1, 1, 1]],
    [[0, 0, 1], [1, 1, 1]]
];

// Etat du jeu
let board = [];            // grille complete
let piece = null;          // piece active
let score = 0;
let linesCleared = 0;
let level = 1;
let gameOverState = false;
let gameTimer = null;      // id retourne par setInterval
let playerName = "";

// Vitesse fixe (simple pour debutant)
const TICK_MS = 300;

function createEmptyBoard() {
    const grid = [];

    // Cree ROWS lignes, chaque ligne contient COLS zeros
    for (let y = 0; y < ROWS; y++) {
        const row = [];
        for (let x = 0; x < COLS; x++) {
            row.push(0);
        }
        grid.push(row);
    }

    return grid;
}

function randomPiece() {
    /*
        Fonction externe Math.random():
        - retourne un nombre entre 0 et 1 (ex: 0.438)
        - Math.floor() coupe la partie decimale
    */
    const id = Math.floor(Math.random() * PIECES.length);

    return {
        shape: PIECES[id],
        color: COLORS[id],
        x: 3,
        y: 0
    };
}

function rotate(matrix) {
    /*
        Rotation 90 degres vers la droite.
        Version "simple a lire" avec boucles classiques.
    */
    const oldHeight = matrix.length;
    const oldWidth = matrix[0].length;

    const rotated = [];

    for (let x = 0; x < oldWidth; x++) {
        const newRow = [];

        for (let y = oldHeight - 1; y >= 0; y--) {
            newRow.push(matrix[y][x]);
        }

        rotated.push(newRow);
    }
    return rotated;
}

function collide(boardRef, pieceRef) {
    // Verifie si la piece touche un mur, le bas ou un bloc deja fixe
    for (let dy = 0; dy < pieceRef.shape.length; dy++) {
        for (let dx = 0; dx < pieceRef.shape[dy].length; dx++) {
            const value = pieceRef.shape[dy][dx];

            if (value === 0)
                continue;

            const x = pieceRef.x + dx;
            const y = pieceRef.y + dy;

            // Hors limites gauche/droite
            if (x < 0 || x >= COLS)
                return true;

            // Hors limites bas
            if (y >= ROWS)
                return true;

            // Collision avec bloc deja place
            if (y >= 0 && boardRef[y][x])
                return true;
        }
    }

    return false;
}

function mergePieceIntoBoard() {
    // Copie la piece active dans la grille
    for (let dy = 0; dy < piece.shape.length; dy++) {
        for (let dx = 0; dx < piece.shape[dy].length; dx++) {
            if (piece.shape[dy][dx]) {
                board[piece.y + dy][piece.x + dx] = piece.color;
            }
        }
    }
}

function clearFullLines() {
    let removed = 0;

    // On parcourt de bas en haut
    for (let y = ROWS - 1; y >= 0; y--) {
        let full = true;

        // Verifie si toute la ligne est remplie
        for (let x = 0; x < COLS; x++) {
            if (board[y][x] === 0) {
                full = false;
                break;
            }
        }

        if (full) {
            // Supprime la ligne
            board.splice(y, 1);

            // Ajoute une ligne vide en haut
            const newRow = [];
            for (let i = 0; i < COLS; i++)
                newRow.push(0);
            board.unshift(newRow);

            removed++;

            // On re-check la meme position (car les lignes ont bouge)
            y++;
        }
    }

    return removed;
}

function drawBlock(x, y, color) {
    const px = x * SIZE;
    const py = y * SIZE;

    // Bloc principal
    ctx.fillStyle = color;
    ctx.fillRect(px, py, SIZE, SIZE);

    // Petit reflet
    ctx.fillStyle = "rgba(255, 255, 255, 0.24)";
    ctx.fillRect(px + 2, py + 2, SIZE - 4, 4);

    // Contour
    ctx.strokeStyle = "rgba(8, 14, 30, 0.65)";
    ctx.strokeRect(px + 0.5, py + 0.5, SIZE - 1, SIZE - 1);
}

function drawGridBackground() {
    // Fond
    ctx.fillStyle = "#0f1b2f";
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    // Lignes de la grille
    ctx.strokeStyle = "rgba(255,255,255,0.06)";

    for (let x = 0; x <= COLS; x++) {
        ctx.beginPath();
        ctx.moveTo(x * SIZE, 0);
        ctx.lineTo(x * SIZE, canvas.height);
        ctx.stroke();
    }

    for (let y = 0; y <= ROWS; y++) {
        ctx.beginPath();
        ctx.moveTo(0, y * SIZE);
        ctx.lineTo(canvas.width, y * SIZE);
        ctx.stroke();
    }
}

function drawBoard() {
    for (let y = 0; y < ROWS; y++) {
        for (let x = 0; x < COLS; x++) {
            if (board[y][x]) {
                drawBlock(x, y, board[y][x]);
            }
        }
    }
}

function drawCurrentPiece() {
    for (let dy = 0; dy < piece.shape.length; dy++) {
        for (let dx = 0; dx < piece.shape[dy].length; dx++) {
            if (piece.shape[dy][dx]) {
                drawBlock(piece.x + dx, piece.y + dy, piece.color);
            }
        }
    }
}

function draw() {
    drawGridBackground();
    drawBoard();
    drawCurrentPiece();
}

function updateHud() {
    if (scoreEl)
        scoreEl.textContent = String(score);

    // Ces elements peuvent ne pas exister selon ton HTML
    if (linesEl)
        linesEl.textContent = String(linesCleared);
    if (levelEl)
        levelEl.textContent = String(level);
}

function gameOver() {
    gameOverState = true;

    if (statusEl) {
        statusEl.textContent = "Game Over. Score envoye au CGI.";
        statusEl.style.color = "#ffb2a6";
    }

    sendScore(score);
}

function update() {
    // Ne rien faire si la partie est terminee
    if (gameOverState)
        return;

    // 1) La piece descend d'une case
    piece.y++;

    // 2) Si collision, on annule la descente et on fixe la piece
    if (collide(board, piece)) {
        piece.y--;
        mergePieceIntoBoard();

        // 3) Nettoyage des lignes
        const removed = clearFullLines();
        linesCleared += removed;

        // Score simple: 100 points par ligne
        score += removed * 100;

        // Niveau simple: +1 tous les 10 lignes
        level = Math.floor(linesCleared / 10) + 1;

        updateHud();

        // 4) Nouvelle piece
        piece = randomPiece();

        // 5) Si collision immediate -> game over
        if (collide(board, piece)) {
            gameOver();
        }
    }
}

/*
    Fonction externe addEventListener:
    - permet d'ecouter un evenement du navigateur
    - ici, evenement "keydown" = touche clavier appuyee
*/
document.addEventListener("keydown", function (e) {
    if (gameOverState)
        return;

    if (e.key === "ArrowLeft") {
        piece.x--;
        if (collide(board, piece))
            piece.x++;
    }

    if (e.key === "ArrowRight") {
        piece.x++;
        if (collide(board, piece))
            piece.x--;
    }

    if (e.key === "ArrowDown") {
        // chute acceleree (une etape)
        update();
    }

    if (e.key === "ArrowUp") {
        // On tente une rotation, puis rollback si invalide
        const oldShape = piece.shape;
        piece.shape = rotate(piece.shape);

        if (collide(board, piece))
            piece.shape = oldShape;
    }

    draw();
});

function startGameLoop() {
    /*
        Fonction externe setInterval:
        - appelle une fonction toutes les X millisecondes
        - ici, toutes les TICK_MS ms
    */
    if (gameTimer)
        clearInterval(gameTimer);

    gameTimer = setInterval(function () {
        update();
        draw();
    }, TICK_MS);
}

function resetGame() {
    board = createEmptyBoard();
    piece = randomPiece();
    score = 0;
    linesCleared = 0;
    level = 1;
    gameOverState = false;

    if (statusEl) {
        statusEl.textContent = "Joueur: " + playerName + " - Ready. Clear some lines.";
        statusEl.style.color = "#a8c3db";
    }

    updateHud();
    startGameLoop();
    draw();
}

// Clique sur le bouton Restart -> reset complet
if (restartBtn)
    restartBtn.addEventListener("click", resetGame);

function sendScore(finalScore) {
    /*
        Fonction externe fetch(url):
        - envoie une requete HTTP vers le serveur
        - ici on passe le score dans l'URL:
          /cgi-bin/save-score.cgi?score=123

        .then(...) = "quand la reponse arrive, fais ceci"
    */
    const body = "name=" + encodeURIComponent(playerName)
        + "&score=" + encodeURIComponent(String(finalScore));

    fetch("/cgi-bin/save-score.cgi", {
        method: "POST",
        headers: {
            "Content-Type": "application/x-www-form-urlencoded"
        },
        body: body
    })
        .then(function (res) {
            return res.text();
        })
        .then(function (text) {
            console.log("CGI response:", text);
        })
        .catch(function (err) {
            // En cas d'erreur reseau
            console.log("Erreur fetch score:", err);
        });
}

function askPlayerName() {
    /*
        Fonction externe prompt(message):
        - affiche une popup avec un champ texte
        - retourne ce que l'utilisateur tape (ou null si annule)
    */
    let firstTry = true;

    while (true) {
        const message = firstTry
            ? "Username:"
            : "Invalid username :( retry:";
        const input = prompt(message);

        // Si annulation, on prend une valeur par defaut au lieu de boucler a l'infini
        if (input === null) {
            firstTry = false;
            continue;
        }

        const trimmed = input.trim();

        // Nom vide => invalide
        if (trimmed.length === 0) {
            firstTry = false;
            continue;
        }

        // Premier caractere numerique => invalide
        const firstChar = trimmed[0];
        if (firstChar >= '0' && firstChar <= '9' || firstChar == '_'
            || firstChar == '/' || firstChar == '$') {
            firstTry = false;
            continue;
        }

        return trimmed;
    }
}

// Demarre une partie au chargement de la page
playerName = askPlayerName();
resetGame();
