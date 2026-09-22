const canvas = document.getElementById("grid");
const ctx = canvas.getContext("2d");
const stepButton = document.getElementById("step");
const runButton = document.getElementById("run");
const clearButton = document.getElementById("clear");

const WIDTH = 48;
const HEIGHT = 27;
const CELL_SIZE = 18;

canvas.width = WIDTH * CELL_SIZE;
canvas.height = HEIGHT * CELL_SIZE;

let wasm;
let grid;

async function main() {
    try {
        const result = await WebAssembly.instantiateStreaming(
            fetch("output1.wasm")
        );

        wasm = result.instance.exports;

        const gridPtr = wasm.get_grid();

        grid = new Int32Array(
            wasm.memory.buffer,
            gridPtr,
            WIDTH * HEIGHT
        );

        drawGrid();
    } catch (error) {
        console.error(error);
        status.textContent =
            "Failed to load output_1.wasm. Make sure it is in this directory and served over HTTP.";
    }
}

function drawGrid() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    for (let row = 0; row < HEIGHT; row++) {
        for (let col = 0; col < WIDTH; col++) {
            const index = row * WIDTH + col;

            if (grid[index] === 1) {
                ctx.fillRect(
                    col * CELL_SIZE,
                    row * CELL_SIZE,
                    CELL_SIZE,
                    CELL_SIZE
                );
            }
        }
    }

    ctx.beginPath();

    for (let i = 0; i <= WIDTH; i++) {
        const p = i * CELL_SIZE;

        ctx.moveTo(p, 0);
        ctx.lineTo(p, canvas.height);
    }

    for (let i = 0; i <= HEIGHT; i++) {
        const p = i * CELL_SIZE;

        ctx.moveTo(0, p);
        ctx.lineTo(canvas.width, p);
    }

    ctx.stroke();
}

canvas.addEventListener("click", (event) => {
    if (!wasm) return;

    const rect = canvas.getBoundingClientRect();

    const col = Math.floor(
        (event.clientX - rect.left) / CELL_SIZE
    );

    const row = Math.floor(
        (event.clientY - rect.top) / CELL_SIZE
    );

    if (row < 0 || row >= HEIGHT || col < 0 || col >= WIDTH) {
        return;
    }

    const index = row * WIDTH + col;

    // Toggle the cell.
    const alive = grid[index] === 1 ? 0 : 1;

    wasm.set_cell(row, col, alive);

    drawGrid();
});

stepButton.addEventListener("click", () => {
    if (!wasm) return;

    wasm.step();
    drawGrid();

    clearInterval(intervalId);
    intervalId = null;
});

clearButton.addEventListener("click", () => {
    if (!wasm) return;

    for (let row = 0; row < HEIGHT; row++) {
        for (let col = 0; col < WIDTH; col++) {
            wasm.set_cell(row, col, 0);
        }
    }

    drawGrid();
    
    clearInterval(intervalId);
    intervalId = null;
});

let intervalId = null;

runButton.addEventListener("click", () => {
    if (!wasm) return;

    if (intervalId !== null) {
        clearInterval(intervalId);
        intervalId = null;
        return;
    }

    intervalId = setInterval(() => {
        wasm.step();
        drawGrid();
    }, 200); 
});

main();
