#include "VisualWindow.h"

#include <QPainter>
#include <QPixmap>

#include <QTime>
VisualWindow::VisualWindow(QWidget *parent) : QWidget(parent)
{

    resize(500,300);
    updatesEnabled();

    Z_Buffer.initialize(-width/2, height/2, width, height);



}

void VisualWindow::zoom()
{


    Z_Buffer.zoom();
    transVec = QVector3D(0.0f, 0.0f, 0.0f);
    scale = 1;
    yaw = 0.0f;
    pitch = 0.0f;
    lastX = 0.0f;
    lastY = 0.0f;

}



void VisualWindow::paintEvent(QPaintEvent *event)
{
//    while (goPaint == 0)
//    {

//    }

    QPainter painter(this);
    painter.setWindow(0, 0, width, height);
    painter.setViewport(width/2, height/2, width, -height);;

    painter.setPen(QColor(0,0,0));
    painter.drawRect(-width/2,-height/2, width-1, height-1);

    QMatrix4x4 model;


    model.translate(transVec);
    model.rotate(pitch, QVector3D(1.0f, 0.0f, 0.0f));
    model.rotate(yaw, QVector3D(0.0f, 1.0f, 0.0f));
    model.scale(scale, scale, scale);


    if (goPaint==1)
    {

        QTime t;
        t.restart();
        Z_Buffer.loadMatrix(model);
        Z_Buffer.readyScan();
        Z_Buffer.goScan();
        timeused = t.elapsed() * 1000.0f;


        for (int i=0; i<this->height; i++)
        {
            for (int j=0; j< Z_Buffer.IT.intervalColumn[i].size(); j++)
            {
                if (Z_Buffer.IT.intervalColumn[i][j].id == -1)
                {
                    painter.setPen(QColor(150,150,150));
                    painter.drawLine(QPoint(Z_Buffer.IT.intervalColumn[i][j].x_l, this->height/2-i), QPoint(Z_Buffer.IT.intervalColumn[i][j].x_r, this->height/2-i));

                }
                if(Z_Buffer.IT.intervalColumn[i][j].id >=0)
                {
                    painter.setPen(QColor(qRound(200+250.0*Z_Buffer.IT.intervalColumn[i][j].diffuse)/5,200+qRound(250.0*Z_Buffer.IT.intervalColumn[i][j].diffuse)/5, 200 + qRound(250.0*Z_Buffer.IT.intervalColumn[i][j].diffuse)/5));
                    painter.drawLine(QPoint(Z_Buffer.IT.intervalColumn[i][j].x_l, this->height/2-i), QPoint(Z_Buffer.IT.intervalColumn[i][j].x_r, this->height/2-i));
                }
                painter.setPen(QColor(0,0,0));
               // painter.drawPoint(QPoint(Z_Buffer.IT.intervalColumn[i][j].x_l, this->height/2-i));

            }

        }
//        goPaint = 0;
    }
    painter.setPen(QColor(255,0,0));



//    painter.drawLine(model*v1.toPoint(), model*v2.toPoint());


//     update();
     //QPixmap scanlin(500,300);
    //painter.begin(&scanlin);
    //painter.drawRect(0,0,500,300);
    //painter.drawLine(QPoint(-250,150), QPoint(250,150));
    //painter.drawLine(QPoint(-250,150), QPoint(-250,150));
    //painter.drawLine(QPoint(2,1),QPoint(2,100));

    //painter.drawLine(QPoint(3,1),QPoint(3,100));
    //painter.drawLine(QPoint(5,1),QPoint(5,100));
   // painter.end();

}

void VisualWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        //
        cursor.setShape(Qt::ClosedHandCursor);
        setCursor(cursor);

        // Calculate the change value of yaw and pitch
        float xOffset = event->x() - lastX;
        float yOffest = event->y() - lastY;
        lastX = event->x();
        lastY = event->y();

        float sensitivity = 0.4f;
        xOffset *= sensitivity;
        yOffest *= sensitivity;

        yaw += xOffset;
        pitch += yOffest;

        if (pitch > 89.0f)
        {
            pitch = 89.0f;
        }
        if (pitch < -89.0f)
        {
            pitch = -89.0f;
        }
    }
    else if (event->buttons() == Qt::RightButton)
    {
        float xOffset = event->x() - lastX;
        float yOffest = event->y() - lastY;
        lastX = event->x();
        lastY = event->y();


        float sensitivity =/* 0.01f;*/ 1;
        xOffset *= sensitivity;
        yOffest *= sensitivity;

        transVec += QVector3D(xOffset, -yOffest, 0);
    }

    update();
}

void VisualWindow::wheelEvent(QWheelEvent *event)
{
    float sensitivity = 0.0005f;
    scale *= 1.0f - event->delta() * sensitivity;

    update();
}

void VisualWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        cursor.setShape(Qt::PointingHandCursor);
        setCursor(cursor);
    }
    else if (event->button() == Qt::RightButton)
    {
        cursor.setShape(Qt::SizeAllCursor);
        setCursor(cursor);
    }

    lastX = event->x();
    lastY = event->y();

}

void VisualWindow::mouseReleaseEvent(QMouseEvent *event)
{
    cursor.setShape(Qt::ArrowCursor);
    setCursor(cursor);

    update();
}
