#pragma once

#include <vector>
#include <string>
#include <istream>
#include <ostream>

namespace conway
{
    struct Game
    {
        Game(size_t h_, size_t w_)
            : h(h_),
              w(w_),
              state(h, row_t(w, false)),
              nextState(h, row_t(w, false))
        {
        }

        size_t height() const { return h; }
        size_t width() const { return w; }
        bool get(size_t i, size_t j) const { return state[i][j]; }
        void set(size_t i, size_t j, bool v) { state[i][j] = v; }
        void resize(size_t h_, size_t w_)
        {
            h = h_;
            w = w_;
            state.resize(h);
            nextState.resize(h);
            for (size_t i = 0; i < h; ++i)
            {
                state[i].resize(w, false);
                nextState[i].resize(w, false);
            }
        }

        void clear()
        {
            state.clear();
            nextState.clear();
            resize(h, w);
        }

        void nextGeneration()
        {
            for (size_t i = 0; i < h; ++i)
            {
                for (size_t j = 0; j < w; ++j)
                {
                    size_t neighbors = countNeighbors(i, j);
                    bool cellState = get(i, j);

                    if (cellState)
                    {
                        if (neighbors < 2 || neighbors > 3) cellState = false;
                    }
                    else if (neighbors == 3)
                    {
                        cellState = true;
                    }

                    nextState[i][j] = cellState;
                }
            }

            state.swap(nextState);
        }

        inline size_t countNeighbors(int i, int j)
        {
            size_t result = 0;
            for (int x = -1; x <= 1; ++x)
            {
                size_t ni = (i + x < 0) ? h + x : (i + x) % h;
                for (int y = -1; y <= 1; ++y)
                {
                    if (!x && !y) continue;
                    size_t nj = (j + y < 0) ? w + y : (j + y) % w;
                    result += get(ni, nj) ? 1 : 0;
                }
            }
            return result;
        }

        std::ostream& write(std::ostream& os)
        {
            for (size_t i = 0; i < h; ++i)
            {
                for (size_t j = 0; j < w; ++j)
                {
                    bool cellState = get(i, j);
                    os << (cellState ? '1' : '0');
                }
                os << std::endl;
            }
            return os;
        }

        void read(std::istream& is)
        {
            state.clear();

            size_t h_ = 0, w_ = 0;
            std::string tmp;
            while ((is >> tmp).good())
            {
                w_ = std::max(tmp.size(), w_);
                state.push_back(row_t(w_, false));

                for (size_t i = 0; i < tmp.size(); ++i)
                {
                    state[h_][i] = tmp[i] == '1';
                }

                h_++;
            }
            resize(h_, w_);
        }

        void orWithAt(const Game& o, size_t y, size_t x)
        {
            for (size_t i = 0; i < o.height() && i + y < h; ++i)
            {
                for (size_t j = 0; j < o.width() && j + x < w; ++j)
                {
                    set(y + i, x + j, get(y + i, x + j) || o.get(i, j));
                }
            }
        }

    protected:
        size_t h, w;
        typedef std::vector<bool> row_t;
        typedef std::vector<row_t> state_t;
        state_t state, nextState;
    };

}  // namespace conway
