#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <iomanip>
#include <conio.h>// Для использования _kbhit() и _getch() на Windows
#include <Windows.h>
#include "SDL.h"

#define _CRT_SECURE_NO_WARNINGS_

const int size = 20; // Размер игрового поля
const int FRAME_RATE = 20;
const int CELL_SIZE = 10;

// Функция для инициализации игрового поля из файла
void initializeBoardFromFile(std::vector<std::vector<char>>& board, const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        board.clear();
        std::string line;
        while (std::getline(file, line) && board.size() < size) {
            std::vector<char> row;
            for (char ch : line) {
                if (ch == '0' || ch == '*') {

                    row.push_back(ch);
                }
            }
            if (!row.empty()) {
                board.push_back(row);
            }
        }
        file.close();
    }
    else {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}

// Функция для записи данных в HTML-файл
void writeToHTML(int generation, int bornCells, const std::vector<std::vector<char>>& board, const std::string& filename) {
    std::ofstream htmlFile(filename, std::ios::app);
    if (htmlFile.is_open()) {
        htmlFile << "<html><head><title>Game of Life</title></head><body>" << std::endl;
        htmlFile << "<h2>Поколение: " << generation << "</h2>" << std::endl;
        htmlFile << "<p>Число рождённых клеток: " << bornCells << "</p>" << std::endl;
        htmlFile << "<pre>";
        for (const auto& row : board) {
            for (char cell : row) {
                htmlFile << cell << ' ';
            }
            htmlFile << "<br>";
        }
        htmlFile << "</pre><hr>" << std::endl;
        htmlFile << "</body></html>" << std::endl;
        htmlFile.close();
    }
    else {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}

// Функция для инициализации игрового поля случайными значениями (живые и мёртвые клетки)
void initializeBoard(std::vector<std::vector<char>>& board) {
    srand(static_cast<unsigned int>(time(0)));
    for (int i = 0; i < size; ++i) {
        std::vector<char> row;
        for (int j = 0; j < size; ++j) {
            row.push_back((rand() % 2 == 0) ? '0' : '*'); // '0' или '*' с вероятностью 50%
        }
        board.push_back(row);
    }
}



// Функция для отображения текущего состояния игрового поля
void displayBoard(const std::vector<std::vector<char>>& board) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            std::cout << board[i][j] << ' ';
        }
        std::cout << std::endl;
    }
}


int countNeighbors(const std::vector<std::vector<char>>& board, int row, int col) {
    int size = board.size();
    int count = 0;

    for (int i = row - 1; i <= row + 1; ++i) {
        for (int j = col - 1; j <= col + 1; ++j) {
            // Зацикливание координат для бесконечного поля
            int x = (i + size) % size;
            int y = (j + size) % size;

            if (!(i == row && j == col)) {
                count += (board[x][y] == '0' ? 1 : 0);
            }
        }
    }

    return count;
}


// Функция для обновления состояния игрового поля в соответствии с правилами "Игры жизни"
void updateBoard(std::vector<std::vector<char>>& board) {
    std::vector<std::vector<char>> newBoard = board;

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int neighbors = countNeighbors(board, i, j);

            if (board[i][j] == '0') { // Живая клетка
                newBoard[i][j] = (neighbors == 2 || neighbors == 3) ? '0' : '*';
            }
            else { // Мёртвая клетка
                newBoard[i][j] = (neighbors == 3) ? '0' : '*';
            }
        }
    }

    board = newBoard;
}
 // Размер клетки

void drawBoard(SDL_Renderer* renderer, const std::vector<std::vector<char>>& board) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            SDL_Rect cellRect = { j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE };

            if (board[i][j] == '0') {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }

            SDL_RenderFillRect(renderer, &cellRect);
        }
    }

    SDL_RenderPresent(renderer);
}
// Основная функция
void smoothUpdate(std::vector<std::vector<char>>& board, int& generation,std::string outputFilename) {
    char key;
    while (true) {

        std::cout << "Нажмите любую клавишу, кроме 'Escape', для движения вперед. Нажмите 'Escape', чтобы выйти." << std::endl;

        if (_kbhit()) {
            key = _getch();
            if (key == 27) { // 27 - ASCII код клавиши 'Escape'
                break;
            }
        }


        std::cout << "Текущее состояние поля:" << std::endl;
        displayBoard(board);
        std::cout << std::endl;

        updateBoard(board);

        // Подсчет числа рождённых клеток
        int bornCells = 0;
        for (const auto& row : board) {
            for (char cell : row) {
                bornCells += (cell == '0') ? 1 : 0;
            }
        }

        // Запись в HTML-файл
        writeToHTML(generation, bornCells, board, outputFilename);

        // Пауза для достижения 60 Гц
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FRAME_RATE));

        system("cls");

        generation++;
    }
}

// Основная функция
int main(int argc, char* argv[]) {

    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_filename> <output_filename>" << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::vector<char>> board;
    initializeBoardFromFile(board, argv[1]);
    //initializeBoard(board);
    char key;
    int generation = 0;

    std::cout << "Выберите режим:" << std::endl;
    std::cout << "1. Плавная игра" << std::endl;
    std::cout << "2. Ручное управление" << std::endl;

    int mode;
    std::cin >> mode;

    std::string outputFilename = argv[2];

    if (mode == 1) {
        SDL_Init(SDL_INIT_VIDEO);

        SDL_Window* window = SDL_CreateWindow("Game of Life", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            size * CELL_SIZE, size * CELL_SIZE, SDL_WINDOW_SHOWN);
        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        /* smoothUpdate(board, generation, outputFilename);*/
        while (true) {
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    return 0;
                }
            }

            // Отображение текущего состояния поля
            drawBoard(renderer, board);
            int bornCells = 0;
            for (const auto& row : board) {
                for (char cell : row) {
                    bornCells += (cell == '0') ? 1 : 0;
                }
            }
            writeToHTML(generation, bornCells, board, outputFilename);
            // Обновление состояния поля
            updateBoard(board);

            // Пауза для достижения 60 Гц
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FRAME_RATE));
            generation++;
        }
    }
    else if (mode == 2) {
        while (true) {



            std::cout << "Текущее состояние поля:" << std::endl;
            displayBoard(board);
            std::cout << std::endl;

            updateBoard(board);

            // Подсчет числа рождённых клеток
            int bornCells = 0;
            for (const auto& row : board) {
                for (char cell : row) {
                    bornCells += (cell == '0') ? 1 : 0;
                }
            }

            // Запись в HTML-файл
            writeToHTML(generation, bornCells, board, outputFilename);
            std::cout << "Нажмите любую клавишу, кроме 'Escape', для движения вперед. Нажмите 'Escape', чтобы выйти." << std::endl;


            key = _getch();
            if (key == 27) { // 27 - ASCII код клавиши 'Escape'
                break;
            }

            system("cls");

            generation++;
        }
    }
    return 0;
}