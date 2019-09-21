#include "interval_scanline_zbuffer.h"

#include <QFile>
#include <QStringList>
#include <cmath>
#include <algorithm>

namespace  iszb
{

void edgeTable::initilize(int h)
{
    this->edgeTableColumn = new QVector<edgeItem>[h];
    this->height = h;
}

void edgeTable::clear()
{
    for (int i=0; i<height; i++)
    {
        edgeTableColumn[i].clear();
        edgeTableColumn[i].resize(0);
    }

}


static bool compareEdgeByX(edgeItem e1, edgeItem e2)
{
    return e1.x < e2.x;
}

void activeEdgeList::sort()
{

    std::sort(edgeList.begin(),edgeList.end(),compareEdgeByX);

}

void polygonTable::initilize(int h)
{
    this->polygonTableColumn = new QVector<polygonItem> [h];
    this->height = h;
}

void polygonTable::clear()
{
    for (int i=0; i<this->height; i++)
    {
        this->polygonTableColumn[i].clear();
        this->polygonTableColumn[i].resize(0);
    }
}

polygonItem &activePolygonList::searchPolygon(int id) // find the active polygon in the IPL by its id
{
    for (int i=0; i<polygonList.size(); i++)
    {
        if (this->polygonList[i].id == id)
        {
            return  polygonList[i];
        }
    }
}

void activePolygonList::resetInflag()
{
    for (int i=0; i<this->polygonList.size(); i++)
    {
        polygonList[i].inflag = 0;
    }
}


void interval_scanline_zbuffer::initialize(int left, int top, int width, int height)
{
    ET.initilize(height);
    PT.initilize(height);
    IT.initialize(height);
    this->windowWidth = width;
    this->windowHeight = height;
    this->top = top;
    bottom = top - height + 1;
    this->left = left;
    right = left + width -1;


    this->transformMatrix.scale(1,1,1);
}

void interval_scanline_zbuffer::clear()
{

    this->transformMatrix.setToIdentity();
    this->vertexIdBase = 0;
    this->nVertices= 0;
    this->nFaces =0;
    this->initialVertexArray.clear();
    this->initialVertexArray.resize(0);
    this->FaceArray.clear();
    this->FaceArray.resize(0);
    this->zoomedVertexArray.clear();
    this->zoomedVertexArray.resize(0);

    this->isZoomed = 0;

}

void intervalTable::initialize(int height)
{
    this->intervalColumn = new QVector<interval> [height];
    this->height = height;

}

void intervalTable::clear()
{
    for (int i=0; i<height; i++)
    {
        this->intervalColumn[i].clear();
        this->intervalColumn[i].resize(0);
    }

}


void interval_scanline_zbuffer::loadMatrix(QMatrix4x4 &mat)
{

    transformMatrix = mat;
}

void interval_scanline_zbuffer::zoom()
{

    transformMatrix.setToIdentity();
    float model_center_x = (model_xmax + model_xmin)/2.0f;
    float model_center_y = (model_ymax + model_ymin)/2.0f;
    float dx =  model_xmax - model_xmin;
    float dy =  model_ymax - model_ymin;


    float scalor = this->windowHeight/dx < this->windowHeight/dy ? this->windowHeight/dx: this->windowHeight/dy;
    scalor = scalor*0.8f;
    QMatrix4x4 transMat;

    transMat.translate(-model_center_x*scalor, -model_center_y*scalor);
    transMat.scale(scalor,scalor,scalor);
//    qDebug() <<transMat;
//    qDebug() << scalor;
    QVector3D v;
    zoomedVertexArray.clear();
    for (int i=0; i<initialVertexArray.size(); i++)
    {
         v  = transMat * initialVertexArray[i];
         zoomedVertexArray.push_back(v);
    }
    isZoomed = 1;
}

void interval_scanline_zbuffer::readOBJ(QString filename)
{
    isZoomed = 0;
    QFile file(filename);
    QStringList list;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while (!file.atEnd())
        {
            QString str = file.readLine();


            if (str.length() < 2)
            {
                continue;
            }
            if (str[0] == 'v') //vertices, regardless the normal, the  texture
            {
                str = str.trimmed();
                if(str[1] == ' ') //geometric vertices
                {
                    list = str.split(" ", QString::SkipEmptyParts);

                    QVector3D v = {list[1].toFloat(), list[2].toFloat(),list[3].toFloat()};                
                    if (initialVertexArray.size() == 0)
                    {
                        this->model_xmax = v[0];
                        this->model_xmin = v[0];
                        this->model_ymin = v[1];
                        this->model_ymax = v[1];
                    }
                    else
                    {
                        if ( v[0] > model_xmax)
                        {
                            model_xmax = v[0];
                        }
                        else if (v[0]<model_xmin)
                        {
                            model_xmin = v[0];
                        }

                        if (v[1] > model_ymax)
                        {
                            model_ymax = v[1];
                        }
                        else if(v[1] < model_ymin)
                        {
                             model_ymin = v[1];
                        }
                    }
                    this->initialVertexArray.push_back(v);
                    nVertices++;
                }

            }
            else if (str[0] == 'f')
            {
                str = str.trimmed();

                list = str.split(" ", QString::SkipEmptyParts);
                int v_index;

                face f;
                loop lp;
                f.id= nFaces;

                if (list[1].contains("/"))
                {

                    for(int i=1; i<list.size(); i++)
                    {
                        QStringList sublist = list[i].split('/');
                        v_index = sublist[0].toInt() - 1 + vertexIdBase;
                        lp.lVertices.push_back(v_index);
                    }
                }
                else
                {

                    for (int i=1; i<list.length(); i++)
                    {
                        v_index = list[i].toInt() - 1 + vertexIdBase;
                        lp.lVertices.push_back(v_index);
                    }

                }
                f.floops.push_back(lp);
                this->FaceArray.push_back(f);
                this->nFaces++;
            }

        }

    }
    vertexIdBase = vertexIdBase + nVertices;
    isZoomed = 0;
    file.close();

}


void interval_scanline_zbuffer::readyScan()
{
    int nnnn=0;
    for (int i=0; i<FaceArray.size(); i++)       
    {
         QVector<QVector3D> *currentVertexArray;

        if (isZoomed == 0)
        {
            currentVertexArray = &initialVertexArray;
        }
        else
        {
             currentVertexArray = &zoomedVertexArray;
        }


        QVector4D polyCoeffcient = solvePolygonCoeficient_abcd(*currentVertexArray, transformMatrix , FaceArray[i]);
        QVector3D polynormal = polyCoeffcient.toVector3D();

        /*********************************************/
        /*                                           */
        /*   Acceleration algorithm 1：Face Culling  */
        /*                                           */
        /*********************************************/

        if (polynormal[2] <= 1e-8f)
        {
            continue;
        }
        float polygon_ymax = bottom;
        float polygon_ymin = top;
        int nedgesSelected = 0;
        for (int j=0; j<FaceArray[i].floops[0].lVertices.size(); j++)
        {
            //heigher_v and lower_v are the coordinates of two vertices
            QVector3D higher_v = /*transformMatrix *initialScaleMatrix*/  transformMatrix * (*currentVertexArray)[FaceArray[i].floops[0].lVertices[j]];
            QVector3D lower_v =/* transformMatrix * initialScaleMatrix */ transformMatrix * (*currentVertexArray)[FaceArray[i].floops[0].lVertices[(j+1) % FaceArray[i].floops[0].lVertices.size()]];


            /***************************************************************/
            /*                                                             */
            /*   Acceleration algorithm 2：reject edges outside the window  */
            /*                                                             */
            /***************************************************************/
            //tips: qRound() is from  QT ide

            if (higher_v[1]< lower_v[1])
            {
                QVector3D temp = higher_v;
                higher_v = lower_v;
                lower_v = temp;
            }

            if (qRound(higher_v[1])<=bottom || qRound(lower_v[1])>= top)
            {
                continue;
            }
            else if (qRound(higher_v[1])> top &&  qRound(lower_v[1])< top) // clip the edge cross the top line
            {
                higher_v[0] = (top - lower_v[1])/(higher_v[1]-lower_v[1]) * (higher_v[0]-lower_v[0]) + lower_v[0];
                higher_v[1] = top;
            }
            else if (qRound(higher_v[1])>bottom && qRound(lower_v[1])<bottom)
            {
                lower_v[0] = higher_v[0] + (lower_v[0] - higher_v[0])/ (higher_v[1] - lower_v[1]) * (higher_v[1]-bottom);
                lower_v[1] = bottom;
            }


            if (qRound(higher_v[1]) == qRound(lower_v[1]))//reject horizontal edge
            {
                continue;
            }

            if (higher_v[1]> polygon_ymax)
            {
                polygon_ymax = higher_v[1];
            }
            if (lower_v[1] < polygon_ymin)
            {
                polygon_ymin = lower_v[1];
            }

            edgeItem e;
            e.dx = -( higher_v[0] - lower_v[0]) / (higher_v[1] -  lower_v[1]);
            e.dy = qRound(higher_v[1]) - qRound(lower_v[1]);
            e.x =  higher_v[0]/* + (higher_v[1]-round(higher_v[1])) * e->dx*/;//tips:round() is from <cmath>
            e.id = i;
            // z = -(a * x + b * y + d) / c
            this->ET.edgeTableColumn[top - qRound(higher_v[1])].push_back(e);
            nedgesSelected ++;
        }

        //reject polygons outside the window
        if (abs(polygon_ymax - bottom) <= 1e-10f)
        {
            continue;
        }
        if (abs(polygon_ymin - top) <= 1e-10f)
        {
            continue;
        }
        // reject polygons without edges in the edge tables
        if (nedgesSelected ==0)
        {
            continue;
        }

        polygonItem poly;
        poly.coefficient_abcd = polyCoeffcient;
        poly.diffuse =  polynormal.normalized()[2];/*QVector3D::dotProduct( polynormal.normalized() , QVector3D(0,0,1))*/;
        if (poly.diffuse < 0)
        {
           poly.diffuse = abs(poly.diffuse);
        }

        poly.dy = qRound(polygon_ymax) - qRound(polygon_ymin);
        poly.id = i;
        this->PT.polygonTableColumn[top - qRound(polygon_ymax)].push_back(poly);
        nnnn++;
    }
//    qDebug() << "facesize"<< FaceArray.size();
//    qDebug() <<"  faceused "<< nnnn;

}

void interval_scanline_zbuffer::goScan()
{
    this->IT.clear();
    activeEdgeList AEL;
    activePolygonList APL;

    for (int ihight=0; ihight< windowHeight; ihight++)
    {


        //add new polygons in APL
        for (int ipoly=0; ipoly < PT.polygonTableColumn[ihight].size(); ipoly++)
        {
            APL.polygonList.push_back(PT.polygonTableColumn[ihight][ipoly]);
        }

        APL.resetInflag();

        // add new edges in AEL
        for (int iedge=0; iedge<ET.edgeTableColumn[ihight].size(); iedge++)
        {
            AEL.edgeList.push_back(ET.edgeTableColumn[ihight][iedge]);
        }
        AEL.sort();//sort the new active edge table


        QVector <int> inPolygonIdInCurrentInterval;

        for (int iedge=-1; iedge<AEL.edgeList.size(); iedge++) // iedge == -1 : (left,  firstedge); iedge = AEL.edgeList.size(): (lastedge, right)
        {
            // these case happens at the left and right bounder
            if (iedge == - 1 && AEL.edgeList.size()==0)//in this case there are no polygons between left and right
            {
                interval new_it;
                new_it.x_l = left;
                new_it.x_r = right;
                new_it.id = -1;
                IT.intervalColumn[ihight].push_back(new_it);
                break;
            }
            else if(iedge == -1) //&& AEL.edgeList.size() > 0
            {
                edgeItem activeEdge = AEL.edgeList[0];
                if (round(activeEdge.x) > left)
                {
                    interval new_it;
                    new_it.x_l = left;
                    new_it.x_r = qRound(activeEdge.x);
                    new_it.id = -1;
                    IT.intervalColumn[ihight].push_back(new_it);
                }
                continue;
            }
            else if (iedge == AEL.edgeList.size()-1)
            {
                edgeItem activeEdge = AEL.edgeList[iedge];
//                polygonItem &activePolygon = APL.searchPolygon(activeEdge.id);
                if (round(activeEdge.x) < right)
                {
                    interval new_it;
                    new_it.x_l = qRound( activeEdge.x);
                    new_it.x_r = right;
                    new_it.id = -1;
                    IT.intervalColumn[ihight].push_back(new_it);
                }
                continue;
            };



            //left edge
            edgeItem activeEdge1 = AEL.edgeList[iedge];
            polygonItem &activePolygon1 = APL.searchPolygon(activeEdge1.id);

            if (activePolygon1.inflag == 0)
            {
                activePolygon1.inflag = 1;
                inPolygonIdInCurrentInterval.push_back(activePolygon1.id);

            }
            else
            {
                activePolygon1.inflag = 0;
                for (int i=0; i< inPolygonIdInCurrentInterval.size(); i++)
                {
                    if (inPolygonIdInCurrentInterval[i] == activeEdge1.id)
                    {
                       inPolygonIdInCurrentInterval.erase(inPolygonIdInCurrentInterval.begin() + i);
                       break;
                    }
                }

            }
            //right edge
            edgeItem activeEdge2 = AEL.edgeList[iedge+1];
            if (round(activeEdge2.x)<=left)
            {
                continue;
            }

            // create a new interval
            interval new_interval;
            new_interval.x_l = qRound(activeEdge1.x);
            new_interval.x_r = qRound(activeEdge2.x);
            if (new_interval.x_l == new_interval.x_r)
            {
                continue;
            }


            new_interval.id = -1;
            if (inPolygonIdInCurrentInterval.size() == 0)
            {

                IT.intervalColumn[ihight].push_back(new_interval);
                continue;
            }
            else if (inPolygonIdInCurrentInterval.size() == 1)
            {
                new_interval.id = inPolygonIdInCurrentInterval[0];

            }
            else if (inPolygonIdInCurrentInterval.size() >= 2)
            {
                float zmax = -100000000000000000;

                for (int i=0; i<inPolygonIdInCurrentInterval.size(); i++)        
                {

                    polygonItem temp_poly = APL.searchPolygon(inPolygonIdInCurrentInterval[i]);
                    float tempz = -(qRound(activeEdge1.x) * temp_poly.coefficient_abcd[0] + (top - ihight) *temp_poly.coefficient_abcd[1] +temp_poly.coefficient_abcd[3]) /temp_poly.coefficient_abcd[2];
                    if (tempz > zmax)
                    {
                        zmax = tempz;
                        new_interval.id = inPolygonIdInCurrentInterval[i];
                    }
                }
            }



            if (new_interval.id != -1)
            {
                new_interval.diffuse = APL.searchPolygon(new_interval.id).diffuse;
            }

            IT.intervalColumn[ihight].push_back(new_interval);



        }

        //update old polygons in APL
        for (int ipoly = 0; ipoly<APL.polygonList.size();ipoly++)
        {
            --APL.polygonList[ipoly].dy;
            if (APL.polygonList[ipoly].dy <= 0)
            {
                APL.polygonList.erase(APL.polygonList.begin() + ipoly);
                ipoly--;
            }
        }

        //update old edges in the AEL
        for (int iedge=0; iedge< AEL.edgeList.size(); iedge++)
        {
            AEL.edgeList[iedge].dy --;
            if (  AEL.edgeList[iedge].dy <= 0)
            {

                AEL.edgeList.erase(AEL.edgeList.begin() + iedge);
                iedge--;
            }
            else
            {

                AEL.edgeList[iedge].x += AEL.edgeList[iedge].dx;

                if (qRound(AEL.edgeList[iedge].x) > right && AEL.edgeList[iedge].dx > 0)
                {
                    AEL.edgeList[iedge].x = right;
                    AEL.edgeList[iedge].dx = 0;

                }
                else if (qRound(AEL.edgeList[iedge].x) < left && AEL.edgeList[iedge].dx <0)
                {
                    AEL.edgeList[iedge].x = left;
                    AEL.edgeList[iedge].dx = 0;

                }

//                AEL.edgeList[iedge].z = AEL.edgeList[iedge].z + AEL.edgeList[iedge].dzx * AEL.edgeList[iedge].dx + AEL.edgeList[iedge].dzy;
            }

        }



    }

    PT.clear();
    ET.clear();
}



QVector4D solvePolygonCoeficient_abcd(const QVector<QVector3D> &vertexArray, QMatrix4x4 &mat, const face &poly)
{

    QVector3D v0 = mat * vertexArray[poly.floops[0].lVertices[0]];
    QVector3D v01 = mat * (vertexArray[poly.floops[0].lVertices[1]]) - v0/*mat * (vertexArray[poly.floops[0].lVertices[0]])*/;
    QVector3D v12 = mat * (vertexArray[poly.floops[0].lVertices[2]]) - mat *(vertexArray[poly.floops[0].lVertices[1]]);
//    if (poly.floops[0].lVertices.size() == 3||poly.floops[0].lVertices.size() == 4)
//    {
    // i don't consider concave polygon，maybe will update is in time
        QVector3D abc = QVector3D::crossProduct(v01, v12);
        abc = abc.normalized();
        float d = - QVector3D::dotProduct(abc, v0);
        return QVector4D(abc,d);
//    }


}









}
