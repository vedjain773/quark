int next_grid[1296];
int grid[1296];

int ROWS = 27;
int COLS = 48;

void set_cell(int row, int col, int alive) {
    grid[row * COLS + col] = alive;
}

void set_next(int row, int col, int alive) {
    next_grid[row * COLS + col] = alive;
}

int get_alive_neighs(int row, int col) {
    int alive = 0;
    
    for (int i = -1; i < 2; i += 1) {
        for (int j = -1; j < 2; j += 1) {
            if (i == 0 && j == 0) continue;

            int nx = col + i;
            int ny = row + j;

            if (nx < 0 || nx > COLS - 1) continue;
            if (ny < 0 || ny > ROWS - 1) continue;

            if (grid[ny * COLS + nx] == 1) alive += 1; 
        }
    }

    return alive;
}

void step() {
    int i;
    int j;

    for (i = 0; i < ROWS; i += 1) {
        for (j = 0; j < COLS; j += 1) {
            int alive = get_alive_neighs(i, j);
            int is_alive = grid[i * COLS + j];

            if (is_alive) {
                if (alive <= 1 || alive >= 4) set_next(i, j, 0);
                else set_next(i, j, 1);
            } else {
                if (alive == 3) set_next(i, j, 1);
                else set_next(i, j, 0);
            }
        }
    }

    i = 0; 
    j = 0;

    for (int i = 0; i < ROWS; i += 1) {
        for (int j = 0; j < COLS; j += 1) {
            set_cell(i, j, next_grid[i * COLS + j]);
        }
    }
}

int *get_grid() {
    return &grid[0];
}
