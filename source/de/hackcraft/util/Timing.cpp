#include "Timing.h"

#include <iomanip>
#include <iostream>
#include <sstream>

Timing::Timing() {
    setDate(0, 0);
    setTime(0, 0, 0);
    setFPS(30);
}

/*
cTiming::cTiming(const cTiming& orig) {
}

cTiming::~cTiming() {
}
 */

float Timing::getSPF() {
    return mSPF;
}

unsigned int Timing::getDeltacycle() {
    return mDeltacycle;
}

unsigned int Timing::getFrame() {
    return mFrame;
}

void Timing::setFPS(float fps) {
    mFPS = fps;
    mSPF = 1.0f / fps;
}

void Timing::setDate(unsigned int year, unsigned int day) {
    mYear = year;
    mDay = day;
}

void Timing::setTime(unsigned hour, unsigned minute, unsigned second) {
    mHour = hour;
    mMinute = minute;
    mSecond = second;
    mMSec = 0;
    mFrame = 0;
    mDeltacycle = 1;
}

void Timing::advanceDelta() {
    mDeltacycle++;
}

void Timing::advanceTime(int deltamsec) {
    mSPF = float(deltamsec) / 1000.0f;
    mFPS = 1.0f / (mSPF + 0.000001f);
    mFrame++;
    mDeltacycle = 1;
    mMSec += deltamsec;
    if (mMSec >= 1000) {
        mFrame = 0;
        mSecond += mMSec / 1000;
        mMSec %= 1000;
        if (mSecond >= 60) {
            mMinute += mSecond / 60;
            mSecond %= 60;
            if (mMinute >= 60) {
                mHour += mMinute / 60;
                mMinute %= 60;
                if (mHour >= 24) {
                    mDay += mHour / 24;
                    mHour %= 24;
                    if (mDay >= 366) {
                        mYear += mDay / 366;
                        mDay %= 366;
                    }
                }
            }
        }
    }
}

float Timing::getTime24() {
    float time24 = float(mHour) + float(mMinute) / 60.0f + float(mSecond) / (60.0f * 60.0f) + float(mMSec) / (60.0f * 60.0f * 1000.0f);
    return time24;
}

OID Timing::getTimekey() {
    OID ddd = mDay;
    OID hh = mHour;
    OID mm = mMinute;
    OID ss = mSecond;
    OID ff = mFrame;
    OID serid = ((((ddd * 100 + hh) *100 + mm) *100 + ss) * 100 + ff) * 10000 + mDeltacycle;
    return serid;
}

std::string Timing::getDate() {
    std::stringstream s;
    s << mYear << "_" << mDay;
    return s.str();
}

std::string Timing::getTime() {
    std::stringstream s;
    s << mHour;
    s << ":" << std::setfill('0') << std::setw(2) << mMinute;
    s << ":" << std::setfill('0') << std::setw(2) << mSecond;
    return s.str();
}
