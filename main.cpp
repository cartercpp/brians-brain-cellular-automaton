#include <iostream>
#include <format>
#include <vector>
#include <utility>
#include <chrono>
#include <random>
#include <thread>
#include <stop_token>

enum class CELL{RESTING, FIRING, RECOVERING};

int main()
{
    constexpr int rows = 88,
                  columns = 100;

    std::vector<std::vector<CELL>> grid(rows, std::vector<CELL>(columns, CELL::RESTING));

    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 2);

    for (int row = 0; row < rows; ++row)
        for (int column = 0; column < columns; ++column)
            grid[row][column] = static_cast<CELL>(dist(rd));

    auto moveCursor = [](int row, int column) {
        std::cout << std::format("\033[{};{}H", row + 1, column + 1);
    };

    auto setColor = [](int red, int green, int blue) {
        std::cout << std::format("\033[38;2;{};{};{}m", red, green, blue);
    };

    std::cout << "\033[?25l";

    std::jthread render{[&](std::stop_token st) {
        std::vector<std::vector<CELL>> newGrid{grid};

        while (!st.stop_requested())
        {
            for (int row = 0; row < rows; ++row)
            {
                moveCursor(row, 0);

                for (int column = 0; column < columns; ++column)
                {
                    switch (grid[row][column])
                    {
                        case CELL::RESTING:
                        {
                            setColor(0, 0, 0);
                            std::cout << ' ';

                            int firingNeighbors = 0;
                            for (int i = row - 1; i <= row + 1; ++i)
                                for (int i2 = column - 1; i2 <= column + 1; ++i2)
                                {
                                    if ((i == row) && (i2 == column))
                                        continue;

                                    int neighborRow = i,
                                        neighborColumn = i2;

                                    if (i < 0)
                                        neighborRow = rows + i;
                                    else if (i >= rows)
                                        neighborRow = i % rows;

                                    if (i2 < 0)
                                        neighborColumn = columns + i2;
                                    else if (i2 >= columns)
                                        neighborColumn = i2 % columns;

                                    if (grid[neighborRow][neighborColumn] == CELL::FIRING)
                                        ++firingNeighbors;
                                }

                            newGrid[row][column] = (firingNeighbors == 2) ? CELL::FIRING : CELL::RESTING;

                            break;
                        }
                        case CELL::FIRING:
                            setColor(255, 255, 255);
                            std::cout << '*';
                            newGrid[row][column] = CELL::RECOVERING;
                            break;
                        case CELL::RECOVERING:
                            setColor(255, 0, 0);
                            std::cout << '.';
                            newGrid[row][column] = CELL::RESTING;
                            break;
                    }

                }
            }

            std::swap(grid, newGrid);
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    }};
    std::cin.get();
}