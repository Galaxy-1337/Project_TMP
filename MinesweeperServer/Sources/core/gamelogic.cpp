#include "Headers/core/gamelogic.h"
#include <QRandomGenerator>
#include <QQueue>
#include <QDebug>

GameLogic::GameLogic(QObject *parent) : QObject(parent)
{
    resetGame();
}

void GameLogic::initializeGame(int rows, int cols, int minesCount)
{
    m_rows = rows;
    m_cols = cols;
    m_minesCount = minesCount;
    m_remainingMines = minesCount;
    m_gameOver = false;
    m_gameWon = false;
    m_firstClick = true;
    m_scoutScore = 0;
    m_saboteurScore = 0;
    m_currentPlayer = Scout;

    // Инициализация пустого поля
    m_gameField.resize(m_rows);
    for (int i = 0; i < m_rows; ++i) {
        m_gameField[i].resize(m_cols);
        for (int j = 0; j < m_cols; ++j) {
            m_gameField[i][j] = Cell();
        }
    }

    emit gameFieldChanged();
    emit playerSwitched(m_currentPlayer);
}

void GameLogic::resetGame()
{
    initializeGame(m_rows, m_cols, m_minesCount);
}

bool GameLogic::openCell(int row, int col, PlayerRole player)
{
    qDebug() << "GameLogic::openCell called for (" << row << "," << col << ") by player" << player << ". Current player:" << m_currentPlayer;

    if (m_gameOver) {
        qDebug() << "openCell: Game is over.";
        return false;
    }
    if (!isValidCell(row, col)) {
        qDebug() << "openCell: Invalid cell coordinates.";
        return false;
    }
    if (player != m_currentPlayer) {
        qDebug() << "openCell: Not current player's turn. Expected:" << m_currentPlayer << ", Got:" << player;
        return false;
    }

    Cell &cell = m_gameField[row][col];

    // --- ЛОГИКА ПЕРВОГО ХОДА ---
    if (m_firstClick) {
        qDebug() << "openCell: First click detected. Generating mines.";
        generateMines(row, col); // Мины генерируются, избегая 3x3 вокруг (row, col)
        calculateMinesAround();  // Подсчитываем мины вокруг
        m_firstClick = false;

        // Теперь, когда мины и цифры расставлены, обрабатываем первый клик
        if (!cell.hasMine && cell.minesAround == 0) {
            // Если первая клетка пустая (0 мин вокруг), раскрываем всю область
            qDebug() << "openCell: First click on empty cell (0 mines around). Revealing area.";
            // m_scoutScore будет обновлен внутри revealEmptyArea
            revealEmptyArea(row, col);
        } else if (!cell.hasMine && cell.minesAround > 0) {
            // Если первая клетка с цифрой (не мина), просто открываем ее
            qDebug() << "openCell: First click on a numbered cell. Opening only this cell.";
            cell.state = Opened;
            m_scoutScore += 1; // Начисляем очки за эту одну открытую клетку
        } else {
            // Этого не должно произойти, если generateMines работает корректно
            qWarning() << "openCell: First click landed on a mine despite safe zone logic!";
            // В случае ошибки, можно обработать как поражение или просто открыть
            cell.state = Opened; // Открываем, чтобы показать ошибку
            m_gameOver = true;
            m_gameWon = false;
            emit gameOver(false);
            emit scoresChanged(m_scoutScore, m_saboteurScore);
            emit gameFieldChanged();
            return true; // Игра окончена
        }

        emit scoresChanged(m_scoutScore, m_saboteurScore); // Обновляем очки после первого хода
        emit gameFieldChanged(); // Обновляем поле после первого хода
        return true; // Первый ход полностью обработан, выходим
    }
    // --- КОНЕЦ ЛОГИКИ ПЕРВОГО ХОДА ---


    // Для всех последующих ходов:
    // Нельзя открыть уже открытую клетку
    if (cell.state == Opened) {
        qDebug() << "openCell: Cell (" << row << "," << col << ") is already opened.";
        return false;
    }

    // Логика для разных ролей (только для Разведчика, так как Диверсант не вызывает openCell)
    if (player == Scout) {
        qDebug() << "openCell: Scout's turn (subsequent click).";
        if (cell.hasMine) {
            qDebug() << "openCell: Cell has mine.";
            if (cell.state == Flagged) { // Если клетка была помечена флагом (Диверсантом)
                cell.state = Opened;
                qDebug() << "openCell: Scout opened a flagged mine at (" << row << "," << col << "). Mine disarmed.";
            } else {
                m_gameOver = true;
                m_gameWon = false;
                cell.state = Opened; // Открываем мину, чтобы показать ее
                emit gameOver(false); // false означает поражение
                qDebug() << "openCell: Scout opened an unflagged mine at (" << row << "," << col << "). Game Over (Scout loses).";
                emit gameFieldChanged(); // Обновляем поле, чтобы показать мину
                return true; // Ход завершен, игра окончена
            }
        } else {
            cell.state = Opened;
            m_scoutScore += 1;
            qDebug() << "openCell: Scout opened a safe cell at (" << row << "," << col << "). Score:" << m_scoutScore;

            if (cell.minesAround == 0) {
                qDebug() << "openCell: Cell has 0 mines around. Revealing empty area.";
                revealEmptyArea(row, col);
            }
        }
    } else { // player == Saboteur
        qWarning() << "openCell: Saboteur tried to open a cell directly. This should not happen.";
        return false; // Диверсант не должен вызывать openCell
    }

    // Проверка победы
    if (checkWinCondition()) {
        m_gameOver = true;
        m_gameWon = true;
        emit gameOver(true); // true означает победу
        qDebug() << "openCell: Win condition met. Game Over (Win).";
    }

    emit scoresChanged(m_scoutScore, m_saboteurScore); // Обновляем очки
    emit gameFieldChanged(); // Уведомляем об изменении поля
    return true; // Ход успешно выполнен
}

bool GameLogic::toggleCellMark(int row, int col)
{
    qDebug() << "GameLogic::toggleCellMark called for (" << row << "," << col << ") by player" << m_currentPlayer;

    if (m_gameOver) {
        qDebug() << "toggleCellMark: Game is over.";
        return false;
    }
    if (!isValidCell(row, col)) {
        qDebug() << "toggleCellMark: Invalid cell coordinates.";
        return false;
    }
    if (m_gameField[row][col].state == Opened) {
        qDebug() << "toggleCellMark: Cannot mark an opened cell.";
        return false; // Нельзя помечать открытые клетки
    }
    if (m_currentPlayer != Saboteur) { // Только Диверсант может помечать
        qDebug() << "toggleCellMark: Not Saboteur's turn or role.";
        return false;
    }

    Cell &cell = m_gameField[row][col];
    int oldState = cell.state;

    if (cell.state == Closed) {
        cell.state = Flagged;
        m_remainingMines--;
        if (cell.hasMine) {
            m_saboteurScore += 10;
            qDebug() << "toggleCellMark: Saboteur flagged a mine. Score:" << m_saboteurScore;
        } else {
            m_saboteurScore -= 5;
            qDebug() << "toggleCellMark: Saboteur flagged a safe cell. Score:" << m_saboteurScore;
        }
    } else if (cell.state == Flagged) {
        cell.state = QuestionMark;
        m_remainingMines++; // Возвращаем мину в счетчик, если флаг снят
        qDebug() << "toggleCellMark: Saboteur changed Flagged to QuestionMark.";
    } else { // cell.state == QuestionMark
        cell.state = Closed;
        qDebug() << "toggleCellMark: Saboteur changed QuestionMark to Closed.";
    }

    emit gameFieldChanged();
    emit scoresChanged(m_scoutScore, m_saboteurScore);
    return true;
}

bool GameLogic::switchPlayerTurn()
{
    m_currentPlayer = (m_currentPlayer == Scout) ? Saboteur : Scout;
    emit playerSwitched(m_currentPlayer);
    return true;
}

void GameLogic::generateMines(int firstClickRow, int firstClickCol)
{
    int minesPlaced = 0;
    while (minesPlaced < m_minesCount) {
        int row = QRandomGenerator::global()->bounded(m_rows);
        int col = QRandomGenerator::global()->bounded(m_cols);
        // Определяем, находится ли клетка в "безопасной зоне" 3x3 вокруг первого клика
        bool inSafeZone = false;
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (row == firstClickRow + dr && col == firstClickCol + dc) {
                    inSafeZone = true;
                    break;
                }
            }
            if (inSafeZone) break;
        }
        // Не ставим мину в безопасной зоне и где уже есть мины
        if (!inSafeZone && !m_gameField[row][col].hasMine) {
            m_gameField[row][col].hasMine = true;
            minesPlaced++;
        }
    }
    qDebug() << "Mines generated, avoiding 3x3 area around (" << firstClickRow << "," << firstClickCol << ")";
}

void GameLogic::calculateMinesAround()
{
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_cols; ++j) {
            if (!m_gameField[i][j].hasMine) {
                int count = 0;
                // Проверяем 8 соседних клеток
                for (int di = -1; di <= 1; ++di) {
                    for (int dj = -1; dj <= 1; ++dj) {
                        if (di == 0 && dj == 0) continue;
                        int ni = i + di;
                        int nj = j + dj;
                        if (isValidCell(ni, nj) && m_gameField[ni][nj].hasMine) {
                            count++;
                        }
                    }
                }
                m_gameField[i][j].minesAround = count;
            }
        }
    }
}

void GameLogic::revealEmptyArea(int row, int col)
{
    QQueue<QPair<int, int>> queue;
    queue.enqueue(qMakePair(row, col));
    qDebug() << "revealEmptyArea: Starting from (" << row << "," << col << ")";
    while (!queue.isEmpty()) {
        auto current = queue.dequeue();
        int r = current.first;
        int c = current.second;
        if (!isValidCell(r, c)) {
            continue;
        }
        Cell &currentCell = m_gameField[r][c];
        // Если клетка уже открыта, является миной, или помечена флагом/вопросом, пропускаем
        // (Разведчик не должен открывать помеченные клетки через revealEmptyArea,
        // только через прямой клик, где логика флага обрабатывается)
        if (currentCell.state == Opened || currentCell.hasMine || currentCell.state == Flagged || currentCell.state == QuestionMark) {
            continue;
        }
        currentCell.state = Opened; // Открываем клетку
        m_scoutScore += 1; // Начисляем очки за каждую открытую клетку в области
        qDebug() << "revealEmptyArea: Opened cell (" << r << "," << c << "). Scout score:" << m_scoutScore;
        if (currentCell.minesAround == 0) {
            // Добавляем все соседние клетки для раскрытия
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    if (dr == 0 && dc == 0) continue;
                    queue.enqueue(qMakePair(r + dr, c + dc));
                }
            }
        }
    }
    qDebug() << "revealEmptyArea: Finished.";
}

bool GameLogic::isValidCell(int row, int col) const
{
    return row >= 0 && row < m_rows && col >= 0 && col < m_cols;
}

bool GameLogic::checkWinCondition() const
{
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_cols; ++j) {
            // Если есть неоткрытая клетка без мины - победа не достигнута
            if (!m_gameField[i][j].hasMine && m_gameField[i][j].state != Opened) {
                return false;
            }
        }
    }
    return true;
}
