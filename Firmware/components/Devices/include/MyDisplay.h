/**
 * @file MyDisplay.h
 * @author Gustice
 * @brief 
 * @version 0.1
 * @date 2021-01-01
 * 
 * @copyright Copyright (c) 2021
 */

#pragma once 

#include "Display_SSD1306.h"

typedef struct point_def
{
    uint16_t x;
    uint16_t y;
} point_t;


class MyDisplay
{
public:
    MyDisplay(SSD1306 * display);
    ~MyDisplay(){}

    void Display(void) {_display->display();}
    void NewImage(void) {_display->clearDisplay();}

    void PrintInitFrame(bool wifiOk, bool supply, float battery);
    void PrintLogLine(char * line);

private:
    SSD1306 * _display;
    void printBattery(point_t anchor, uint32_t full);
    void printConnectSymbol(point_t anchor);
    void printPowerSymbol(point_t anchor);
    void printWarningSymbol(point_t anchor);

    static const size_t LogLines = 4;
    static const size_t LogLineChars = 25; // Each line contains max 24 characters
    char log[LogLines][LogLineChars]; 

    int logLine = 0;
};

