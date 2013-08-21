#include "selectionareawidget.h"
#include <QRegion>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QSettings>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>

#define FPS 1000/24
#define LED_WALL_WIDTH 24
#define LED_WALL_HEIGHT 16

SelectionAreaWidget::SelectionAreaWidget(QWidget *parent) :
    QWidget(parent)
{
    /******
     * App settings
     **/
    QSettings settings("commit", "led_wall");

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
    //
    QMenu* serialPortsMenu = contextMenu->addMenu("Serial Port");
    connect(serialPortsMenu, SIGNAL(aboutToShow()), this, SLOT(updateSerialPorts()));
    trayIcon->setContextMenu(contextMenu);
    //
    contextMenu->addAction(quitAction);

    /*****
     * Selection Area
     **/
    QDesktopWidget desktop;
    QRect storedValue = settings.value("selectedArea", QRect(0, 0, desktop.width(), desktop.height())).toRect();
    selectedArea = new QRect(storedValue.topLeft(), storedValue.bottomRight());
    hideWindowTimer = new QTimer(this);
    hideWindowTimer->setSingleShot(true);
    connect(hideWindowTimer, SIGNAL(timeout()), this, SLOT(hideWindowTimeout()));

    // Window setup
    resize(desktop.width(), desktop.height());
    setMask(QRegion(0, 0, desktop.width(), desktop.height()));
    setStyleSheet("background:transparent;");
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);

    /****
     * screen grabbing
     **/
    normalizedSelectedArea = new QRect(selectedArea->x(), selectedArea->y(), selectedArea->width(), selectedArea->height());
    grabScreenTimer = new QTimer(this);
    connect(grabScreenTimer, SIGNAL(timeout()), this, SLOT(grabScreen()));
    grabScreenTimer->start(FPS);

    /****
     * Preview
     **/
    previewWindow = new PreviewWidget();
    previewWindow->resize(LED_WALL_WIDTH * 6, LED_WALL_HEIGHT * 6);

    /****
     * Comunication
     **/
    startSerialComunication();
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
    normalizedSelectedArea->setRect(x, y, std::abs(selectedArea->width()), std::abs(selectedArea->height()));

    // Save value
    QSettings settings("commit", "led_wall");
    settings.setValue("selectedArea", *normalizedSelectedArea);

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

/****
 * Serial
 **/
void SelectionAreaWidget::serialPortSelected()
{
    QAction* action = (QAction*) QObject::sender();
    startSerialComunication(action->text());
}

void SelectionAreaWidget::startSerialComunication(QString portName)
{
    // select the best port if none was given
    if(portName == NULL)
    foreach (const QSerialPortInfo &serialPort, QSerialPortInfo::availablePorts()) {
        if(serialPort.portName().toLower().indexOf("usb") >= 0 ||
           serialPort.portName().toLower().indexOf("tty") >= 0 ||
           serialPort.portName().toLower().indexOf("com") >= 0
        ){
            portName = serialPort.portName();
            break;
        }
    }

    if(portName == NULL || currentPortName == portName) return;

    serial.close();
    serial.setPortName(portName);
    if (!serial.open(QIODevice::ReadWrite)) {
        QMessageBox msg;
        msg.setText(tr("Can't open %1, error code %2")
                    .arg(serial.portName()).arg(serial.error()));
        msg.exec();
        return;
    }

    if (!serial.setBaudRate(QSerialPort::Baud9600)) {
        QMessageBox msg;
        msg.setText(tr("Can't set rate 9600 baud to port %1, error code %2")
                     .arg(serial.portName()).arg(serial.error()));
        msg.exec();
        return;
    }

    currentPortName = portName;
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
void SelectionAreaWidget::updateSerialPorts()
{
    QMenu *serialPortMenu = (QMenu*) QObject::sender();
    serialPortMenu->clear();

    // Add all serial ports to the list
    foreach (const QSerialPortInfo &serialPort, QSerialPortInfo::availablePorts()) {
        QAction *action;
        if(currentPortName == serialPort.portName())
            action = new QAction(QIcon(":/check.png"), serialPort.portName(), this);
        else
            action = new QAction(serialPort.portName(), this);

        connect(action, SIGNAL(triggered()), this, SLOT(serialPortSelected()));
        serialPortMenu->addAction(action);
    }

}

SelectionAreaWidget::~SelectionAreaWidget()
{
    trayIcon->setVisible(false);
    delete trayIcon;
}
