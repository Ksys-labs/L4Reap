#include "tscalibr.h"

#include <stdio.h>

TSCalibr::TSCalibr()
  : state(ST_START)
{
    window = new MMSRootWindow("","100%","100%");
    window->setBgColor(MMSFBColor(0x60,0x90,0xc0,0xff));

    connection = window->onBeforeHandleInput->connect(sigc::mem_fun(this, &TSCalibr::myHandleInput));

    r1 = Point(window->getGeometry().w/8, window->getGeometry().h/2);
    r2 = Point(window->getGeometry().w*7/8, window->getGeometry().h/8);
    r3 = Point(window->getGeometry().w*7/8, window->getGeometry().h*7/8);
    
    r1c = Point(window->getGeometry().w/8, window->getGeometry().h/8);
    r2c = Point(window->getGeometry().w*7/8, window->getGeometry().h/2);
    r3c = Point(window->getGeometry().w/8, window->getGeometry().h*7/8);
    
    // extract the label
    label = new MMSLabel(window, "");
    label->setFont("rom","DejaVuSansMono.ttf",14);
    label->setAlignment(MMSALIGNMENT_CENTER);
    label->setColor(MMSFBColor(0xff,0xff,0xff,0xff));
    window->add(label);

    window->show();

    surface = window->getSurface();


    config = new MMSConfigData();

    updateScreen();
}


void TSCalibr::updateScreen()
{
    switch(state)
    {
        case ST_START:
            label->setText("Tap screen to start ...");
            config->setTsMode(true);
            break;

        case ST_TAP1:
            label->setText("Tap cursor...");
            drawCursor(r1);
            break;

        case ST_TAP2:
            p1 = p;
            label->setText("Tap cursor...");
            drawCursor(r2);
            break;

        case ST_TAP3:
            p2 = p;
            label->setText("Tap cursor...");
            drawCursor(r3);
            break;

        case ST_CHECK_START:
            p3 = p;
            calibrate();
            label->setText("Calibration complete. Tap screen to start test ...");
            break;
            
        case ST_CHECK1:
            label->setText("Tap cursor...");
            drawCursor(r1c);
            break;

        case ST_CHECK2:
            p1 = p;
            label->setText("Tap cursor...");
            drawCursor(r2c);
            break;

        case ST_CHECK3:
            p2 = p;
            label->setText("Tap cursor...");
            drawCursor(r3c);
            break;

        case ST_END:
            p3 = p;
            if (checkCalibration())
            {
                label->setText("Calibration complete!");
                config->setTsCalibration(cal_const);
                config->setTsMode(false);
            }
            else
            {
                label->setText("Calibration failed. Tap screen to start again ...");
                state = ST_START;
            }
            break;

        default:
            break;
    }
}

void TSCalibr::drawCursor(const Point& p)
{
    window->waitUntilShown();
    surface->setColor(0xff,0xff,0xff,0xff);
    surface->drawLine(p.x,p.y-10,p.x,p.y+10);
    surface->drawLine(p.x-10,p.y,p.x+10,p.y);
    surface->flip();
}

bool TSCalibr::myHandleInput(MMSWindow *win, MMSInputEvent *inputevent)
{
    if ( (inputevent->type == MMSINPUTEVENTTYPE_TSCALIBRATION) ||
    		(inputevent->type == MMSINPUTEVENTTYPE_BUTTONPRESS) )
    {
        printf("Touchscreen: TAP at %dx%d\n", inputevent->posx, inputevent->posy);
        if (state < ST_END)
        {
            state++;
            p = Point(inputevent->posx, inputevent->posy);
            updateScreen();
        }
        return true;
    }
    else
        return false;
}

bool TSCalibr::calibrate()
{
    float A[3][3];
    float X[3];
    float Y[3];

    float resX[3];
    float resY[3];

    A[0][0] = p1.x; A[1][0] = p1.y; A[2][0] = 1;
    A[0][1] = p2.x; A[1][1] = p2.y; A[2][1] = 1;
    A[0][2] = p3.x; A[1][2] = p3.y; A[2][2] = 1;

    X[0] = r1.x; Y[0] = r1.y;
    X[1] = r2.x; Y[1] = r2.y;
    X[2] = r3.x; Y[2] = r3.y;

    float d = determinant3x3((float*)A);
    if ( abs(d) < 0.001 )
    {
        printf("Error D==0\n");
        return false;
    }

    reverse3x3((float*)A);

    multiple3x3_1x3((float*)A, (float*)X, (float*)resX);

    multiple3x3_1x3((float*)A, (float*)Y, (float*)resY);

    printf("aX=%f, bX=%f, dX=%f\n", resX[0], resX[1], resX[2]);
    printf("aY=%f, bY=%f, dY=%f\n", resY[0], resY[1], resY[2]);

    cal_const = TsCalibration (resX[0], resX[1], resX[2], resY[0], resY[1], resY[2]);

    return true;
}

bool TSCalibr::checkCalibration()
{

    bool res1 = checkPoint(toPixelPoint(p1), r1c);
    bool res2 = checkPoint(toPixelPoint(p2), r2c);
    bool res3 = checkPoint(toPixelPoint(p3), r3c);
    
    return (res1 && res2 && res3);
}

Point TSCalibr::toPixelPoint(const Point& a)
{
    Point res;

    res.x = (float)a.x*cal_const.aX + (float)a.y*cal_const.bX + cal_const.dX;
    res.y = (float)a.x*cal_const.aY + (float)a.y*cal_const.bY + cal_const.dY;

    return res;
}

bool TSCalibr::checkPoint(const Point& a, const Point& b)
{
    printf("a'(%d,%d) a(%d,%d)\n", a.x, a.y, b.x, b.y);
    return ( (abs(a.x - b.x) <= 5) && (abs(a.y - b.y) <= 5) );
}

#define IDX(x,y) ((y)+(3*(x)))

float TSCalibr::determinant3x3(float* m)
{
    float res = 0;

    // 1
    res = m[IDX(0,0)]*(m[IDX(1,1)]*m[IDX(2,2)] - m[IDX(2,1)]*m[IDX(1,2)]);
    // 2
    res -= m[IDX(1,0)]*(m[IDX(0,1)]*m[IDX(2,2)] - m[IDX(2,1)]*m[IDX(0,2)]);
    // 3
    res += m[IDX(2,0)]*(m[IDX(0,1)]*m[IDX(1,2)] - m[IDX(1,1)]*m[IDX(0,2)]);

    return res;
}

void TSCalibr::reverse3x3(float* m)
{
    float eA[6][3];
    float str[6];

    // step 1
    eA[0][0] = m[IDX(0,0)]; eA[1][0] = m[IDX(1,0)]; eA[2][0] = m[IDX(2,0)]; eA[3][0] = 1; eA[4][0] = 0; eA[5][0] = 0;
    eA[0][1] = m[IDX(0,1)]; eA[1][1] = m[IDX(1,1)]; eA[2][1] = m[IDX(2,1)]; eA[3][1] = 0; eA[4][1] = 1; eA[5][1] = 0;
    eA[0][2] = m[IDX(0,2)]; eA[1][2] = m[IDX(1,2)]; eA[2][2] = m[IDX(2,2)]; eA[3][2] = 0; eA[4][2] = 0; eA[5][2] = 1;

    // step 2
    divstr6((float*)eA,0,eA[0][0]);

    // step 3
    mulstr6((float*)eA,0,eA[0][1],str);
    substr6((float*)eA,1,str);

    // step 4
    mulstr6((float*)eA,0,eA[0][2],str);
    substr6((float*)eA,2,str);

    // step 5
    divstr6((float*)eA,1,eA[1][1]);

    // step 6
    mulstr6((float*)eA,1,eA[1][2],str);
    substr6((float*)eA,2,str);

    // step 7
    divstr6((float*)eA,2,eA[2][2]);

    // step 8
    mulstr6((float*)eA,2,eA[2][1],str);
    substr6((float*)eA,1,str);

    // step 9
    mulstr6((float*)eA,2,eA[2][0],str);
    substr6((float*)eA,0,str);

    // step 10
    mulstr6((float*)eA,1,eA[1][0],str);
    substr6((float*)eA,0,str);

    m[IDX(0,0)] = eA[3][0]; m[IDX(1,0)] = eA[4][0]; m[IDX(2,0)] = eA[5][0];
    m[IDX(0,1)] = eA[3][1]; m[IDX(1,1)] = eA[4][1]; m[IDX(2,1)] = eA[5][1];
    m[IDX(0,2)] = eA[3][2]; m[IDX(1,2)] = eA[4][2]; m[IDX(2,2)] = eA[5][2];
}

void TSCalibr::multiple3x3_1x3(float* a, float *b, float *res)
{
    int i,j;
    for(i = 0; i < 3; i++)
    {
        res[i] = 0;
        for (j=0; j < 3; ++j)
            res[i] += (a[IDX(j,i)]*b[j]);
    }
}

void TSCalibr::divstr6(float* m, int str, float v)
{
    int i;
    for (i = 0; i < 6; i++)
        m[IDX(i,str)] /= v;
}

void TSCalibr::mulstr6(float* m, int str, float v, float* resstr)
{
    int i;
    for (i = 0; i < 6; i++)
        resstr[i] = m[IDX(i,str)] * v;
}

void TSCalibr::substr6(float* m, int str, float* v)
{
    int i;
    for (i = 0; i < 6; i++)
        m[IDX(i,str)] -= v[i];
}

