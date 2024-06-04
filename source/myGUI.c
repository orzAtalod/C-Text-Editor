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

////////////////////////////////////// �����ͳ������� ////////////////////////////////////////////

// �궨��
#define TITLE_HEIGHT 0             // �������߶�
#define MENU_HEIGHT 0.3            // �˵����߶�
#define TOOL_HEIGHT 0.5            // �������߶�
#define BUTTON_WIDTH 1.0           // ��ť���
#define BUTTON_GAP 0.05            // ��ť֮��ļ�϶
#define SCROLLBAR_WIDTH 0.1        // ���������
#define WINDOW_MARGIN 0            // ���ڱ�Ե�ļ�϶
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
static ButtonEvent AboutMenuMethod[2] = {};    // ����˵���
static char *fileMenu[]  = {"File", "Save", "Open", "New", "Save As", "Close"};
static char *toolsMenu[] = {"Tools", "Stats", "Search", "Browse"};
static char *aboutMenu[] = {"About", "Settings", "Help"};

static char lastSelectedColor[32]                     = "Red"; // �洢�����ɫ
static ButtonEventWithInput colorButton               = NULL;
static ButtonEventWithInput sizeButton                = NULL;
static ButtonEventWithInput fontButton                = NULL;

static int inputMode                               = 0;      // ����ģʽ��־λ
static ButtonEventWithInput inputConfirmedCallback = NULL;
static char inputBuffer[256];                                // ������������
static int inputBufferLength                       = 0;
static char majorInputMessage[256];                          // ����������ʾ��Ϣ
static int keyboardConfirmed                       = 0;
static int keyboardCancelled                       = 0;

static AreaPaintMethod searchResultDraw = NULL;
static AreaPaintMethod statDisplayDraw  = NULL;               // ���ڻ���ͳ�����

static ColorDefinitionMethod defineColor = NULL;

// �洢��ݼ�ӳ��
static struct {
    char keyCombo[5];
    ButtonEvent callback;
} hotKeys[100];
static int hotKeyCount = 0;

// �洢ʱ��
static clock_t startTime;
long GetClock() {
    clock_t currentTime = clock();
    return (long)((double)(currentTime - startTime) / CLOCKS_PER_SEC * 1000);
}

////////////////////////////////////////////////// �����߼� ///////////////////////////////////////////
static void DrawMenuBar_(double x, double y, double width, double height) {
    // ���Ʋ˵������������߿򣬲���䣩
    SetPenColor("Black");
    drawRectangle(x, y, width, height, 0);

    // ���Ʋ˵�
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
    // ���ƹ��������������߿򣬲���䣩
    SetPenColor("Black");
    drawRectangle(x, y, width, height, 0);

    // ���尴ťλ�ú�����
    double buttonX = x + BUTTON_GAP;
    double buttonY = y + BUTTON_GAP;
    double buttonWidth = BUTTON_WIDTH;
    double buttonHeight = height - 2 * BUTTON_GAP;

    // ����ÿһ����ť
    SetPenColor("Black");  // ����Ĭ����ɫ��ֹ�߿���Ӱ��
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
    // ��������ֵ�λ�úͳߴ�
    double browserWidth = (width / 3.0);
    double editorWidth = (2.0 * width / 3.0) - BUTTON_GAP - SCROLLBAR_WIDTH;
    double scrollbarX = x + (width - SCROLLBAR_WIDTH);

    // ��������������߿򣬲���䣩
    SetPenColor("Black");
    drawRectangle(x, y, browserWidth, height, 0);
    if (explorerDraw) {
        explorerDraw(x, y, browserWidth, height);
    } else {
        drawLabel(x + 0.1, y + height - 0.2, "Browser Area");
    }

    // ���Ʊ༭�������߿򣬲���䣩
    SetPenColor("Black");
    drawRectangle(x + browserWidth + BUTTON_GAP, y, editorWidth, height, 0);
    if (editorGraphicDraw) {
        editorGraphicDraw(x + browserWidth + BUTTON_GAP, y, editorWidth, height);
    } else {
        drawLabel(x + browserWidth + BUTTON_GAP + 0.1, y + height - 0.2, "Editing Area");
    }

    // ���ƹ����������߿򣬲���䣩
    SetPenColor("Black");
    drawRectangle(scrollbarX, y, SCROLLBAR_WIDTH, height, 0);

    if (rollerUpperBoundDraw && rollerLowerBoundDraw) {
        const double upperBound = rollerUpperBoundDraw();
        const double lowerBound = rollerLowerBoundDraw();
        const double topY = y + height * (1 - upperBound);
        const double bottomY = y + height * (1 - lowerBound);
        drawRectangle(scrollbarX, bottomY, SCROLLBAR_WIDTH, topY - bottomY, 1);  // ���������ֶ�
    } else {
        drawLabel(scrollbarX + 0.1, y + height - 0.2, "Scrollbar");
    }
}

void DrawInputBar(double x, double y, double width, double height) {
    SetPenColor("Black");
    drawRectangle(x, y, width, height, 0);
    
    drawLabel(x + 0.1, y + height / 2 - 0.1, inputMode==1 ? "Input: " : "Search: ");
    drawLabel(x + 1.0, y + height / 2 - 0.1, inputBuffer);

    // ȷ�ϰ�ť
    if (button(GenUIID(0), x + width - 2 * BUTTON_WIDTH - 2 * BUTTON_GAP, y + BUTTON_GAP, BUTTON_WIDTH, height - 2 * BUTTON_GAP, "ȷ��") || keyboardConfirmed) {
        keyboardConfirmed = 0;
        if (inputMode == 1) {
            if (inputConfirmedCallback) {
                inputConfirmedCallback(inputBuffer);
                inputConfirmedCallback = NULL; // ��ֹ�ٴδ���
            }
            ChangeDisplayMethodToMain();
        } // ����ģʽ��ȷ�ϰ�ť��Ч��
    }

    // �˳���ť
    if (button(GenUIID(1), x + width - BUTTON_WIDTH - BUTTON_GAP, y + BUTTON_GAP, BUTTON_WIDTH, height - 2 * BUTTON_GAP, "�˳�") || keyboardCancelled) {
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

    // ��ɫ������������
    textbox(GenUIID(0), x + 1, y + height - 0.3, 4, INPUT_BAR_HEIGHT, settings.colorName, sizeof(settings.colorName));
    drawLabel(x + 0.1, y + height - 0.3, "��ɫ��:");
    


    if (textbox(GenUIID(1), x + 1, y + height - 0.6, 1, INPUT_BAR_HEIGHT, settings.red, sizeof(settings.red))) {

    }
    drawLabel(x + 0.1, y + height - 0.6, "Red:");



    if (textbox(GenUIID(2), x + 1, y + height - 0.9, 1, INPUT_BAR_HEIGHT, settings.green, sizeof(settings.green))) {

    }
    drawLabel(x + 0.1, y + height - 0.9, "Green:");



    if (textbox(GenUIID(3), x + 1, y + height - 1.2, 1, INPUT_BAR_HEIGHT, settings.blue, sizeof(settings.blue))) {

    }
        drawLabel(x + 0.1, y + height - 1.2, "Blue:");

    // ����������������
    textbox(GenUIID(4), x + 1, y + height - 1.5,5, INPUT_BAR_HEIGHT, settings.fontName, sizeof(settings.fontName));
    drawLabel(x + 0.1, y + height - 1.5, "����:");
    // ����ȷ�Ϻ��˳���ť
    if (button(GenUIID(5), x + width - 2 * BUTTON_WIDTH - 2 * BUTTON_GAP, y + BUTTON_GAP, BUTTON_WIDTH, height - 2 * BUTTON_GAP, "ȷ��")) {
        // ���������߼�
        ChangeDisplayMethodToMain();
    }
    if (button(GenUIID(6), x + width - BUTTON_WIDTH - BUTTON_GAP, y + BUTTON_GAP, BUTTON_WIDTH, height - 2 * BUTTON_GAP, "�˳�")) {
        ChangeDisplayMethodToMain();
    }
}

////////////////////////////////////////////////// ��ʾ�߼� ////////////////////////////////////////////
double screenHeight;
double screenWidth;

static void display();
static void displayBegin()
{
    // ���ñ���Ϊ��ɫ
    SetPenColor("White");
    MovePen(0, 0);
    StartFilledRegion(1);
    DrawLine(screenWidth, 0);
    DrawLine(0, screenHeight);
    DrawLine(-screenWidth, 0);
    DrawLine(0, -screenHeight);
    EndFilledRegion();
    
    // ���û�����ɫΪ��ɫ
    SetPenColor("Black");
}

static void displayMod0() //������
{
    DrawMenuBar_(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, MENU_HEIGHT);
    DrawToolBar(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 2 * WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, TOOL_HEIGHT);
    double mainAreaHeight = screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 3 * WINDOW_MARGIN;
    DrawMainArea(WINDOW_MARGIN, WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, mainAreaHeight);
    if(inputMode!=0) display();
}

static void displayMod1() //��������
{
    DrawMenuBar_(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, MENU_HEIGHT);
    DrawToolBar(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 2 * WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, TOOL_HEIGHT);
    double mainAreaHeight = screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - INPUT_BAR_HEIGHT - 4 * WINDOW_MARGIN;
    DrawMainArea(WINDOW_MARGIN, WINDOW_MARGIN + INPUT_BAR_HEIGHT + WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, mainAreaHeight);
    DrawInputBar(WINDOW_MARGIN, WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, INPUT_BAR_HEIGHT);
    if(inputMode!=1) display();
}

static void displayMod2() //��������
{
    DrawMenuBar_(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, MENU_HEIGHT);
    DrawToolBar(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 2 * WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, TOOL_HEIGHT);
    double mainAreaHeight = screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 3 * WINDOW_MARGIN;
    DrawMainArea(WINDOW_MARGIN, WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, mainAreaHeight);

    // ������������ʹ�С
    double popupWidth = 5.0;
    double popupHeight = 3.0;
    double mainAreaX = WINDOW_MARGIN;
    double mainAreaY = WINDOW_MARGIN;
    double mainAreaWidth = screenWidth - 2 * WINDOW_MARGIN;
    double popupX = mainAreaX + (mainAreaWidth - popupWidth) / 2;
    double popupY = mainAreaY + (mainAreaHeight - popupHeight) / 2;

    // ���ǵײ������ֹ�ڵ�
    SetPenColor("White");
    MovePen(popupX, popupY);
    StartFilledRegion(1);
    DrawLine(popupWidth, 0);
    DrawLine(0, popupHeight);
    DrawLine(-popupWidth, 0);
    DrawLine(0, -popupHeight);
    EndFilledRegion();

    // ���Ƶ����ⲿ����
    SetPenColor("Black");
    drawLabel(popupX + 0.1, popupY + popupHeight + 0.2, majorInputMessage);

    // ���Ƶ���
    drawRectangle(popupX, popupY, popupWidth, popupHeight, 0);
    drawLabel(popupX + 0.1, popupY + popupHeight - 0.8, "Input: ");
    drawLabel(popupX + 1.0, popupY + popupHeight - 0.8, inputBuffer);

    // ����ȷ�ϰ�ť
    if (button(GenUIID(0), popupX + popupWidth - 2 * BUTTON_WIDTH - 2 * BUTTON_GAP, popupY + BUTTON_GAP, BUTTON_WIDTH, popupHeight - 2 * BUTTON_GAP, "ȷ��") || keyboardConfirmed) {
        keyboardConfirmed = 0;
        if (inputConfirmedCallback) {
            inputConfirmedCallback(inputBuffer);
            inputConfirmedCallback = NULL;
        }
        ChangeDisplayMethodToMain();
    }
    // �����˳���ť
    if (button(GenUIID(1), popupX + popupWidth - BUTTON_WIDTH - BUTTON_GAP, popupY + BUTTON_GAP, BUTTON_WIDTH, popupHeight - 2 * BUTTON_GAP, "�˳�") || keyboardCancelled) {
        keyboardCancelled = 0;
        ChangeDisplayMethodToMain();
    }
    if(inputMode!=2) display();
}

static void displayMod3() //��������
{
    DrawMenuBar_(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, MENU_HEIGHT);

    // ����ԭ����ťλ�ò���
    double toolBarYStart = screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - WINDOW_MARGIN;
    double mainAreaHeight = toolBarYStart - INPUT_BAR_HEIGHT - 2 * WINDOW_MARGIN;

    // ��������������
    double searchInputYStart = toolBarYStart - INPUT_BAR_HEIGHT;
    DrawInputBar(WINDOW_MARGIN, searchInputYStart, screenWidth - 2 * WINDOW_MARGIN, INPUT_BAR_HEIGHT);

    // �������������ʾ��
    if (searchResultDraw) {
        searchResultDraw(WINDOW_MARGIN, WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, searchInputYStart - 2 * WINDOW_MARGIN);
    }
    if(inputMode!=3) display();
}

static void displayMod4() //ͳ�ƽ���
{
    DrawMenuBar_(WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT - WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, MENU_HEIGHT);

    // ����ͳ�����
    if (statDisplayDraw) {
        statDisplayDraw(WINDOW_MARGIN, WINDOW_MARGIN, screenWidth - 2 * WINDOW_MARGIN, screenHeight - TITLE_HEIGHT - MENU_HEIGHT);
    }

    // �����˳���ť
    if (button(GenUIID(2), screenWidth / 2 - BUTTON_WIDTH / 2, WINDOW_MARGIN, BUTTON_WIDTH, INPUT_BAR_HEIGHT, "�˳�")) {
        ChangeDisplayMethodToMain();
    }
    if(inputMode!=4) display();
}

static void displayMod5() //���ý���
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

////////////////////////////////////// �¼������߼� ///////////////////////////////////////////
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

    // ����Ļ����ת��Ϊlibgraphics����
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
        if(inputMode == 0) //������
        {
            editorKeyboard(ch);
        }
        else if(inputMode==1 || inputMode==2 || inputMode==3) //�������
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

    //UI״̬�ı�
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

    //�жϿ�ݼ�
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

    //���������ַ�
    if(!GetCtrlKeyState() && !GetAltKeyState() && !GetShiftKeyState() && inputMode==0)
    {
        if(inputMode == 0)
        {
            //1�� 2�� 3�� 4�� 5�س� 6�˸� 7Delete 
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

//////////////////////////////////////////////// �����л����� //////////////////////////////////////////

// ʵ�ֽ������ʱ�ĸ����߼�
static void changeDisplayMethodProcess()
{
    // ����������������ݣ���Ϊȡ������
    if(inputConfirmedCallback) { inputConfirmedCallback(0); inputConfirmedCallback=NULL; }
    // ����������϶����̣���ȡ���϶�
    mouseUpProcess(1); mouseUpProcess(2);
}

static void clearInputBuffers()
{
    inputBufferLength = 0;
    inputBuffer[0] = '\0';
}

// ������
void ChangeDisplayMethodToMain() {
    changeDisplayMethodProcess();
    double mainAreaHeight = screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - 3 * WINDOW_MARGIN;
    setRollerHeight(mainAreaHeight);
    inputMode = 0;
}

// С�������
void ChangeDisplayMethodToInput(ButtonEventWithInput callback) {
    changeDisplayMethodProcess();
    double mainAreaHeight = screenHeight - TITLE_HEIGHT - MENU_HEIGHT - TOOL_HEIGHT - INPUT_BAR_HEIGHT - 4 * WINDOW_MARGIN
    setRollerHeight(mainAreaHeight);
    inputMode = 1;
    inputConfirmedCallback = callback;
    clearInputBuffers();
}

// �����������
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

// ��������
void ChangeDisplayMethodToSearch(ButtonEventWithInput callback) {
    changeDisplayMethodProcess();
    inputMode = 3;
    inputConfirmedCallback = callback;
    clearInputBuffers();
}

// ͳ�ƽ���
void ChangeDisplayMethodToStat() {
    changeDisplayMethodProcess();
    inputMode = 4;
}

// ���ý���
void ChangeDisplayMethodToSetting() {
    changeDisplayMethodProcess();
    inputMode = 5;
}

////////////////////////////////////////////////// �������ʼ���߼� /////////////////////////////////////
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


//////////////////////////////////////////////// ʵ��ע�ắ�� //////////////////////////////////////////
//Display �� Draw ����
void RegisterExplorerDraw(AreaPaintMethod cb)         { explorerDraw = cb; }
void RegisterEditorGraphicDraw(AreaPaintMethod cb)    { editorGraphicDraw = cb; }
void RegisterRollerUpperBoundDraw(GetInfoMethod cb)   { rollerUpperBoundDraw = cb; }
void RegisterRollerLowerBoundDraw(GetInfoMethod cb)   { rollerLowerBoundDraw = cb; }
void RegisterSearchDisplayMethod(AreaPaintMethod cb)  { searchResultDraw = cb; }
void RegisterStatDisplayMethod(AreaPaintMethod cb)    { statDisplayDraw = cb; }
void RegisterSetRollerHeightMethod(SetValueMethod cb) { setRollerHeight = cb; }
void RegisterSetEditorWidth(SetValueMethod cb)        { setEditorWidth = cb; }

//�����Ӧ����
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

//������Ӧ����
void RegisterEditorKeyboard(KeyboardEventMethod cb)        { editorKeyboard = cb; }
void RegisterEditorKeyboardSpecial(KeyboardEventMethod cb) { editorKeyboardSpecial = cb; }
void RegisterEditorHotKey(char* keyCombo, ButtonEvent cb) {
    strcpy(hotKeys[hotKeyCount].keyCombo, keyCombo);
    hotKeys[hotKeyCount].callback = cb;
    hotKeyCount++;
}

//�˵���Ӧ����
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

//��ť��Ӧ����
void RegisterColorMethod(ButtonEventWithInput cb)     { colorButton = cb; }
void RegisterBoldMethod(ButtonEvent cb)               { boldButton = cb; }
void RegisterItalicMethod(ButtonEvent cb)             { italicButton = cb; }
void RegisterPointSizeMethod(ButtonEventWithInput cb) { sizeButton = cb; }
void RegisterFontMethod(ButtonEventWithInput cb)      { fontButton = cb; }
void RegisterImageMethod(ButtonEvent cb)              { imageButton = cb; }