#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QLabel>

class PreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = 0);
    void setImage(QPixmap img);
private:
    QLabel *imageLabel;
signals:
    
public slots:
    
};

#endif // PREVIEWWIDGET_H
