class Logic
{
public:
    Logic(Board* board, Config* config) : board(board), config(config)
    {
        rand_eng = std::default_random_engine(
            !((*config)("Bot", "NoRandom")) ? unsigned(time(0)) : 0);
        scoring_mode = (*config)("Bot", "BotScoringType");
        optimization = (*config)("Bot", "Optimization");
        Max_depth = 5;  // Установим максимальную глубину для алгоритма минимакс
    }

    // Основной метод для вызова минимакс алгоритма и получения лучшего хода для бота
    vector<move_pos> get_best_move(const bool color)
    {
        next_best_state.clear();
        next_move.clear();

        int cur_state = 0;
        vector<move_pos> res;
        do
        {
            res.push_back(next_move[cur_state]);
            cur_state = next_best_state[cur_state];
        } while (cur_state != -1 && next_move[cur_state].x != -1);

        return res;
    }

private:
    // Метод для вычисления оценки позиции на доске
    double calc_score(const vector<vector<POS_T>>& mtx, const bool first_bot_color) const
    {
        double w = 0, wq = 0, b = 0, bq = 0;
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                w += (mtx[i][j] == 1);   // Добавляем обычные фигуры белых
                wq += (mtx[i][j] == 3);  // Добавляем ферзи белых
                b += (mtx[i][j] == 2);   // Добавляем обычные фигуры черных
                bq += (mtx[i][j] == 4);  // Добавляем ферзи черных
                if (scoring_mode == "NumberAndPotential")
                {
                    w += 0.05 * (mtx[i][j] == 1) * (7 - i);  // Учитываем позицию для обычных фигур
                    b += 0.05 * (mtx[i][j] == 2) * (i);      // Учитываем позицию для черных фигур
                }
            }
        }
        if (!first_bot_color)
        {
            swap(b, w);  // Инвертируем результаты для черных и белых
            swap(bq, wq);
        }

        // Если на доске нет фигур, то возвращаем бесконечность
        if (w + wq == 0)
            return INF;
        if (b + bq == 0)
            return 0;

        int q_coef = (scoring_mode == "NumberAndPotential") ? 5 : 4; // Учитываем коэффициент для ферзей
        return (b + bq * q_coef) / (w + wq * q_coef); // Оценка позиции
    }

    // Метод для выполнения хода на доске
    vector<vector<POS_T>> make_turn(vector<vector<POS_T>> mtx, move_pos turn) const
    {
        if (turn.xb != -1)
            mtx[turn.xb][turn.yb] = 0;  // Убираем фигуру, которую побили
        if ((mtx[turn.x][turn.y] == 1 && turn.x2 == 0) || (mtx[turn.x][turn.y] == 2 && turn.x2 == 7))
            mtx[turn.x][turn.y] += 2;  // Превращаем фигуру в ферзя
        mtx[turn.x2][turn.y2] = mtx[turn.x][turn.y];  // Перемещаем фигуру
        mtx[turn.x][turn.y] = 0;  // Ставим пустое место
        return mtx;
    }

    // Метод для выполнения минимакс с альфа-бета отсечением
    double minimax(vector<vector<POS_T>> mtx, bool color, size_t depth, double alpha = -INF, double beta = INF, POS_T x = -1, POS_T y = -1)
    {
        if (depth == Max_depth)  // Если достигли максимальной глубины
        {
            return calc_score(mtx, color);
        }

        if (x != -1)
        {
            find_turns(x, y, mtx);  // Ищем возможные ходы для конкретной фигуры
        }
        else
        {
            find_turns(color, mtx);  // Ищем ходы для всего игрока
        }

        auto turns_now = turns;
        bool have_beats_now = have_beats;

        if (!have_beats_now && x != -1)
        {
            return minimax(mtx, 1 - color, depth + 1, alpha, beta);
        }

        if (turns.empty())
            return (depth % 2 == 0 ? INF : -INF);  // Если нет ходов, возвращаем максимально плохой результат

        double best_score = (depth % 2 == 0) ? -INF : INF;

        for (auto turn : turns_now)
        {
            vector<vector<POS_T>> new_mtx = make_turn(mtx, turn);  // Применяем ход
            double score = minimax(new_mtx, 1 - color, depth + 1, alpha, beta);  // Рекурсивно считаем результат

            // Обновляем лучший результат в зависимости от глубины
            if (depth % 2 == 0)  // Максимизируем для бота
            {
                best_score = max(best_score, score);
                alpha = max(alpha, best_score);
            }
            else  // Минимизируем для соперника
            {
                best_score = min(best_score, score);
                beta = min(beta, best_score);
            }

            // Альфа-бета отсечение
            if (alpha >= beta)
                break;
        }

        return best_score;
    }

    // Основной метод для нахождения лучшего хода для бота
    void find_best_move(bool color)
    {
        double best_score = -INF;
        move_pos best_move;

        for (auto turn : turns)
        {
            vector<vector<POS_T>> new_mtx = make_turn(board->get_board(), turn);  // Применяем ход
            double score = minimax(new_mtx, 1 - color, 0, -INF, INF);  // Прогоняем минимакс

            if (score > best_score)
            {
                best_score = score;
                best_move = turn;
            }
        }

        next_move.push_back(best_move);  // Запоминаем лучший ход
    }

private:
    Board* board;
    Config* config;
    vector<move_pos> turns;
    bool have_beats;
    int Max_depth;  // Глубина для минимакс
    default_random_engine rand_eng;
    string scoring_mode;
    string optimization;
    vector<move_pos> next_move;
    vector<int> next_best_state;
};
