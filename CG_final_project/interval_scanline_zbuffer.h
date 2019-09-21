#ifndef INTERVAL_SCANLINE_ZBUFFER_H
#define INTERVAL_SCANLINE_ZBUFFER_H

#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QRgb>

namespace  iszb //namespace interval scanline Z-buffer
{



// geometric data
class loop;
class face;

class loop
{
public:
    QVector<int> lVertices;//the list of the vertices's id
};

class face
{
public:
    QVector<loop> floops;// the first loop is the outer loop (if this face has one more loops)
    int id;
};
//geometric data

// interval scanline

class edgeItem;
class polygonItem;
class edgeTable;
class polygonTable;
class activeEdgeList;

class edgeItem //  edge in the sorted edge table
{
public:

    float x;
    float dx;
    int dy;
    int id;
    //tips:
    //   I use the class Qvector aa container of the edge,so i don't need pointer *next
};




class polygonItem // polygon in the sorted polygon table
{
public:
    polygonItem():inflag(0){}
    QVector4D coefficient_abcd;//store the coefficients a b c d in a Qvector4D
    float dy;
    QRgb color;
    int id;
    int inflag;
    float diffuse;
    //tips:
    //   I use the class Qvector as the container of the edge,so i don't need pointer *next
};

class edgeTable
{
public:

    void initilize(int h);
    void clear();
    QVector<edgeItem> *edgeTableColumn;
    int height=0;

};

class polygonTable
{
public:
    void initilize(int h);
    void clear();
    QVector <polygonItem> *polygonTableColumn;
    int height = 0;
};

class activeEdgeList
{
public:
    QVector<edgeItem> edgeList;//contain edgeitem from the edge table
    void sort();// sort edges in the edgelist
};

class activePolygonList
{
public:
    QVector<polygonItem> polygonList;//contain polygonitem from polygon table
    polygonItem &searchPolygon(int id);//find the polygon by its id
    void resetInflag();//reset the inflag of all the polygonitem in the polygonlist
};

class inPolygonList
{
    void erasePolygonId(int id);
    QVector <int> inPolygonIdList;
};

class interval
{
public:
    interval():id(-1){}
    int x_l;//leftside of the interval
    int x_r;//rightside of the interval
    float diffuse;
    int id; //z_l is decieded by polygon id; id = -1 means the background

};

class intervalTable
{
public:
    void initialize(int height);
    void clear();
    QVector<interval> *intervalColumn;// every row in the intervalColumn is a list of intervals

    int height;

};

// interval scanline
class interval_scanline_zbuffer
{
public:

    interval_scanline_zbuffer():vertexIdBase(0), nVertices(0), nFaces(0){}

    //void setWindowSize(int w, int h);
    void initialize(int left, int top,  int width, int height);
    void clear();
    void loadMatrix(QMatrix4x4 &mat);
    void zoom();//move the model in the cencter and fit the window
    void readOBJ(QString filename);
    void readyScan();//Prepare the data structures required by span scanlin z-buffer algorithm
    void goScan();//start span scanline z-buffer: scan lines!

    edgeTable ET;//edges table
    polygonTable PT;// polygon table
    intervalTable IT;// interval table

    QMatrix4x4 transformMatrix;

    QVector<QVector3D> initialVertexArray;
    QVector<QVector3D> zoomedVertexArray;
//    QVector<QVector3D> transformedVertexArray;//the vertex array after model transform firstly and  view transform secondly
    QVector<face> FaceArray;

//private:
    int isZoomed;
    float model_ymax;
    float model_ymin;
    float model_xmax;
    float model_xmin;
    int vertexIdBase;// for read one more models, in this cycle reading file the vertices' ids and faces' ids are based on the last cycle reading fill
    int nVertices ;// the number of vertices
    int nFaces ;//the number of faces, for .obj files the number of loops equals the number of loops
    int windowWidth;
    int windowHeight;
    int top;
    int bottom;
    int left;
    int right;
};

QVector4D solvePolygonCoeficient_abcd(const QVector<QVector3D> &vertexArray, QMatrix4x4 &mat,  const face &poly);

}//namespace iszb
#endif // INTERVAL_SCANLINE_ZBUFFER_H
