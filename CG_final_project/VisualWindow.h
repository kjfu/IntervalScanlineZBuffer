#ifndef VISUALWINDOW_H
#define VISUALWINDOW_H

#include <QWidget>
#include <QString>
#include <QMouseEvent>

#include "interval_scanline_zbuffer.h"

class VisualWindow : public QWidget
{
    Q_OBJECT
public:
    explicit VisualWindow(QWidget *parent = nullptr);

//    iszb::interval_scanline_zbuffer *z_buffer;

    iszb::interval_scanline_zbuffer Z_Buffer;
    void zoom();

    float timeused;
    QString objFileName;
    bool goPaint = 0;
protected:
    void paintEvent(QPaintEvent *event);


    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:

    QCursor cursor;
    QVector3D transVec = QVector3D(0.0f, 0.0f, 0.0f);
    float scale = 1;
    float yaw = 0.0f;
    float pitch = 0.0f;
    float lastX = 0.0f;
    float lastY = 0.0f;
    int width =800;
    int height = 500;
};

#endif // VISUALWINDOW_H
