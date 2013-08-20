#include "previewwidget.h"
#include <QVBoxLayout>

PreviewWidget::PreviewWidget(QWidget *parent) :
    QWidget(parent)
{
    imageLabel = new QLabel(this);
    imageLabel->setScaledContents(true);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(imageLabel, 1);
    setLayout(layout);

//    imageLabel->setStyleSheet("background:red");
}

void PreviewWidget::setImage(QPixmap img)
{
    imageLabel->setPixmap(img);
}
