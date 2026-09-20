//EXPECTED: 13

int counter = 10;

int main() {
    for (int i = 0; i < 3; i += 1) {
        counter += 1;
    }

    return counter;
}
