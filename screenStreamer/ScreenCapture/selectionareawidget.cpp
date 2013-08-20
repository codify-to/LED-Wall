#include "selectionareawidget.h"
#include <QRegion>
#include <QDebug>
#include <QApplication>

SelectionAreaWidget::SelectionAreaWidget(QWidget *parent) :
    QWidget(parent)
{
    /*****
     * System tray
     **/
    //Initialize the system tray
    trayIcon = new QSystemTrayIcon(QIcon(":/tray_icon.png"));
    trayIcon->setVisible(true);

    //Create the menu actions
    toggleSelectionAction = new QAction("&Toggle Section Area", this);
    connect(toggleSelectionAction, SIGNAL(triggered()), this, SLOT(toggleSelectionArea()));
    quitAction = new QAction("Stop Streaming", this);
    connect(quitAction , SIGNAL(triggered()), this, SLOT(quit()));

    //Add Actions to it
    contextMenu = new QMenu(this);
    contextMenu->addAction(toggleSelectionAction);
    contextMenu->addAction(quitAction);
    trayIcon->setContextMenu(contextMenu);

    /*****
     * Selection Area
     **/
    selectedArea = NULL;
    hideWindowTimer = new QTimer(this);
    hideWindowTimer->setSingleShot(true);
    connect(hideWindowTimer, SIGNAL(timeout()), this, SLOT(hideWindowTimeout()));
    // Window setup
    QDesktopWidget desktop;
    resize(desktop.width(), desktop.height());
    setMask(QRegion(0, 0, desktop.width(), desktop.height()));
    setStyleSheet("background:transparent;");
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);

    show();
}

/***
 *Selection Area
 **/
void SelectionAreaWidget::mouseMoveEvent(QMouseEvent *event)
{
    int x = selectedArea->x();// std::min(selectedArea->x(), event->x());
    int y = selectedArea->y();// std::min(selectedArea->y(), event->y());
    int w = event->x() - x; //std::abs(selectedArea->x() - event->x());
    int h = event->y() - y; //std::abs(selectedArea->y() - event->y());
    selectedArea->setRect(x, y, w, h);


    // Needs redraw
    update();
}
void SelectionAreaWidget::mousePressEvent(QMouseEvent *event)
{
    // Do not hide the window...
    hideWindowTimer->stop();

    // Create a new Rectangle
    if(selectedArea == NULL)
        selectedArea = new QRect();
    selectedArea->setRect(event->x(), event->y(), 0, 0);

    // Needs redraw
    update();
}
void SelectionAreaWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    // Check if the rectangle is big enough
    if(qAbs(selectedArea->width()*selectedArea->height()) < 256){
        selectedArea = NULL;
    }
    // Needs redraw
    update();

    hideWindowTimer->start(2000);
}
void SelectionAreaWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    painter.eraseRect(0, 0, 100, 100);

    QRegion clipRegion(0, 0, width(), height());
    if(selectedArea != NULL){
        int x = selectedArea->left() + std::min(selectedArea->width(), 0);
        int y = selectedArea->top() + std::min(selectedArea->height(), 0);
        //
        clipRegion -= QRegion(x, y, std::abs(selectedArea->width()), std::abs(selectedArea->height()));
    }

    QColor black(0, 0, 0, 100);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRegion(clipRegion);
    painter.fillRect(0, 0, width(), height(), black);

}
void SelectionAreaWidget::hideWindowTimeout()
{
    hide();
}

/***
 *Menu Implementation
 **/
void SelectionAreaWidget::toggleSelectionArea()
{
    setVisible(!this->isVisible());
}
void SelectionAreaWidget::quit()
{
    QApplication::quit();
}

SelectionAreaWidget::~SelectionAreaWidget()
{
    trayIcon->setVisible(false);
    delete trayIcon;
}
