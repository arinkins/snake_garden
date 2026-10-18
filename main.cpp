// ============================================================
//  Игра "Змейка" - вариант "Сад"
//  Поле 10x10, три типа фруктов:
//    'A' - яблоко  (+1 очко)
//    'B' - банан   (+2 очка)
//    'C' - вишня   (+3 очка, исчезает через 3 хода)
//  Управление: WASD / стрелки, H - справка, Q - выход
// ============================================================

#include <ncurses.h>
#include <vector>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <string>

// ---------- Константы игры ----------
const int FIELD_W = 10;          // ширина поля
const int FIELD_H = 10;          // высота поля
const int CHERRY_LIFE = 3;       // сколько ходов живёт вишня

// Смещения для экрана (где рисовать поле в терминале)
const int OFFSET_Y = 3;
const int OFFSET_X = 2;

// ---------- Типы данных ----------
struct Point { int x, y; };

enum FruitType { APPLE, BANANA, CHERRY };

struct Fruit {
    Point pos;
    FruitType type;
    int life;    // для вишни — оставшиеся ходы; для остальных не используется
    bool alive;
};

enum Direction { UP, DOWN, LEFT, RIGHT };

// ============================================================
//  Пользовательские функции
// ============================================================

// Проверка: занята ли клетка змейкой
bool isOnSnake(const std::deque<Point>& snake, const Point& p) {
    for (const auto& s : snake)
        if (s.x == p.x && s.y == p.y) return true;
    return false;
}

// Проверка: занята ли клетка уже имеющимся фруктом
bool isOnFruit(const std::vector<Fruit>& fruits, const Point& p) {
    for (const auto& f : fruits)
        if (f.alive && f.pos.x == p.x && f.pos.y == p.y) return true;
    return false;
}

// Случайная свободная точка на поле
Point randomFreeCell(const std::deque<Point>& snake,
                     const std::vector<Fruit>& fruits) {
    Point p;
    do {
        p.x = rand() % FIELD_W;
        p.y = rand() % FIELD_H;
    } while (isOnSnake(snake, p) || isOnFruit(fruits, p));
    return p;
}

// Создать новый фрукт случайного типа
Fruit spawnFruit(const std::deque<Point>& snake,
                 const std::vector<Fruit>& fruits) {
    Fruit f;
    f.pos  = randomFreeCell(snake, fruits);
    f.type = static_cast<FruitType>(rand() % 3);
    f.alive = true;
    f.life = (f.type == CHERRY) ? CHERRY_LIFE : -1;
    return f;
}

// Символ для отображения фрукта
char fruitChar(FruitType t) {
    switch (t) {
        case APPLE:  return 'A';
        case BANANA: return 'B';
        case CHERRY: return 'C';
    }
    return '?';
}

// Очки за фрукт
int fruitScore(FruitType t) {
    switch (t) {
        case APPLE:  return 1;
        case BANANA: return 2;
        case CHERRY: return 3;
    }
    return 0;
}

// Следующая позиция головы по направлению
Point nextHead(const Point& head, Direction dir) {
    Point n = head;
    switch (dir) {
        case UP:    n.y -= 1; break;
        case DOWN:  n.y += 1; break;
        case LEFT:  n.x -= 1; break;
        case RIGHT: n.x += 1; break;
    }
    return n;
}

// ---------- Отрисовка ----------
void drawField(const std::deque<Point>& snake,
               const std::vector<Fruit>& fruits,
               int score, int turn) {
    clear();

    // Заголовок
    mvprintw(0, OFFSET_X, "=== SNAKE: GARDEN (variant 6) ===");
    mvprintw(1, OFFSET_X, "Score: %d   Turn: %d   [H]help [Q]quit", score, turn);

    // Верхняя граница
    mvaddch(OFFSET_Y, OFFSET_X, '+');
    for (int i = 0; i < FIELD_W; ++i) addch('-');
    addch('+');

    // Поле
    for (int y = 0; y < FIELD_H; ++y) {
        mvaddch(OFFSET_Y + 1 + y, OFFSET_X, '|');
        for (int x = 0; x < FIELD_W; ++x) {
            Point cur{x, y};
            char ch = ' ';

            if (isOnSnake(snake, cur)) {
                ch = (cur.x == snake.front().x && cur.y == snake.front().y)
                     ? '@' : 'o';
            } else {
                for (const auto& f : fruits) {
                    if (f.alive && f.pos.x == x && f.pos.y == y) {
                        ch = fruitChar(f.type);
                        break;
                    }
                }
            }
            addch(ch);
        }
        addch('|');
    }

    // Нижняя граница
    mvaddch(OFFSET_Y + FIELD_H + 1, OFFSET_X, '+');
    for (int i = 0; i < FIELD_W; ++i) addch('-');
    addch('+');

    // Легенда
    mvprintw(OFFSET_Y + FIELD_H + 3, OFFSET_X,
             "A=apple(+1)  B=banana(+2)  C=cherry(+3, 3 turns)");
    refresh();
}

// ---------- Справка ----------
void showHelp(bool pause) {
    clear();
    mvprintw(1, 2, "=== HELP / RULES ===");
    mvprintw(3, 2, "Goal: eat as many fruits as possible without dying.");
    mvprintw(4, 2, "Field: 10x10. Snake starts at center, length 1.");
    mvprintw(5, 2, "Fruits:");
    mvprintw(6, 4, "A - apple  : +1 point");
    mvprintw(7, 4, "B - banana : +2 points");
    mvprintw(8, 4, "C - cherry : +3 points, disappears after 3 turns");
    mvprintw(10, 2, "Controls:");
    mvprintw(11, 4, "W A S D or arrow keys - move");
    mvprintw(12, 4, "H - show this help");
    mvprintw(13, 4, "Q - quit game");
    mvprintw(15, 2, "Lose: hit wall or your own tail.");
    mvprintw(17, 2, "Press any key to %s...", pause ? "start" : "continue");
    refresh();
    timeout(-1);   // блокирующий ввод
    getch();
    flushinp();
    timeout(0);    // вернуть неблокирующий
}

// ---------- Экран проигрыша ----------
void gameOver(int score) {
    clear();
    mvprintw(5, 2, "=== GAME OVER ===");
    mvprintw(7, 2, "Your score: %d", score);
    mvprintw(9, 2, "Press any key to exit...");
    refresh();
    timeout(-1);
    getch();
}

// ============================================================
//  Основной игровой цикл
// ============================================================
int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    // Инициализация ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(0);          // неблокирующий getch

    // Справка перед началом игры
    showHelp(true);
    flushinp();
    // --- Инициализация состояния ---
    std::deque<Point> snake;
    snake.push_front({FIELD_W / 2, FIELD_H / 2});  // старт в центре

    std::vector<Fruit> fruits;
    fruits.push_back(spawnFruit(snake, fruits));   // один фрукт на старте

    Direction dir = RIGHT;
    Direction pendingDir = RIGHT;   // буфер, чтобы нельзя было развернуться на 180°

    int score = 0;
    int turn  = 0;
    bool running = true;

    // --- Главный цикл ---
    while (running) {
        drawField(snake, fruits, score, turn);
        napms(140); 
        // --- Ввод ---
        int ch = getch();
        switch (ch) {
            case 'w': case 'W': case KEY_UP:
                if (dir != DOWN) pendingDir = UP; break;
            case 's': case 'S': case KEY_DOWN:
                if (dir != UP) pendingDir = DOWN; break;
            case 'a': case 'A': case KEY_LEFT:
                if (dir != RIGHT) pendingDir = LEFT; break;
            case 'd': case 'D': case KEY_RIGHT:
                if (dir != LEFT) pendingDir = RIGHT; break;
            case 'h': case 'H':
                showHelp(false);
                continue;
            case 'q': case 'Q':
                running = false;
                continue;
            default:
                break;
        }
        dir = pendingDir;

        // --- Шаг змейки ---
        Point newHead = nextHead(snake.front(), dir);

        // Проверка стены
        if (newHead.x < 0 || newHead.x >= FIELD_W ||
            newHead.y < 0 || newHead.y >= FIELD_H) {
            gameOver(score);
            break;
        }
        // Проверка хвоста (кроме последнего сегмента, который сдвинется)
        {
            std::deque<Point> body = snake;
            body.pop_back();  // хвост уйдёт
            if (isOnSnake(body, newHead)) {
                gameOver(score);
                break;
            }
        }

        // --- Поедание фрукта ---
        bool ate = false;
        for (auto& f : fruits) {
            if (f.alive && f.pos.x == newHead.x && f.pos.y == newHead.y) {
                score += fruitScore(f.type);
                f.alive = false;
                ate = true;
                break;
            }
        }

        // Двигаем змейку
        snake.push_front(newHead);
        if (!ate) snake.pop_back();   // не выросла — хвост убираем

        // --- Обновление фруктов ---
        // Убираем мёртвые
        for (auto it = fruits.begin(); it != fruits.end(); ) {
            if (!it->alive) it = fruits.erase(it);
            else ++it;
        }
        // Уменьшаем жизнь вишен, удаляем просроченные
        for (auto it = fruits.begin(); it != fruits.end(); ) {
            if (it->type == CHERRY && it->life > 0) {
                it->life--;
                if (it->life == 0) { it = fruits.erase(it); continue; }
            }
            ++it;
        }
        // Если фруктов нет — рождаем новый
        if (fruits.empty()) {
            fruits.push_back(spawnFruit(snake, fruits));
        }

        ++turn;
        napms(150);
    }

    endwin();
    return 0;
}