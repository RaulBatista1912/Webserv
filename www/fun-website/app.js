// Fade in de la card au chargement
document.addEventListener("DOMContentLoaded", function () {

    // Bouton "dede" → effet confetti simple
    var dede = document.querySelector(".extra-actions .secondary");
    if (dede) {
        dede.addEventListener("click", function () {
            spawnConfetti();
        });
    }

    var sortBtn = document.getElementById("sort-btn");
    var sortInput = document.getElementById("sort-input");
    var sortResult = document.getElementById("sort-result");

    if (sortBtn && sortInput && sortResult) {
        function runSort() {
            var raw = sortInput.value.trim();
            if (!raw) {
                sortResult.textContent = "Enter a comma-separated list like 2,7,2,6,8,1.";
                return;
            }

            sortResult.textContent = "Sorting...";
            fetch("/sort/sort.cgi?numbers=" + encodeURIComponent(raw), {
                method: "GET"
            })
                .then(function (res) {
                    return res.text();
                })
                .then(function (text) {
                    sortResult.textContent = text;
                })
                .catch(function () {
                    sortResult.textContent = "Sort CGI request failed.";
                });
        }

        sortBtn.addEventListener("click", runSort);
        sortInput.addEventListener("keydown", function (event) {
            if (event.key === "Enter")
                runSort();
        });
    }

});


function spawnConfetti() {
    var colors = ["#3dd8ff", "#ff6b6b", "#ffe66d", "#a8ff78", "#f8f8f8"];
    for (var i = 0; i < 200; i++) {
        (function (i) {
            var dot = document.createElement("div");
            dot.style.cssText = [
                "position:fixed",
                "width:8px", "height:8px",
                "border-radius:50%",
                "background:" + colors[i % colors.length],
                "left:" + (Math.random() * 100) + "%",
                "top:" + (Math.random() * 100) + "%",
                "pointer-events:none",
                "z-index:9999",
                "transition:transform 0.7s ease, opacity 0.7s ease"
            ].join(";");
            document.body.appendChild(dot);
            setTimeout(function () {
                dot.style.transform = "translate(" + (Math.random() * 200 - 100) + "px, " + (Math.random() * 200 - 100) + "px)";
                dot.style.opacity = "0";
            }, 20);
            setTimeout(function () { document.body.removeChild(dot); }, 800);
        })(i);
    }
}