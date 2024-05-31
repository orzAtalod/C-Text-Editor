#ifndef __MY_GUI_H_INCLUDED__
#define __MY_GUI_H_INCLUDED__

/*
*
* interface for GUI system
* 
* 基础交互逻辑如下：
* 1. main函数在各种初始化函数完成的结尾调用 GUI_Start() 函数并且结束控制流
*    在 GUI_Start() 函数中应该完成各种消息事件的登录并且主导消息循环
* 2. 各种按钮所激活的函数通过相应的 register 函数在 GUI_start() 函数调用前
*    完成初始化，GUI系统只负责显示和简单的消息处理 
* 3. 不是只有 GUI 会使用图形库函数，程序的其他部分也会使用图形库函数进行绘
*    制（主要为editorGraphic.h），在 GUI 执行绘制任务的同时，需要调用相应
*    的函数传入相应参数完成细部描绘，具体要求见界面设计文档
* 4. 具备界面切换功能，不同的界面遵循不同的消息转发逻辑，具体要求见界面设
*    计文档 
* 5. 在功能测试时可能会随需求添加新的按钮 
* 6. 许多坐标需要传入相对于相应区域**左上角**的坐标，别传成绝对坐标了，也
*    别传成相对于左下角的坐标；相对坐标的方向和 libgraphics 的定义相同，即
*    向右向上为正 
* 7. 在任意时刻能够调用 GetClock() 函数记录自从 GUI_Start() 函数执行后所过
*    的毫秒数 
*
*/ 

void GUI_Start();

typedef void (*ButtonEvent)();
typedef void (*ButtonEventWithInput)(const char*);
typedef void (*AreaPaintMethod)(double,double,double,double); 
typedef double (*GetInfoMethod)(void);
typedef void (*MouseEventMethod)(double,double);
typedef void (*KeyboardEventMethod)(char); 
typedef void (*ColorDefinitionMethod)(const char*,double,double,double)

/*
*
* 界面 1：主编辑界面 
* 从上到下分为三行，分别为菜单栏、按钮栏和大编辑区，大编辑区应占大部分区域 
* 
* 菜单栏包含三项，内容如下 
* 1. 文件：保存、打开、新建、另存为、关闭 
* 2. 工具：统计、搜索、浏览 
* 3. 关于：设置、帮助
*
* 按钮栏包含一组按钮，内容分别如下，如果没有在文后进一步说明，则点击按钮后直接转发消息 
* 1. 颜色，需要自行切换到输入界面（界面 2）来获取具体颜色
* 2. 快捷颜色，具体颜色为上次选择的非黑色颜色（默认为红色） 
*    快捷颜色按钮需要提示目前的快捷颜色是什么（比如更改文本颜色为预期的快捷颜色） 
* 4. 加粗
* 5. 斜体 
* 6. 字号，自行切换到输入界面（界面 2）来获取具体字号 
* 7. 字体，自行切换到输入界面（界面 2）来获取具体字体
* 8. 图片插入
* 如果按钮太多导致排版困难，可以考虑去掉 2 这一个按钮
*
* 大编辑区从左至右分为三个部分，分别为浏览栏、编辑区和滚动条，绘制方法分别为：
* 浏览栏：使用注册的 ExplorerDraw(double, double, double, double)函数，传入浏览栏的左上角
*         与右下角坐标完成绘制。 
* 编辑区：使用注册的 EditorGraphicDraw(double, double, double, double)函数，传入编辑区的
*         左上角与右下角坐标完成绘制。 
* 滚动条：使用注册的 RollerUpperBound() 和 RollerLowerBound() 函数获知滚动条粗段的起始
*         与结束位置，自行绘制（起始、结束位置格式为占整条长度的比例，在 0 和 1 之间） 
*
* 当鼠标事件位于编辑区时，或者当键盘事件发生时，需要转发一些信息：
* 1. 快捷键信息，由 RegisterEditorHotKey(char*, ButtonEvent)定义，当按下快捷键时调用相应
*    函数。 
* 2. 鼠标拖动信息，当鼠标指针位于编辑区并且左键被按下时，对于任何鼠标移动信息调用注册的
*    EditorMouseLeftDown(double,double) 传入鼠标当前所处位置相对于编辑区左上角的坐标。 
* 3. 鼠标抬起信息，当鼠标指针松开时调用 EditorMouseLeftUp() 事件（有修改，现在在鼠标移出
*    界面后不再视为已松开鼠标） 
* 4. 右键与左键相同，左右两键同时按下时视为先松开原本按着的键再按下新键
* 5. 中键的滚动也需要被记录 
* 4. 键盘上下左右、Enter、Backspace、Delete 信息，调用相应函数即可。 
* 5. 键盘输入信息，关于键盘输入信息的说明及要求见附录 1 。 
*
* 鼠标事件位于浏览区、滚动条时也需要转发一些信息，内容与编辑区的同名函数相同
* 若鼠标事件跨越浏览区与编辑区的界限（从编辑区拖动到浏览区或反之）则不将鼠标事件转发至 
*   跨越入的区域，而是仍旧转发给跨越前的区域，照常计算鼠标的相对坐标（这次不在第四象限了） 
*  
*/

void ChangeDisplayMethodToMain();

void RegisterSaveMethod(ButtonEvent);
void RegisterOpenMethod(ButtonEvent);
void RegisterCreateMethod(ButtonEvent);
void RegisterSaveasMethod(ButtonEvent);
void RegisterCloseMethod(ButtonEvent);

void RegisterStatMethod(ButtonEvent);
void RegisterSearchMethod(ButtonEvent);
void RegisterExploreMethod(ButtonEvent);

void RegisterSettingMethod(ButtonEvent);
void RegisterHelpMethod(ButtonEvent);

void RegisterColorMethod(ButtonEventWithInput);
void RegisterBoldMethod(ButtonEvent);
void RegisterItalicMethod(ButtonEvent);
void RegisterPointSizeMethod(ButtonEventWithInput);
void RegisterFontMethod(ButtonEventWithInput);
void RegisterImageMethod(ButtonEvent);

void RegisterExplorerDraw(AreaPaintMethod);
void RegisterEditorGraphicDraw(AreaPaintMethod);
void RegisterRollerUpperBoundDraw(GetInfoMethod);
void RegisterRollerLowerBoundDraw(GetInfoMethod);

void RegisterEditorHotKey(char*, ButtonEvent);
void RegisterEditorMouseLeftDown(MouseEventMethod);
void RegisterEditorMouseLeftUp(ButtonEvent);
void RegisterEditorMouseMiddleRollup(ButtonEvent);
void RegisterEditorMouseMiddleRolldown(ButtonEvent);
void RegisterEditorMouseRightDown(MouseEventMethod);
void RegisterEditorMouseRightUp(ButtonEvent);
void RegisterEditorKeyboard(KeyboardEventMethod);

void RegisterEditorKeyboardSpecial(KeyboardEventMethod); //1上 2下 3左 4右 5回车 6退格 7Delete 

void RegisterExplorerMouseLeftDown(MouseEventMethod);
void RegisterExplorerMouseLeftUp(ButtonEvent);
void RegisterExplorerMouseRightDown(MouseEventMethod);
void RegisterExplorerMouseRightUp(ButtonEvent);

void RegisterRollerMouseLeftDown(MouseEventMethod);
void RegisterRollerMouseLeftUp(ButtonEvent);

/*
*
* 界面 2：输入界面
* 与主编辑界面基本相同，区别是在某处（随排版而定，建议是在按纽栏下方或页面最底部）增加
*   一行输入栏，同时禁用所有与大编辑区相关的转发事件。 
* 输入栏应该拦截键盘字符输入（见附录 1）转换为并以某个字体（可更改）显示输入内容并记录 
* 在输入栏的某处设置确认和退出按钮，按回车也可直接确认输入内容。 
* 确认输入内容后应调用相应回调函数；如果是自行进入则应获取相应值作为消息参数。 
* 退出输入后应丢弃输入内容，调用相应回调函数传入空字符串，如果是自行进入则不执行任何行动
* 在输入过程中鼠标点击菜单栏中选项或者按钮栏中按钮等同于直接退出并同时触发相应按钮，需要
*   注意鼠标点击大编辑区或其他无关区域应忽略，因为消息转发已被禁用。 
*
*/

void ChangeDisplayMethodToInput(ButtonEventWithInput callback);

/*
*
* 界面 3(1)：弹窗输入界面
* 与输入界面基本相同，区别在于输入栏以弹窗形式呈现，并移动到大编辑区中央。 
* 弹窗应该能在输入栏的上方显示一行文本。 
* 编辑区消息同样被禁用，同时可以不绘制编辑区内容，因为尚不清楚能否同时显示编辑区内容与
*   弹窗内容并且不相互叠加；如果extgraphic中的erase相关方法能实现不相互叠加，则可能显
*   示编辑区更加美观。 
*
*/

void ChangeDisplayMethodToMajorInput(char* inputMessage, ButtonEventWithInput callback);

/*
*
* 界面 3(2)：弹窗按钮界面 
* 与弹窗输入界面基本相同，输入方式不是文本框而是按钮
* 参数为所显示的文本，按钮数量（0/1或2个），各按钮文本（字符数组从0开始）
* 返回值为所选择按钮的序号（序号从1开始），若直接退出则返回 0 
*
*/

int ChangeDisplayMethodToMajorButton(char* inputMessage, int textNum, const char** textValue);

/*
*
* 界面 4：搜索界面（与浏览界面共用） 
* 界面仅包含菜单栏、输入栏与搜索结果显示三个部分
* 菜单栏所包含内容与主编辑界面相同
* 输入栏逻辑与弹窗输入界面相同，但需注意输入栏所输入内容需要实时转发（每更改一次即调用
*   一次相应回调函数）
* 搜索结果显示使用外部模块绘制，逻辑与其他外部模块绘制的界面相同
* 搜索结果显示部分需要转发鼠标点击事件 
* 同时需要有退出按钮，退出后返回原界面（如果不易实现可以改为返回主编辑界面） 
*
*/

void ChangeDisplayMethodToSearch();

void RegisterSearchDisplayMethod(AreaPaintMethod);
void RegisterSearchInputMethod(ButtonEventWithInput);

/*
*
* 界面 5：统计界面
* 界面仅包含菜单栏和统计面板两个部分，菜单栏所包含内容与主编辑界面相同，统计面板
*   由外部模块绘制
* 别忘了设计退出按钮 
*
*/

void ChangeDisplayMethodToStat();

void RegisterStatDisplayMethod(AreaPaintMethod);

/*
*
* 界面 6：设置界面 
* 界面包含以下设置项
* 1. 定义颜色，包含颜色名和RGB四个输入栏
* 2. 定义输入栏字体，包含一个输入栏用于输入字体
* 3. 其他选项待定 
*
*/

void ChangeDisplayMethodToSetting();

void RegisterColorDifinitionMethod(ColorDefinitionMethod);

/*
*
* 附录1：键盘输入逻辑
* 合法的输入字符包含键盘上任何可显示字符及空格，回车作为命令考虑
* 当一个键盘事件被考虑为字符输入，当且仅当没有Ctrl、Alt与其一起按下，如果一起按下则应该
*   作为快捷键处理，如果没有相应的快捷键则应被忽略
* 转发键盘输入消息时，需要考虑字符的大小写，如果CAPSLOCK处于开启状态，则字符默认大写。如
*   果在键入字符的同时Shift处于按下状态，则反转输入字符的大小写
* 转发键盘输入消息时，若Shift同时按下，则应转发由Shift转换后的相应字符（如Shit+1转换为!)
* 亦即，键盘键入回调函数传入的参数应该是意图输入的那个字符 
*
*/

/*
*
* 附录2：快捷键格式
* 一个快捷键由 Ctrl、Alt、Shift 与某个字符定义，写作三个二进制数+字符的四位字符串
* 例如，Ctrl+Alt+E写为"110e"，Alt+TAB写为"010\t"，Ctrl+Shift+Enter写为"101\n"
* 字符默认小写，或者是对应键盘上不按shift时输入的那个键 
*
*/

#endif
