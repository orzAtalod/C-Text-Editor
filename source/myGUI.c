#include "myGUI.h"
#include "imgui.h"
#include "extgraph.h"
#include "genlib.h"
#include "simpio.h"
#include "graphics.h"
#include <time.h>
#include"boolean.h"
#include <windows.h>
#include <winuser.h>

////////////////////////////////////// 声明和常量定义 ////////////////////////////////////////////

// 宏定义
#define TITLE_HEIGHT 0             // 标题区高度
#define MENU_HEIGHT 0.3            // 菜单区高度
#define TOOL_HEIGHT 0.5            // 工具区高度
#define BUTTON_WIDTH 1.0           // 按钮宽度
#define BUTTON_GAP 0.05            // 按钮之间的间隙
#define SCROLLBAR_WIDTH 0.1        // 滚动条宽度
#define WINDOW_MARGIN 0            // 窗口边缘的间隙
#define INPUT_BAR_HEIGHT 0.3

static AreaPaintMethod explorerDraw                   = NULL;
static AreaPaintMethod editorGraphicDraw              = NULL;
static GetInfoMethod   rollerUpperBoundDraw           = NULL;
static GetInfoMethod   rollerLowerBoundDraw           = NULL;
static SetValueMethod  setRollerHeight                = NULL;
static SetValueMethod  setEditorWidth                 = NULL;

static ButtonEvent          editorMouseMiddleRollup   = NULL;
static ButtonEvent          editorMouseMiddleRolldown = NULL;
static KeyboardEventMethod  editorKeyboard            = NULL;
static KeyboardEventMethod  editorKeyboardSpecial     = NULL;
static ButtonEvent          imageButton               = NULL;
static ButtonEvent          italicButton              = NULL;
static ButtonEvent          boldButton                = NULL;

static ButtonEvent FileMenuMethod[5]  = {};
static ButtonEvent ToolsMenuMethod[3] = {};
static ButtonEvent AboutMenuMethod[2] = {};    // 定义菜单项
static char *fileMenu[]  = {"File", "Save", "Open", "New", "Save As", "Close"};
static char *toolsMenu[] = {"Tools", "Stats", "Search", "Browse"};
static char *aboutMenu[] = {"About", "Settings", "Help"};

static char lastSelectedColor[32]                     = "Red"; // 存储快捷颜色
static ButtonEventWithInput colorButton               = NULL;
static ButtonEventWithInput sizeButton                = NULL;
static ButtonEventWithInput fontButton                = NULL;

static int inputMode                               = 0;      // 输入模式标志位
static ButtonEventWithInput inputConfirmedCallback = NULL;
static char inputBuffer[256];                                // 输入栏缓冲区
static int inputBufferLength                       = 0;
static char majorInputMessage[256];                          // 弹窗输入提示消息
static int keyboardConfirmed                       = 0;
static int keyboardCancelled                       = 0;

static AreaPaintMethod searchResultDraw = NULL;
static AreaPaintMethod statDisplayDraw  = NULL;               // 用于绘制统计面板

static ColorDefinitionMethod defineColor = NULL;

// 存储快捷键映射
static struct {
    char keyCombo[5];
    ButtonEvent callback;
} hotKeys[100];
static int hotKeyCount = 0;

// 存储时间
static clock_t startTime;
long GetClock() {
    clock_t currentTime = clock();
    return (long)((double)(currentTime - startTime) / CLOCKS_PER_SEC * 1000);
}

////////////////////////////////////////////////// 绘制逻辑 ///////////////////////////////////////////
static void DrawMenuBar_(double x, double y, double width, double height) {
    // 绘制菜单栏背景（仅边框，不填充）
    SetPenColor("Black");
    drawRectangle(x, y, width, height, 0);

    // 绘制菜单
    const int choice1 = menuList(GenUIID(0), x             + BUTTON_GAP, y, width/3-2*BUTTON_GAP, width/3-2*BUTTON_GAP, height-2*BUTTON_GAP, fileMenu,  sizeof(fileMenu) /sizeof(fileMenu[0]));
    const int choice2 = menuList(GenUIID(1), x + width/3   + BUTTON_GAP, y, width/3-2*BUTTON_GAP, width/3-2*BUTTON_GAP, height-2*BUTTON_GAP, toolsMenu, sizeof(toolsMenu)/sizeof(toolsMenu[0]));
    const int choice3 = menuList(GenUIID(2), x + 2*width/3 + BUTTON_GAP, y, width/3-2*BUTTON_GAP, width/3-2*BUTTON_GAP, height-2*BUTTON_GAP, aboutMenu, sizeof(aboutMenu)/sizeof(aboutMenu[0]));
    if(choice1>0) { ChangeDisplayMethodToMain(); FileMenuMethod[choice1-1]();  }
    if(choice2>0) { ChangeDisplayMethodToMain(); ToolsMenuMethod[choice1-1](); }
    if(choice3>0) { ChangeDisplayMethodToMain(); AboutMenuMethod[choice1-1](); }
}

void colorInputCallback(const char* color) {
    if (color && strcmp(color, "Black")!=0 && strcmp(color, "")!=0) {
        strncpy(lastSelectedColor, color, sizeof(lastSelectedColor));
    }
    colorButton(color);
}

void DrawToolBar(double x, double y, double width, double height) {
    // 绘制工具栏背景（仅边框，不填充）
    SetPenColor("Black");
    drawRectangle(x, y, width, height, 0);

    // 定义按钮位置和属性
    double buttonX = x + BUTTON_GAP;
    double buttonY = y + BUTTON_GAP;
    double buttonWidth = BUTTON_WIDTH;
    double buttonHeight = height - 2 * BUTTON_GAP;

    // 绘制每一个按钮
    SetPenColor("Black");  // 设置默认颜色防止边框受影响
    int buttonId = GenUIID(0);
    if (button(buttonId++, buttonX, buttonY, buttonWidth, buttonHeight, "Color")) {
        ChangeDisplayMethodToInput(colorInputCallback);
    }
    if (button(buttonId++, buttonX + (buttonWidth + BUTTON_GAP), buttonY, buttonWidth, buttonHeight, lastSelectedColor)) {
        ChangeDisplayMethodToMain();
        colorButton(lastSelectedColor);
    }
    if (button(buttonId++, buttonX + 2 * (buttonWidth + BUTTON_GAP), buttonY, buttonWidth, buttonHeight, "Bold")) {
        ChangeDisplayMethodToMain();
        boldButton();
    }
    if (button(buttonId++, buttonX + 3 * (buttonWidth + BUTTON_GAP), buttonY, buttonWidth, buttonHeight, "Italic")) {
        ChangeDisplayMethodToMain();
        italicButton();
    }
    if (button(buttonId++, buttonX + 4 * (buttonWidth + BUTTON_GAP), buttonY, buttonWidth, buttonHeight, "Size")) {
        ChangeDisplayMethodToInput(sizeButton);
    }
    if (button(buttonId++, buttonX + 5 * (buttonWidth + BUTTON_GAP), buttonY, buttonWidth, buttonHeight, "Font")) {
        ChangeDisplayMethodToInput(fontButton);
    }
    if (button(buttonId++, buttonX + 6 * (buttonWidth + BUTTON_GAP), buttonY, buttonWidth, buttonHeight, "Image")) {
        ChangeDisplayMethodToMain();
        imageButton();
    }
}

void DrawMainArea(double x, double y, double width, double height) {
    // 计算各部分的位置和尺寸
    double browserWidth = (width / 3.0);
    double editorWidth = (2.0 * width / 3.0) - BUTTON_GAP - SCROLLBAR_WIDTH;
    double scrollbarX = x + (width - SCROLLBAR_WIDTH);

    // 绘制浏览区（仅边框，不填充）
    SetPenColor("Black");
    drawRectangle(x, y, browserWidth, height, 0);
    if (explorerDraw) {
        explorerDraw(x, y, browserWidth, height);
    } else {
        drawLabel(x + 0.1, y + height - 0.2, "Browser Area");
    }

    // 绘制编辑区（仅边框，不填充）
    SetPenColor("Black");
    drawRectangle(x + browserWidth + BUTTON_GAP, y, editorWidth, height, 0);
    if (editorGraphicDraw) {
        editorGraphicDraw(x + browserWidth + BUTTON_GAP, y, editorWidth, height);
    } else {
        drawLabel(x + browserWidth + BUTTON_GAP + 0.1, y + height - 0.2, "Editing Area");
    }

    // 绘制滚动条（仅边框，不填充）
    SetPenColor("Black");
    drawRectangle(scrollbarX, y, SCROLLBAR_WIDTH, height, 0);

    if (rollerUpperBoundDraw && rollerLowerBoundDraw) {
        const double upperBound = rollerUpperBoundDraw();
        const double lowerBound = rollerLowerBoundDraw();
        const double topY = y + height * (1 - upperBound);
        const double bottomY = y + height * (1 - lowerBound);
        drawRectangle(scrollbarX, bottomY, SCROLLBAR_WIDTH, topY - bottomY, 1);  // 填充滚动条粗段
    } else {
        drawLabel(scrollbarX + 0.1, y + height - 0.2, "Scrollbar");
    }
}

void DrawInputBar(double x, double y, double width, double height) {
    SetPenColor("Black");
    drawRectangle(x, y, width, height, 0);
    
    drawLabel(x + 0.1, y + height / 2 - 0.1, inputMode==1 ? "Input: " : "Search: ");
    drawLabel(x + 1.0, y + height / 2 - 0.1, inputBuffer);

    // 确认按钮
    if (button(GenUIID(0), x + width - 2 * BUTTON_WIDTH - 2 * BUTTON_GAP, y + BUTTON_GAP, BUTTON_WIDTH, height - 2 * BUTTON_GAP, "确认") || keyboardConfirmed) {
        keyboardConfirmed = 0;
        if (inputMode == 1) {
            if (inputConfirmedCallback) {
                inputConfirmedCallback(inputBuffer);
                inputConfirmedCallback = NULL; // 防止再次传入
            }
            ChangeDisplayMethodToMain();
        } // 搜索模式下确认按钮无效果
    }

    // 退出按钮
    if (button(GenUIID(1), x + width - BUTTON_WIDTH - BUTTON_GAP, y + BUTTON_GAP, BUTTON_WIDTH, height - 2 * BUTTON_GAP, "退出") || keyboardCancelled) {
        keyboardCancelled = 0;
        ChangeDisplayMethodToMain();
    }
}

void DrawSettingPage(double x, double y, double width, double height) {
	typedef struct {
	    char colorName[32];
	    char red[32];
	    char green[32];
	    char blue[32];
	    char fontName[32];
	} Settings;

	static Settings settings = {"Black", "0", "0", "0", "Arial"};
	
    SetPenColor("Black");

    // 颜色配置输入区域
    textbox(GenUIID(0), x + 1, y + height - 0.3, 4, INPUT_BAR_HEIGHT, settings.colorName, sizeof(settings.colorName));
    drawLabel(x + 0.1, y + height - 0.3, "颜色名:");
    


    if (textbox(GenUIID(1), x + 1, y + height - 0.6, 1, INPUT_BAR_HEIGHT, settings.red, sizeof(settings.red))) {

    }
    drawLabel(x + 0.1, y + height - 0.6, "Red:");



    if (textbox(GenUIID(2), x + 1, y + height - 0.9, 1, INPUT_BAR_HEIGHT, settings.green, sizeof(settings.green))) {

    }
    drawLabel(x + 0.1, y + height - 0.9, "Green:");



    if (textbox(GenUIID(3), x + 1, y + height - 1.2, 1, INPUT_BAR_HEIGHT, settings.blue, sizeof(settings.blue))) {

    }
        drawLabel(x + 0.1, y + height - 1.2, "Blue:");

    // 字体配置输入区域
    textbox(GenUIID(4), x + 1, y + height - 1.5,5, INPUT_BAR_HEIGHT, settings.fontName, sizeof(settings.fontName));
    drawLabel(x + 0.1, y + height - 1.5, "字体:");
    // 绘制确认和退出按钮
    if (button(GenUIID(5), x + width - 2 * BUTTON_WIDTH - 2 * BUTTON_GAP, y + BUTTON_GAP, BUTTON_WIDTH, height - 2 * BUTTON_GAP, "确认")) {
        // 更新设置逻辑
        ChangeDisplayMethodToMain();
    }
    if (button(GenUIID(6), x + width - BUTTON_WIDTH - BUTTON_GAP, y + BUTTON_GAP, BUTTON_WIDTH, height - 2 * BUTTON_GAP, "退出")) {
        ChangeDisplayMethodToMain();
    }
}

////////////////////////////////////////////////// 显示逻辑 ////////////////////////////////////////////
double screenHeight;
double screenWidth;

static void display();
static void displayBegin()
{
    // 设置背景为白色
    SetPenColor("White");
    MovePen(0, 0);
    StartFilledRegion(1);
    DrawLine(screenWidth, 0);
    DrawLine(0, screenHeight);
    DrawLine(-screenWidth, 0);
    DrawLine(0, -screenHeight);
    EndFilledRegion();
    
    // 设置绘制颜色为黑色
    SetPenColor("Black");
}

static void displayMod0() //主界面
{
    DrawMenuBar_(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, MENU_HEIGHT);
    DrawToolBar(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 2 * WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, TOOL_HEIGHT);
    double mainAreaHeight = screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 3 * WINDOW_MARGIN;
    DrawMainArea(WINDOW_MARGIN, WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, mainAreaHeight);
    if(inputMode!=0) display();
}

static void displayMod1() //输入框界面
{
    DrawMenuBar_(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, MENU_HEIGHT);
    DrawToolBar(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 2 * WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, TOOL_HEIGHT);
    double mainAreaHeight = screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - INPUT_BAR_HEIGHT - 4 * WINDOW_MARGIN;
    DrawMainArea(WINDOW_MARGIN, WINDOW_MARGIN + INPUT_BAR_HEIGHT + WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, mainAreaHeight);
    DrawInputBar(WINDOW_MARGIN, WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, INPUT_BAR_HEIGHT);
    if(inputMode!=1) display();
}

static void displayMod2() //弹窗界面
{
    DrawMenuBar_(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, MENU_HEIGHT);
    DrawToolBar(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 2 * WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, TOOL_HEIGHT);
    double mainAreaHeight = screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 3 * WINDOW_MARGIN;
    DrawMainArea(WINDOW_MARGIN, WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, mainAreaHeight);

    // 弹窗输入坐标和大小
    double popupWidth = 5.0;
    double popupHeight = 3.0;
    double mainAreaX = WINDOW_MARGIN;
    double mainAreaY = WINDOW_MARGIN;
    double mainAreaWidth = screenWidth - 2 * WINDOW_MARGIN;
    double popupX = mainAreaX + (mainAreaWidth - popupWidth) / 2;
    double popupY = mainAreaY + (mainAreaHeight - popupHeight) / 2;

    // 覆盖底部区域防止遮挡
    SetPenColor("White");
    MovePen(popupX, popupY);
    StartFilledRegion(1);
    DrawLine(popupWidth, 0);
    DrawLine(0, popupHeight);
    DrawLine(-popupWidth, 0);
    DrawLine(0, -popupHeight);
    EndFilledRegion();

    // 绘制弹窗外部标题
    SetPenColor("Black");
    drawLabel(popupX + 0.1, popupY + popupHeight + 0.2, majorInputMessage);

    // 绘制弹窗
    drawRectangle(popupX, popupY, popupWidth, popupHeight, 0);
    drawLabel(popupX + 0.1, popupY + popupHeight - 0.8, "Input: ");
    drawLabel(popupX + 1.0, popupY + popupHeight - 0.8, inputBuffer);

    // 绘制确认按钮
    if (button(GenUIID(0), popupX + popupWidth - 2 * BUTTON_WIDTH - 2 * BUTTON_GAP, popupY + BUTTON_GAP, BUTTON_WIDTH, popupHeight - 2 * BUTTON_GAP, "确认") || keyboardConfirmed) {
        keyboardConfirmed = 0;
        if (inputConfirmedCallback) {
            inputConfirmedCallback(inputBuffer);
            inputConfirmedCallback = NULL;
        }
        ChangeDisplayMethodToMain();
    }
    // 绘制退出按钮
    if (button(GenUIID(1), popupX + popupWidth - BUTTON_WIDTH - BUTTON_GAP, popupY + BUTTON_GAP, BUTTON_WIDTH, popupHeight - 2 * BUTTON_GAP, "退出") || keyboardCancelled) {
        keyboardCancelled = 0;
        ChangeDisplayMethodToMain();
    }
    if(inputMode!=2) display();
}

static void displayMod3() //搜索界面
{
    DrawMenuBar_(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, MENU_HEIGHT);

    // 保持原来按钮位置不变
    double toolBarYStart = screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - WINDOW_MARGIN;
    double mainAreaHeight = toolBarYStart - INPUT_BAR_HEIGHT - 2 * WINDOW_MARGIN;

    // 绘制搜索输入栏
    double searchInputYStart = toolBarYStart - INPUT_BAR_HEIGHT;
    DrawInputBar(WINDOW_MARGIN, searchInputYStart, screenWidth - 2 * WINDOW_MARGIN, INPUT_BAR_HEIGHT);

    // 绘制搜索结果显示区
    if (searchResultDraw) {
        searchResultDraw(WINDOW_MARGIN, WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, searchInputYStart - 2 * WINDOW_MARGIN);
    }
    if(inputMode!=3) display();
}

static void displayMod4() //统计界面
{
    DrawMenuBar_(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, MENU_HEIGHT);

    // 绘制统计面板
    if (statDisplayDraw) {
        statDisplayDraw(WINDOW_MARGIN, WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT);
    }

    // 绘制退出按钮
    if (button(GenUIID(2), screenWidth / 2 - BUTTON_WIDTH / 2, WINDOW_MARGIN, BUTTON_WIDTH, INPUT_BAR_HEIGHT, "退出")) {
        ChangeDisplayMethodToMain();
    }
    if(inputMode!=4) display();
}

static void displayMod5() //设置界面
{
    DrawMenuBar_(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, MENU_HEIGHT);
    DrawToolBar(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 2 * WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, TOOL_HEIGHT);
    double settingYStart = WINDOW_MARGIN;
    double settingWidth = screenWidth - 2 * WINDOW_MARGIN;
    double settingHeight = screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 4 * WINDOW_MARGIN;
    DrawSettingPage(WINDOW_MARGIN, settingYStart, settingWidth, settingHeight);
    if(inputMode!=5) display();
}

typedef void (*displayFunction)();
static displayFunction displayArr[] = {displayMod0,displayMod1,displayMod2,displayMod3,displayMod4,displayMod5};

static void display()
{
    screenWidth = GetWindowWidth();
    screenHeight = GetWindowHeight();
    displayBegin();
    displayArr[inputMode]();
}

////////////////////////////////////// 事件处理逻辑 ///////////////////////////////////////////
static int              mouseFocusID = 0;
static MouseEventMethod mouseDownFuncs[3][5];       //NULL, left, right ; NULL, editor, explorer, roller, search
static ButtonEvent      mouseUpFuncs[3][5];
static int mouseActivated[3];
static double mouseActivatedDx;
static double mouseActivatedDy;

static void mouseDownProcess(double mx, double my, int button, double dx, double dy, double cx, double cy, int id)
{
    if(mx>=dx && mx<=cx && my<=dy && my>=cy)
    {
        if(mouseFocusID)
        {
            if(mouseUpFuncs[3-button][mouseFocusID])
                mouseUpFuncs[3-button][mouseFocusID]();
            mouseActivated[3-button] = 0;
        }
        mouseActivated[button] = 1;
        if(mouseDownFuncs[button][id])
            mouseDownFuncs[button][id](mx-dx,my-dy);
        mouseFocusID = id;
        mouseActivatedDx = dx;
        mouseActivatedDy = dy;
    }
}

static void mouseUpProcess(int button)
{
    if(mouseActivated[button])
    {
        if(mouseUpFuncs[button][mouseFocusID])
            mouseUpFuncs[button][mouseFocusID]();
        mouseActivated[button] = 0;
        mouseFocusID = 0;
    }
}

static void mouseMoveProcess(double mx, double my)
{
    if(!mouseActivated[1] && !mouseActivated[2]) return;
    int nowButton = mouseActivated[1] ? 1 : 2;
    if(mouseDownFuncs[nowButton][mouseFocusID])
        mouseDownFuncs[nowButton][mouseFocusID](mx-mouseActivatedDx, my-mouseActivatedDy);
}

void MouseEventProcess(int x, int y, int button, int event) {
    DisplayClear();
    uiGetMouse(x, y, button, event);

    // 将屏幕坐标转换为libgraphics坐标
    double xd = ScaleXInches(x);
    double yd = ScaleYInches(y);

    double screenWidth  = GetWindowWidth();
    double screenHeight = GetWindowHeight();

    double titleYStart    = screenHeight - TITLE_HEIGHT - WINDOW_MARGIN;
    double menuYStart     = titleYStart - MENU_HEIGHT;
    double toolYStart     = menuYStart - TOOL_HEIGHT;
    double inputBarYStart = WINDOW_MARGIN;
    double mainAreaYEnd   = inputBarYStart + (inputMode ? INPUT_BAR_HEIGHT : 0);

    double browserWidth = (screenWidth / 3.0);
    double editorXStart = browserWidth + BUTTON_GAP;
    double scrollbarX   = screenWidth - SCROLLBAR_WIDTH;

    if (event==BUTTON_UP && button!=MIDDLE_BUTTON) mouseUpProcess(button==1 ? 1 : 2);
    if (event==MOUSEMOVE) mouseMoveProcess(xd,yd);
    if (event==ROLL_UP || event==ROLL_DOWN)
    {
        if (inputMode==0 && xd>=editorXStart && xd<scrollbarX && yd>mainAreaYEnd && yd<=toolYStart)
        {
            if (event==ROLL_UP   && editorMouseMiddleRollup)
                editorMouseMiddleRollup();
            if (event==ROLL_DOWN && editorMouseMiddleRolldown)
                editorMouseMiddleRolldown();
        }
    }
    if (event==BUTTON_DOWN && button!=MIDDLE_BUTTON)
    {
        if(button==3) button = 2;
        if (inputMode == 0)
        {
            mouseDownProcess(xd, yd, button, 0,            toolYStart, editorXStart, mainAreaYEnd, 2);
            mouseDownProcess(xd, yd, button, editorXStart, toolYStart, scrollbarX,   mainAreaYEnd, 1);
            mouseDownProcess(xd, yd, button, scrollbarX,   toolYStart, screenWidth,  mainAreaYEnd, 3);
        }
        if (inputMode == 3)
        {
            mouseDownProcess(xd, yd, button, 0, screenHeight-TITLE_HEIGHT-MENU_HEIGHT-INPUT_BAR_HEIGHT, screenWidth, 0, 4);
        }
    }

    display();
}

#define KEYSTATE_CONTROL  1
#define KEYSTATE_SHIFT    2
#define KEYSTATE_ALT      4
static int UIKeyState;

static int GetCtrlKeyState()  { return UIKeyState | KEYSTATE_CONTROL; }
static int GetShiftKeyState() { return UIKeyState | KEYSTATE_SHIFT;   }
static int GetAltKeyState()   { return UIKeyState | KEYSTATE_ALT;     }

char* madeupHotkeyCode(char ch)
{
    static char keyTmp[5];
    sprintf(keyTmp,"%d%d%d%c",!!GetCtrlKeyState(),!!GetAltKeyState(),!!GetShiftKeyState(),ch);
    return keyTmp;
}

void CharEventProcess(char ch) {
    DisplayClear();
    uiGetChar(ch);
    if(!GetCtrlKeyState() && !GetAltKeyState())
    {
        if(inputMode == 0) //主界面
        {
            editorKeyboard(ch);
        }
        else if(inputMode==1 || inputMode==2 || inputMode==3) //输入界面
        {
            inputBuffer[inputBufferLength]   = ch;
            inputBuffer[inputBufferLength+1] = '\0';
            ++inputBufferLength;
            if(inputMode==3) inputConfirmedCallback(inputBuffer);
        }
    }
    display();
}

#define KEYBOARD_BACKSPACE 0x08
#define KEYBOARD_ENTER     0x0D
#define KEYBOARD_SHIFT     0x10
#define KEYBOARD_CTRL      0x11
#define KEYBOARD_ALT       0x12
#define KEYBOARD_ESC       0x1B
#define KEYBOARD_SPACE     0x20
#define KEYBOARD_LEFT      0x25
#define KEYBOARD_UP        0x26
#define KEYBOARD_RIGHT     0x27
#define KEYBOARD_DOWN      0x28
#define KEYBOARD_DELETE    0x2E
#define NUMBER_BEGIN       0x30
#define ALPHA_BEGIN        0x41

static int isChar(int key) 
{
    return (key>=NUMBER_BEGIN&&key<NUMBER_BEGIN+10) || (key>=ALPHA_BEGIN&&key<ALPHA_BEGIN+27);
}

static char charize(int key)
{
    if(key>=NUMBER_BEGIN && key<NUMBER_BEGIN+10) return key-NUMBER_BEGIN;
    if(key>=ALPHA_BEGIN  && key<ALPHA_BEGIN +27) return key-ALPHA_BEGIN;
}

void KeyboardEventProcess(int key, int event) {
    DisplayClear();
    uiGetKeyboard(key,event);

    //UI状态改变
    if(key == KEYBOARD_CTRL)
    {
        if(event == KEY_DOWN) UIKeyState |=   KEYSTATE_CONTROL;
        else                  UIKeyState &= 7-KEYSTATE_CONTROL;
    }
    if(key == KEYBOARD_ALT)
    {
        if(event == KEY_DOWN) UIKeyState |=   KEYSTATE_ALT;
        else                  UIKeyState &= 7-KEYSTATE_ALT;
    }
    if(key == KEYBOARD_SHIFT)
    {
        if(event == KEY_DOWN) UIKeyState |=   KEYSTATE_SHIFT;
        else                  UIKeyState &= 7-KEYSTATE_SHIFT;
    }
    if(key==KEYBOARD_CTRL || key==KEYSTATE_ALT || key==KEYSTATE_SHIFT || inputMode>=4)
    {
        display();
        return;
    }
	
    bool ctrlPressed = GetCtrlKeyState();
    bool altPressed = GetAltKeyState();
    bool shiftPressed = GetShiftKeyState();

    //判断快捷键
    if(event==KEY_DOWN && isChar(key) && inputMode!=3)
    {
    	int i;
        for(i=0; i<hotKeyCount; ++i)
        {
            if(strcmp(hotKeys[i].keyCombo, madeupHotkeyCode(charize(key))) == 0)
            {
                hotKeys[i].callback();
            }
        }
    }

    //传入特殊字符
    if(!GetCtrlKeyState() && !GetAltKeyState() && !GetShiftKeyState() && inputMode==0)
    {
        if(inputMode == 0)
        {
            //1上 2下 3左 4右 5回车 6退格 7Delete 
            if(key == KEYBOARD_UP)        editorKeyboardSpecial(1);
            if(key == KEYBOARD_DOWN)      editorKeyboardSpecial(2);
            if(key == KEYBOARD_LEFT)      editorKeyboardSpecial(3);
            if(key == KEYBOARD_RIGHT)     editorKeyboardSpecial(4);
            if(key == KEYBOARD_ENTER)     editorKeyboardSpecial(5);
            if(key == KEYBOARD_BACKSPACE) editorKeyboardSpecial(6);
            if(key == KEYBOARD_DELETE)    editorKeyboardSpecial(7);
        }
        else
        {
            if(key == KEYBOARD_BACKSPACE)
            {
                --inputBufferLength;
                inputBuffer[inputBufferLength] = '\0';
            }
            if(key == KEYBOARD_ENTER)
            {
                keyboardConfirmed = 1;
            }
            if(key == KEYBOARD_ESC)
            {
                keyboardCancelled = 1;
            }
        }
    }

    display();
}

//////////////////////////////////////////////// 界面切换函数 //////////////////////////////////////////

// 实现界面更改时的各种逻辑
static void changeDisplayMethodProcess()
{
    // 若有正在输入的内容，视为取消输入
    if(inputConfirmedCallback) { inputConfirmedCallback(0); inputConfirmedCallback=NULL; }
    // 若鼠标正在拖动过程，则取消拖动
    mouseUpProcess(1); mouseUpProcess(2);
}

static void clearInputBuffers()
{
    inputBufferLength = 0;
    inputBuffer[0] = '\0';
}

// 主界面
void ChangeDisplayMethodToMain() {
    changeDisplayMethodProcess();
    double mainAreaHeight = screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 3 * WINDOW_MARGIN;
    setRollerHeight(mainAreaHeight);
    inputMode = 0;
}

// 小输入界面
void ChangeDisplayMethodToInput(ButtonEventWithInput callback) {
    changeDisplayMethodProcess();
    double mainAreaHeight = screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - INPUT_BAR_HEIGHT - 4 * WINDOW_MARGIN
    setRollerHeight(mainAreaHeight);
    inputMode = 1;
    inputConfirmedCallback = callback;
    clearInputBuffers();
}

// 弹窗输入界面
void ChangeDisplayMethodToMajorInput(char* inputMessage, ButtonEventWithInput callback) {
    changeDisplayMethodProcess();
    double mainAreaHeight = screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 3 * WINDOW_MARGIN;
    setRollerHeight(mainAreaHeight);
    inputMode = 2;
    inputConfirmedCallback = callback;
    clearInputBuffers();
    strncpy(majorInputMessage, inputMessage, sizeof(majorInputMessage));
    majorInputMessage[sizeof(majorInputMessage) - 1] = '\0';
}

// 搜索界面
void ChangeDisplayMethodToSearch(ButtonEventWithInput callback) {
    changeDisplayMethodProcess();
    inputMode = 3;
    inputConfirmedCallback = callback;
    clearInputBuffers();
}

// 统计界面
void ChangeDisplayMethodToStat() {
    changeDisplayMethodProcess();
    inputMode = 4;
}

// 设置界面
void ChangeDisplayMethodToSetting() {
    changeDisplayMethodProcess();
    inputMode = 5;
}

////////////////////////////////////////////////// 启动与初始化逻辑 /////////////////////////////////////
void GUI_Start() {
    InitGraphics();
    startTime = clock();
    setEditorWidth((2.0 * (GetWindowWidth() - 2 * WINDOW_MARGIN) / 3.0) - BUTTON_GAP - SCROLLBAR_WIDTH);

    setMenuColors("Black", "Black", "Red", "Red", 0);
    setButtonColors("Black", "Black", "Red", "Red", 0);
    SetPenColor("Black");

    ChangeDisplayMethodToMain();
    registerMouseEvent(MouseEventProcess);
    registerKeyboardEvent(KeyboardEventProcess);
    registerCharEvent(CharEventProcess);
}


//////////////////////////////////////////////// 实现注册函数 //////////////////////////////////////////
//Display 或 Draw 函数
void RegisterExplorerDraw(AreaPaintMethod cb)         { explorerDraw = cb; }
void RegisterEditorGraphicDraw(AreaPaintMethod cb)    { editorGraphicDraw = cb; }
void RegisterRollerUpperBoundDraw(GetInfoMethod cb)   { rollerUpperBoundDraw = cb; }
void RegisterRollerLowerBoundDraw(GetInfoMethod cb)   { rollerLowerBoundDraw = cb; }
void RegisterSearchDisplayMethod(AreaPaintMethod cb)  { searchResultDraw = cb; }
void RegisterStatDisplayMethod(AreaPaintMethod cb)    { statDisplayDraw = cb; }
void RegisterSetRollerHeightMethod(SetValueMethod cb) { setRollerHeight = cb; }
void RegisterSetEditorWidth(SetValueMethod cb)        { setEditorWidth = cb; }

//鼠标响应函数
//NULL, left, right ; NULL, editor, explorer, roller, search
void RegisterEditorMouseLeftDown(MouseEventMethod cb)    { mouseDownFuncs[1][1] = cb; }
void RegisterEditorMouseLeftUp(ButtonEvent cb)           { mouseUpFuncs[1][1] = cb; }
void RegisterEditorMouseRightDown(MouseEventMethod cb)   { mouseDownFuncs[2][1] = cb; }
void RegisterEditorMouseRightUp(ButtonEvent cb)          { mouseUpFuncs[2][1] = cb; }
void RegisterExplorerMouseLeftDown(MouseEventMethod cb)  { mouseDownFuncs[1][2] = cb; }
void RegisterExplorerMouseLeftUp(ButtonEvent cb)         { mouseUpFuncs[1][2] = cb; }
void RegisterExplorerMouseRightDown(MouseEventMethod cb) { mouseDownFuncs[2][2] = cb; }
void RegisterExplorerMouseRightUp(ButtonEvent cb)        { mouseUpFuncs[2][2] = cb; }
void RegisterRollerMouseLeftDown(MouseEventMethod cb)    { mouseDownFuncs[1][3] = cb; }
void RegisterRollerMouseLeftUp(ButtonEvent cb)           { mouseUpFuncs[1][3] = cb; }
void RegisterSearchMouseLeftDown(MouseEventMethod cb)    { mouseDownFuncs[1][4] = cb; }
void RegisterSearchMouseLeftUp(ButtonEvent cb)           { mouseUpFuncs[1][4] = cb; }

void RegisterEditorMouseMiddleRollup(ButtonEvent cb)       { editorMouseMiddleRollup = cb; }
void RegisterEditorMouseMiddleRolldown(ButtonEvent cb)     { editorMouseMiddleRolldown = cb; }

//键盘响应函数
void RegisterEditorKeyboard(KeyboardEventMethod cb)        { editorKeyboard = cb; }
void RegisterEditorKeyboardSpecial(KeyboardEventMethod cb) { editorKeyboardSpecial = cb; }
void RegisterEditorHotKey(char* keyCombo, ButtonEvent cb) {
    strcpy(hotKeys[hotKeyCount].keyCombo, keyCombo);
    hotKeys[hotKeyCount].callback = cb;
    hotKeyCount++;
}

//菜单响应函数
/*
static char *fileMenu[]  = {"File", "Save", "Open", "New", "Save As", "Close"};
static char *toolsMenu[] = {"Tools", "Stats", "Search", "Browse"};
static char *aboutMenu[] = {"About", "Settings", "Help"};
*/
void RegisterSaveMethod(ButtonEvent cb)   { FileMenuMethod[0]=cb; }
void RegisterOpenMethod(ButtonEvent cb)   { FileMenuMethod[1]=cb; }
void RegisterCreateMethod(ButtonEvent cb) { FileMenuMethod[2]=cb; }
void RegisterSaveasMethod(ButtonEvent cb) { FileMenuMethod[3]=cb; }
void RegisterCloseMethod(ButtonEvent cb)  { FileMenuMethod[4]=cb; }

void RegisterStatMethod(ButtonEvent cb)    { ToolsMenuMethod[0]=cb; }
void RegisterSearchMethod(ButtonEvent cb)  { ToolsMenuMethod[1]=cb; }
void RegisterExploreMethod(ButtonEvent cb) { ToolsMenuMethod[2]=cb; }

void RegisterSettingMethod(ButtonEvent cb) { AboutMenuMethod[0]=cb; }
void RegisterHelpMethod(ButtonEvent cb)    { AboutMenuMethod[1]=cb; }

//按钮响应函数
void RegisterColorMethod(ButtonEventWithInput cb)     { colorButton = cb; }
void RegisterBoldMethod(ButtonEvent cb)               { boldButton = cb; }
void RegisterItalicMethod(ButtonEvent cb)             { italicButton = cb; }
void RegisterPointSizeMethod(ButtonEventWithInput cb) { sizeButton = cb; }
void RegisterFontMethod(ButtonEventWithInput cb)      { fontButton = cb; }
void RegisterImageMethod(ButtonEvent cb)              { imageButton = cb; }