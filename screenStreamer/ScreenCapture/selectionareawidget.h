#ifndef SELECTIONAREAWIDGET_H
#define SELECTIONAREAWIDGET_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QDesktopWidget>
#include <QTimer>
#include <QSerialPort>
#include "previewwidget.h"

class SelectionAreaWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit SelectionAreaWidget(QWidget *parent = 0);
    ~SelectionAreaWidget();

private:
    // Selection
    QRect *selectedArea;
    QRect *finalArea;

    // System tray
    QSystemTrayIcon *trayIcon;
    QMenu *contextMenu;
    QAction *quitAction;
    QAction *toggleSelectionAction;
    QAction *showPreviewAction;

    // Screen grabbing
    QRect *normalizedSelectedArea;
    PreviewWidget *previewWindow;

    // Timers
    QTimer *hideWindowTimer;
    QTimer *grabScreenTimer;

    // Serial communication
    QString currentPortName;
    QSerialPort serial;
    QImage *lastFrame;
    void startSerialComunication(QString portName=NULL);
protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent ( QMouseEvent * event );
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

public slots:
    void toggleSelectionArea();
    void quit();
    void hideWindowTimeout();
    void grabScreen();
    void showPreview();
    void updateSerialPorts();
    void serialPortSelected();
};

#endif // SELECTIONAREAWIDGET_H
