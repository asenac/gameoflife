#include "widgets.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <cstdlib>
#include <sstream>
#include <iostream>

using namespace conway;

//
// Canvas
//

Canvas::Canvas(Game& game_)
    : game(game_),
      grid(false),
      menu(this),
      dropInProgress(false),
      dropAction(Qt::CopyAction),
      dropGame(game)
{
    setMinimumSize(500, 500);
    setAcceptDrops(true);

    menu.addAction("Save image as...", this, SLOT(saveImage()));
}

void Canvas::setGrid(bool grid_)
{
    grid = grid_;
    update();
}

void Canvas::showContextMenu(const QPoint& pos)
{
    const QPoint globalPos = mapToGlobal(pos);

    menu.exec(globalPos);
}

void Canvas::saveImage()
{
    const QString fileName =
        QFileDialog::getSaveFileName(this, "Save image as...", "./conway.png",
                                     "Images (*.png *.xpm *.jpg *.bmp)");

    if (fileName.isEmpty())
        return;

    float hfactor = float(rect().height()) / game.height();
    float wfactor = float(rect().width()) / game.width();

    QPixmap pix(wfactor * game.width(), hfactor * game.height());
    QPainter painter(&pix);
    draw(painter, pix.rect(), game);
    pix.save(fileName);
}

void Canvas::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    draw(painter, rect(), dropInProgress ? dropGame : game);
}

void Canvas::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragStartPosition = event->pos();
    }
    else if (event->button() == Qt::RightButton)
    {
        showContextMenu(event->pos());
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (QApplication::keyboardModifiers() & Qt::ShiftModifier) return;

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
}

void Canvas::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton)) return;

    float hfactor = float(rect().height()) / game.height();
    float wfactor = float(rect().width()) / game.width();
    size_t x = event->x() / wfactor;
    size_t y = event->y() / hfactor;

    if (QApplication::keyboardModifiers() & Qt::ShiftModifier)
    {
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
        Game figure;

        if ((QApplication::keyboardModifiers() & Qt::AltModifier) &&
            game.get(y, x))
        {
            game.extractFigureAt(figure, y, x);
            figure.write(oss);
        }
        else
        {
            game.write(oss);

            QPixmap pix(wfactor * game.width(), hfactor * game.height());
            QPainter painter(&pix);
            draw(painter, pix.rect(), game);
            drag->setPixmap(pix);
        }

        mimeData->setText(oss.str().c_str());
        drag->setMimeData(mimeData);

        // Qt::DropAction dropAction =
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
}

void Canvas::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/plain"))
    {
        event->acceptProposedAction();
        dropAction = event->dropAction();
        dropInProgress = true;
    }
}

void Canvas::mergeDropEvent(QDropEvent* event, Game& targetGame)
{
    std::istringstream iss(event->mimeData()->text().toStdString());

    if (dropAction & Qt::CopyAction)
    {
        Game tmpGame(0, 0);
        tmpGame.read(iss);

        float hfactor = float(rect().height()) / game.height();
        float wfactor = float(rect().width()) / game.width();
        size_t y = event->pos().y() / hfactor;
        size_t x = event->pos().x() / wfactor;

        if (y < targetGame.height() && x < targetGame.width())
        {
            targetGame.orWithAt(tmpGame, y, x);
        }
    }
    else
    {
        targetGame.read(iss);
    }

    update();
}

void Canvas::dragMoveEvent(QDragMoveEvent* event)
{
    if (dropInProgress)
    {
        event->acceptProposedAction();
        dropAction = event->dropAction();
        dropGame = game;  // resync
        mergeDropEvent(event, dropGame);
    }
}

void Canvas::dragLeaveEvent(QDragLeaveEvent* event)
{
    if (dropInProgress)
    {
        dropInProgress = false;
        update();  // Repaint to display the right game
    }
}

void Canvas::dropEvent(QDropEvent* event)
{
    // event->acceptProposedAction();
    dropInProgress = false;
    mergeDropEvent(event, game);
}

void Canvas::draw(QPainter& painter, const QRect& rect, const Game& game)
{
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
                painter.fillRect(x * wfactor, y * hfactor, wfactor, hfactor,
                                 Qt::black);
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

//
// GameWidget
//

GameWidget::GameWidget()
    : game(defaultHeight, defaultWidth), canvas(new Canvas(game)), timer(this)
{
    QFormLayout* formLayout = new QFormLayout();
    QGridLayout* gridLayout = new QGridLayout();

    widthSpinBox = new QSpinBox();
    widthSpinBox->setRange(4, 200);
    widthSpinBox->setValue(defaultWidth);
    gridLayout->addWidget(new QLabel("Width"), 0, 0);
    gridLayout->addWidget(widthSpinBox, 0, 1);

    heightSpinBox = new QSpinBox();
    heightSpinBox->setRange(4, 200);
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
    connect(pushButton, SIGNAL(toggled(bool)), this, SLOT(togglePlay(bool)));
    connect(heightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(resize()));
    connect(widthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(resize()));
    connect(periodSpinBox, SIGNAL(valueChanged(int)), this,
            SLOT(changePeriod(int)));
    connect(randomizeButton, SIGNAL(clicked(bool)), this, SLOT(randomize()));
    connect(clearButton, SIGNAL(clicked(bool)), this, SLOT(clear()));
    connect(checkBox, SIGNAL(toggled(bool)), canvas, SLOT(setGrid(bool)));

    checkBox->setChecked(true);
}

void GameWidget::togglePlay(bool play)
{
    if (play)
        timer.start(periodSpinBox->value());
    else
        timer.stop();
}

void GameWidget::resize()
{
    game.resize(heightSpinBox->value(), widthSpinBox->value());
    canvas->update();
}

void GameWidget::clear()
{
    game.clear();
    canvas->update();
}

void GameWidget::advanceGame()
{
    game.nextGeneration();
    canvas->update();
}

void GameWidget::changePeriod(int period)
{
    if (timer.isActive())
    {
        timer.stop();
        timer.start(period);
    }
}

void GameWidget::randomize()
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

//
// MainWindow
//

MainWindow::MainWindow()
{
    gameWidget = new GameWidget();
    setCentralWidget(gameWidget);
    setWindowTitle("Conway's game of life");
}
