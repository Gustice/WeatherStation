/**
 * @file MyDisplay.cpp
 * @author Gustice
 * @brief UI-Implementation for certain usecase.
 * @details The UI is designed to fit on a 128x68 OLED display. 
 *  The upper half is for status report the lower half is for log messages.
 * @bug This ui builds only once, the cursor can not be modified properly after that.
 * @version 0.1
 * @date 2021-01-01
 * 
 * @copyright Copyright (c) 2021
 */

#include "MyDisplay.h"
#include <math.h>
#include "esp_log.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

const char * TAG = "display";

const size_t SymbolWidth = 12;
const size_t SymbolHeight = 16;
const size_t TextHeight = 8;

const size_t DisplayTitleHeight = 16;
const size_t DisplayWidth = 128;
const size_t DisplayHeight = 64;

const size_t LogRange = DisplayTitleHeight + 2* TextHeight;

const int NumElements = 8;
const int ElementWidth = 30;
const int ElementHeight = 16;

// Display layout - OLED-128x64
// First 2xRow is for title and symbols.
// Next 6xRows for Status messages.
// Each Row has a height of 8 pixels

MyDisplay::MyDisplay(SSD1306 * display){
    _display = display;
    _display->Init(SSD1306_SWITCHCAPVCC);
    _display->clearDisplay();

    memset(log,0,sizeof(log));
    logLine = 0;
}


void MyDisplay::PrintInitFrame(bool wifiOk, bool supply, float battery)
{
    NewImage();
    _display->setCursor(0,0);
    _display->setTextSize(2);
    _display->setTextColor(WHITE);
    _display->print("T/H-Sensor");
    // Clear space for symbols
    _display->fillRect(DisplayWidth-3*SymbolWidth, 0, 3*SymbolWidth,SymbolHeight, BLACK);

    point_t p1 = {DisplayWidth-SymbolWidth ,0};
    printBattery(p1, 50);
    
    point_t p2 = {DisplayWidth-2*SymbolWidth ,0};
    if(wifiOk)
        printConnectSymbol(p2);
    else
        printWarningSymbol(p2);
    
    point_t p3 = {DisplayWidth-3*SymbolWidth ,0};
    if(supply)
        printPowerSymbol(p3);

    _display->setTextSize(1);
    _display->setCursor(0,DisplayTitleHeight);
    _display->print("T = 000.0 °C ");
    _display->setCursor(0,DisplayTitleHeight+TextHeight);
    _display->print("RH= 000.0 %");

    _display->setCursor(0,LogRange);
    _display->print("Line 1");
    _display->setCursor(0,LogRange+TextHeight);
    _display->print("Line 2");
    _display->setCursor(0,LogRange+TextHeight*2);
    _display->print("Line 2");
    _display->setCursor(0,LogRange+TextHeight*3);
    _display->print("Line 3");
    Display();
}

void MyDisplay::PrintLogLine(char * line)
{
    // int len = MIN(24u, strlen(line));
    // _display->setTextColor(WHITE);

    // // if (logLine >= LogLines-1)
    // // {
    // //     _display->fillRect(0, LogRange, DisplayWidth,DisplayHeight-LogRange, BLACK);
    // //     logLine = LogLines-1;
    // //     for (size_t i = 0; i < LogLines-2; i++)
    // //     {
    // //         memcpy(log[logLine], line, len);
    // //         log[logLine][len] = 0; // Terminate string
    // //         _display->setCursor(0,LogRange + i*TextHeight);
    // //         _display->print(log[logLine]);
    // //         logLine++;
    // //     }

    // //     memcpy(log[logLine], line, len);
    // //     _display->print(log[logLine]);
    // // }
    // // else
    // // {
    //     memcpy(log[logLine], line, len);
    //     _display->setCursor(0,LogRange + logLine*TextHeight);
    //     _display->print(log[logLine]);
    //     ESP_LOGI(TAG, "len=%d, line=%d, cursor=%d", len, logLine, _display->getCursorY() );
    //     logLine++;
    // // }

    // Display();
}



void MyDisplay::printBattery(point_t anchor, uint32_t full)
{
    const uint16_t p = 1;     // padding
    const uint16_t sw = SymbolWidth-2*p; // Symbol Width
    const uint16_t sh = SymbolHeight-2*p; // Symbol Height
    const uint16_t fw = sw-2; // fill Width
        
    _display->drawRect(anchor.x+4, anchor.y+p, 4, 2, WHITE); // draw Hat
    _display->drawRect(anchor.x+p, anchor.y+3, sw, 12, WHITE); // draw Frame
    
    if (full < 10)
        return;
    _display->drawRect(anchor.x+p+1, anchor.y+12, fw, 2, WHITE); //  20%

    if (full < 30)
        return;
    _display->drawRect(anchor.x+p+1, anchor.y+10, fw, 2, WHITE); //  40%
    
    if (full < 50)
        return;
    _display->drawRect(anchor.x+p+1, anchor.y+8, fw, 2, WHITE); //  60%
    
    if (full < 70)
        return;
    _display->drawRect(anchor.x+p+1, anchor.y+6, fw, 2, WHITE); //  80%

    if (full < 90)
        return;
    _display->drawRect(anchor.x+p+1, anchor.y+4, fw, 2, WHITE); // 100%
}

void MyDisplay::printConnectSymbol(point_t anchor){

    _display->drawRect(anchor.x+2, anchor.y+9, 2, 4, WHITE);
    _display->drawRect(anchor.x+4, anchor.y+6, 2, 7, WHITE);
    _display->drawRect(anchor.x+6, anchor.y+3, 2, 10, WHITE);
}

void MyDisplay::printPowerSymbol(point_t anchor){
 _display->drawRect(anchor.x+2, anchor.y+5, 7, 4, WHITE);
 _display->drawRect(anchor.x+3, anchor.y+6, 5, 4, WHITE);
 _display->drawRect(anchor.x+4, anchor.y+7, 3, 4, WHITE);
 _display->drawLine(anchor.x+3, anchor.y+2, anchor.x+3, anchor.y+5, WHITE); 
 _display->drawLine(anchor.x+7, anchor.y+2, anchor.x+7, anchor.y+5, WHITE); 
 _display->drawLine(anchor.x+5, anchor.y+10, anchor.x+5, anchor.y+13, WHITE); 
}

void MyDisplay::printWarningSymbol(point_t anchor){
     _display->drawRect(anchor.x+4, anchor.y+2, 4, 5, WHITE);
    _display->drawRect(anchor.x+5, anchor.y+1, 2, 9, WHITE);
    _display->drawRect(anchor.x+4, anchor.y+12, 4, 2, WHITE);
    _display->drawRect(anchor.x+5, anchor.y+11, 2, 4, WHITE);
}

