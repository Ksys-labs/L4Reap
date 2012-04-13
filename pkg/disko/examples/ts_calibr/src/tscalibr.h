#ifndef _TS_CALIBR_H_
#define _TS_CALIBR_H_

#include <disko.h>
#include <list>


struct Point
{
    Point(int xpos = 0, int ypos = 0):x(xpos),y(ypos){};

    int x,y;
};


class TSCalibr {
public:
    TSCalibr();
    ~TSCalibr();

private:
    void updateScreen();
    bool calibrate();
    bool checkCalibration();

    bool checkPoint(const Point& a, const Point& b);
    Point toPixelPoint(const Point& a);

    void drawCursor(const Point& p);

    float determinant3x3(float* m);
    void reverse3x3(float* m);
    void multiple3x3_1x3(float* a, float *b, float *res);
    void divstr6(float* m, int str, float v);
    void mulstr6(float* m, int str, float v, float* resstr);
    void substr6(float* m, int str, float* v);

private:
    enum State {
        ST_START,
        ST_TAP1,
        ST_TAP2,
        ST_TAP3,
        ST_CHECK_START,
        ST_CHECK1,
        ST_CHECK2,
        ST_CHECK3,
        ST_END
    };

    bool myHandleInput(MMSWindow *win, MMSInputEvent *inputevent);

private:
    MMSWindow           *window;
    MMSLabel            *label;

    MMSConfigData       *config;

    unsigned            state;
    MMSFBSurface        *surface;

    sigc::connection    connection;

    Point               p1, p2, p3, p;
    Point               r1, r2, r3;
    Point               r1c, r2c, r3c;

    TsCalibration       cal_const;
};

#endif // _TS_CALIBR_H_
