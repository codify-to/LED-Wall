#ifndef SELECTIONAREAWIDGET_H
#define SELECTIONAREAWIDGET_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QDesktopWidget>
#include <QTimer>
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
};

#endif // SELECTIONAREAWIDGET_H
