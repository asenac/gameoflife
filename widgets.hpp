#pragma once

#include <QApplication>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QWidget>
#include <QCheckBox>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include "conway.hpp"

namespace conway
{
    struct Canvas : QWidget
    {
        Q_OBJECT
    public:
        Canvas(Game& game_) : game(game_), grid(false)
        {
            setMinimumSize(500, 500);
            setAcceptDrops(true);
        }

    public slots:
        void setGrid(bool grid_)
        {
            grid = grid_;
            update();
        }

    protected:
        virtual void paintEvent(QPaintEvent* event) { draw(rect()); }
        virtual void mousePressEvent(QMouseEvent* event)
        {
            if (event->button() == Qt::LeftButton)
                dragStartPosition = event->pos();
        }

        virtual void mouseReleaseEvent(QMouseEvent* event)
        {
            if (QApplication::keyboardModifiers() & Qt::ShiftModifier)
                return;

            float hfactor = float(rect().height()) / game.height();
            float wfactor = float(rect().width()) / game.width();
            size_t x = event->x() / wfactor;
            size_t y = event->y() / hfactor;
            if (x < game.width() && y < game.height())
            {
                game.set(y, x, !game.get(y, x));
                update();
            }
        }

        virtual void mouseMoveEvent(QMouseEvent* event)
        {
            if (!(event->buttons() & Qt::LeftButton)) return;

            if (QApplication::keyboardModifiers() & Qt::ShiftModifier)
            {
                float hfactor = float(rect().height()) / game.height();
                float wfactor = float(rect().width()) / game.width();
                size_t x = event->x() / wfactor;
                size_t y = event->y() / hfactor;
                if (x < game.width() && y < game.height())
                {
                    game.set(y, x, true);
                    update();
                }
            }
            else if ((event->pos() - dragStartPosition).manhattanLength() <
                     QApplication::startDragDistance())
            {
                QDrag* drag = new QDrag(this);
                QMimeData* mimeData = new QMimeData;

                std::ostringstream oss;
                game.write(oss);
                mimeData->setText(oss.str().c_str());
                drag->setMimeData(mimeData);

                // Qt::DropAction dropAction =
                drag->exec(Qt::CopyAction | Qt::MoveAction);
            }
        }

        virtual void dragEnterEvent(QDragEnterEvent *event)
        {
            if (event->mimeData()->hasFormat("text/plain"))
                event->acceptProposedAction();
        }

        virtual void dropEvent(QDropEvent *event)
        {
            std::istringstream iss(event->mimeData()->text().toStdString());
            game.read(iss);
            event->acceptProposedAction();
            update();
        }

        void draw(const QRect& rect)
        {
            QPainter painter(this);
            painter.setPen(Qt::gray);

            float hfactor = float(rect.height()) / game.height();
            float wfactor = float(rect.width()) / game.width();
            int maxWidth = wfactor * game.width();
            int maxHeight = hfactor * game.height();

            painter.fillRect(0, 0, maxWidth, maxHeight, Qt::white);

            for (size_t y = 0; y < game.height(); ++y)
            {
                for (size_t x = 0; x < game.width(); ++x)
                {
                    if (game.get(y, x))
                    {
                        painter.fillRect(x * wfactor, y * hfactor, wfactor,
                                         hfactor, Qt::black);
                    }
                }
            }

            if (grid)
            {
                // vertical lines
                for (int i = 0; i <= game.width(); ++i)
                {
                    painter.drawLine(i * wfactor, 0, i * wfactor, maxHeight);
                }

                // horizontal lines
                for (int i = 0; i <= game.height(); ++i)
                {
                    painter.drawLine(0, i * hfactor, maxWidth, i * hfactor);
                }
            }
        }

        Game& game;
        bool grid;
        QPoint dragStartPosition;
    };

    struct GameWidget : QWidget
    {
        Q_OBJECT
    public:
        static const size_t defaultWidth = 50;
        static const size_t defaultHeight = 50;
        static const size_t defaultPeriod = 200;

        GameWidget()
            : game(defaultHeight, defaultWidth),
              canvas(new Canvas(game)),
              timer(this)
        {
            QFormLayout* formLayout = new QFormLayout();
            QGridLayout* gridLayout = new QGridLayout();

            widthSpinBox = new QSpinBox();
            widthSpinBox->setRange(10, 200);
            widthSpinBox->setValue(defaultWidth);
            gridLayout->addWidget(new QLabel("Width"), 0, 0);
            gridLayout->addWidget(widthSpinBox, 0, 1);

            heightSpinBox = new QSpinBox();
            heightSpinBox->setRange(10, 200);
            heightSpinBox->setValue(defaultHeight);
            gridLayout->addWidget(new QLabel("Height"), 0, 2);
            gridLayout->addWidget(heightSpinBox, 0, 3);

            periodSpinBox = new QSpinBox();
            periodSpinBox->setRange(100, 1000);
            periodSpinBox->setValue(defaultPeriod);
            gridLayout->addWidget(new QLabel("Period (msec)"), 1, 0);
            gridLayout->addWidget(periodSpinBox, 1, 1);

            QCheckBox* checkBox = new QCheckBox("Grid");
            gridLayout->addWidget(checkBox, 1, 2);

            QPushButton* pushButton = new QPushButton("Play");
            pushButton->setCheckable(true);
            gridLayout->addWidget(pushButton, 2, 0);

            QPushButton* randomizeButton = new QPushButton("Random");
            gridLayout->addWidget(randomizeButton, 2, 1);

            QPushButton* clearButton = new QPushButton("Clear");
            gridLayout->addWidget(clearButton, 2, 2);

            formLayout->addRow(gridLayout);
            formLayout->addRow(canvas);

            setLayout(formLayout);

            connect(&timer, SIGNAL(timeout()), this, SLOT(advanceGame()));
            connect(pushButton, SIGNAL(toggled(bool)), this,
                    SLOT(togglePlay(bool)));
            connect(heightSpinBox, SIGNAL(valueChanged(int)), this,
                    SLOT(resize()));
            connect(widthSpinBox, SIGNAL(valueChanged(int)), this,
                    SLOT(resize()));
            connect(periodSpinBox, SIGNAL(valueChanged(int)), this,
                    SLOT(changePeriod(int)));
            connect(randomizeButton, SIGNAL(clicked(bool)), this,
                    SLOT(randomize()));
            connect(clearButton, SIGNAL(clicked(bool)), this,
                    SLOT(clear()));
            connect(checkBox, SIGNAL(toggled(bool)), canvas,
                    SLOT(setGrid(bool)));

            checkBox->setChecked(true);
        }

    protected slots:

        void togglePlay(bool play)
        {
            if (play)
                timer.start(periodSpinBox->value());
            else
                timer.stop();
        }

        void resize()
        {
            game.resize(heightSpinBox->value(), widthSpinBox->value());
            canvas->update();
        }

        void clear()
        {
            game.clear();
            canvas->update();
        }

        void advanceGame()
        {
            game.nextGeneration();
            canvas->update();
        }

        void changePeriod(int period)
        {
            if (timer.isActive())
            {
                timer.stop();
                timer.start(period);
            }
        }

        void randomize()
        {
            for (size_t y = 0; y < game.height(); ++y)
            {
                for (size_t x = 0; x < game.width(); ++x)
                {
                    game.set(y, x, random() % 7 == 0);
                }
            }
            canvas->update();
        }

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
        MainWindow()
        {
            gameWidget = new GameWidget();
            setCentralWidget(gameWidget);
            setWindowTitle("Conway's game of life");
        }

    protected:
        GameWidget* gameWidget;
    };

}  // namespace conway
