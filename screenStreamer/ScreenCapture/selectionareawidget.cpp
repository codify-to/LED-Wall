#include "selectionareawidget.h"
#include <QRegion>
#include <QDebug>
#include <QApplication>
#include <QScreen>

#define FPS 1000/24
#define LED_WALL_WIDTH 24
#define LED_WALL_HEIGHT 16

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
    showPreviewAction = new QAction("Show Preview", this);
    connect(showPreviewAction , SIGNAL(triggered()), this, SLOT(showPreview()));
    quitAction = new QAction("Stop Streaming", this);
    connect(quitAction , SIGNAL(triggered()), this, SLOT(quit()));

    //Add Actions to it
    contextMenu = new QMenu(this);
    contextMenu->addAction(toggleSelectionAction);
    contextMenu->addAction(showPreviewAction);
    contextMenu->addAction(quitAction);
    trayIcon->setContextMenu(contextMenu);

    /*****
     * Selection Area
     **/
    selectedArea = new QRect();;
    hideWindowTimer = new QTimer(this);
    hideWindowTimer->setSingleShot(true);
    connect(hideWindowTimer, SIGNAL(timeout()), this, SLOT(hideWindowTimeout()));
    // Window setup
    QDesktopWidget desktop;
    resize(desktop.width(), desktop.height());
    setMask(QRegion(0, 0, desktop.width(), desktop.height()));
    setStyleSheet("background:transparent;");
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);

    //DEBUG
    //show();

    /****
     * screen grabbing
     **/
    normalizedSelectedArea = new QRect(0, 0, desktop.width(), desktop.height());
    grabScreenTimer = new QTimer(this);
    connect(grabScreenTimer, SIGNAL(timeout()), this, SLOT(grabScreen()));
    grabScreenTimer->start(FPS);

    /****
     * Preview
     **/
    previewWindow = new PreviewWidget();
    previewWindow->resize(LED_WALL_WIDTH * 3, LED_WALL_HEIGHT * 3);
}

/***
 *Selection Area
 **/
void SelectionAreaWidget::mouseMoveEvent(QMouseEvent *event)
{
    int x = selectedArea->x();
    int y = selectedArea->y();
    int w = event->x() - x;
    int h = event->y() - y;
    selectedArea->setRect(x, y, w, h);


    // Needs redraw
    update();
}
void SelectionAreaWidget::mousePressEvent(QMouseEvent *event)
{
    // Do not hide the window...
    hideWindowTimer->stop();

    // Create a new Rectangle
    selectedArea->setRect(event->x(), event->y(), 0, 0);

    // Needs redraw
    update();
}
void SelectionAreaWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    // Check if the rectangle is big enough
    if(qAbs(selectedArea->width()*selectedArea->height()) < 256){
        selectedArea->setRect(0, 0, width(), height());
    }

    // Normalize the selected area (optimization)
    int x = selectedArea->left() + std::min(selectedArea->width(), 0);
    int y = selectedArea->top() + std::min(selectedArea->height(), 0);
    delete normalizedSelectedArea;
    normalizedSelectedArea = new QRect(x, y, std::abs(selectedArea->width()), std::abs(selectedArea->height()));

    // Needs redraw
    update();

    hideWindowTimer->start(2000);
}
void SelectionAreaWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    QRegion clipRegion(0, 0, width(), height());
    if(selectedArea != NULL){
        int x = selectedArea->left() + std::min(selectedArea->width(), 0);
        int y = selectedArea->top() + std::min(selectedArea->height(), 0);
        //
        QRect selection = QRect(x, y, std::abs(selectedArea->width()), std::abs(selectedArea->height()));
        painter.fillRect(selection, QColor(100, 255, 100, 100));

        clipRegion -= QRegion(selection);
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
 * Screen grabbing
 **/
void SelectionAreaWidget::grabScreen()
{
    QPixmap screenTexture = QGuiApplication::primaryScreen()->grabWindow(
                QApplication::desktop()->winId(),
                normalizedSelectedArea->x(),
                normalizedSelectedArea->y(),
                normalizedSelectedArea->width(),
                normalizedSelectedArea->height())
    // Resize to fit our screen resolution
    .scaled(LED_WALL_WIDTH, LED_WALL_HEIGHT);

    // Should we update the preview window?
    if(previewWindow->isVisible())
        previewWindow->setImage(screenTexture.scaled(LED_WALL_WIDTH*3, LED_WALL_HEIGHT*3,Qt::KeepAspectRatio));

}

void SelectionAreaWidget::showPreview()
{
    previewWindow->show();
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
