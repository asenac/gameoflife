#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QWidget>
#include "conway.hpp"

class QSpinBox;

namespace conway
{
    struct Canvas : QWidget
    {
        Q_OBJECT
    public:
        Canvas(Game& game_);

    public slots:
        void setGrid(bool grid_);

    protected:
        virtual void paintEvent(QPaintEvent* event);

        virtual void mousePressEvent(QMouseEvent* event);

        virtual void mouseReleaseEvent(QMouseEvent* event);

        virtual void mouseMoveEvent(QMouseEvent* event);

        virtual void dragEnterEvent(QDragEnterEvent* event);

        void mergeDropEvent(QDropEvent* event, Game& targetGame);

        virtual void dragMoveEvent(QDragMoveEvent* event);

        virtual void dragLeaveEvent(QDragLeaveEvent* event);

        virtual void dropEvent(QDropEvent* event);

        void draw(QPainter& painter, const QRect& rect, const Game& game);

        Game& game;
        bool grid;

        // Drag and drop
        QPoint dragStartPosition;
        bool dropInProgress;
        Qt::DropAction dropAction;
        Game dropGame;  // Temporary game for drop actions
    };

    struct GameWidget : QWidget
    {
        Q_OBJECT
    public:
        static const size_t defaultWidth = 50;
        static const size_t defaultHeight = 50;
        static const size_t defaultPeriod = 200;

        GameWidget();

    protected slots:

        void togglePlay(bool play);

        void resize();

        void clear();

        void advanceGame();

        void changePeriod(int period);

        void randomize();

    protected:
        Game game;
        Canvas* canvas;
        QTimer timer;
        QSpinBox *heightSpinBox, *widthSpinBox, *periodSpinBox;
    };

    struct MainWindow : QMainWindow
    {
        Q_OBJECT
    public:
        MainWindow();

    protected:
        GameWidget* gameWidget;
    };

}  // namespace conway
