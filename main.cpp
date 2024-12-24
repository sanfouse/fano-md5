#include <iostream>
#include "FanoCoder.h"

using namespace std;

int main() {
    FanoCoder coder;
    cout << "Выберите режим (1 - Архивация, 2 - Разархивация): ";
    int mode;
    cin >> mode;

    if (mode == 1) {
        string inputFilename, outputFilename;
        cout << "Введите имя исходного файла: ";
        cin >> inputFilename;
        cout << "Введите имя выходного файла: ";
        cin >> outputFilename;
        coder.compress(inputFilename, outputFilename);

    } else if (mode == 2) {
        string inputFilename, outputFilename;
        cout << "Введите имя архивного файла: ";
        cin >> inputFilename;
        cout << "Введите имя выходного файла: ";
        cin >> outputFilename;
        coder.decompress(inputFilename, outputFilename);

    } else {
        cout << "Неверный режим." << endl;
    }

    return 0;
}
