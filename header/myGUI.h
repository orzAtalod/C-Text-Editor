#ifndef __MY_GUI_H_INCLUDED__
#define __MY_GUI_H_INCLUDED__

/*
*
* interface for GUI system
* 
* ���������߼����£�
* 1. main�����ڸ��ֳ�ʼ��������ɵĽ�β���� GUI_Start() �������ҽ���������
*    �� GUI_Start() ������Ӧ����ɸ�����Ϣ�¼��ĵ�¼����������Ϣѭ��
* 2. ���ְ�ť������ĺ���ͨ����Ӧ�� register ������ GUI_start() ��������ǰ
*    ��ɳ�ʼ����GUIϵͳֻ������ʾ�ͼ򵥵���Ϣ���� 
* 3. ����ֻ�� GUI ��ʹ��ͼ�ο⺯�����������������Ҳ��ʹ��ͼ�ο⺯�����л�
*    �ƣ���ҪΪeditorGraphic.h������ GUI ִ�л��������ͬʱ����Ҫ������Ӧ
*    �ĺ���������Ӧ�������ϸ����棬����Ҫ�����������ĵ�
* 4. �߱������л����ܣ���ͬ�Ľ�����ѭ��ͬ����Ϣת���߼�������Ҫ���������
*    ���ĵ� 
* 5. �ڹ��ܲ���ʱ���ܻ�����������µİ�ť 
* 6. ���������Ҫ�����������Ӧ����**���Ͻ�**�����꣬�𴫳ɾ��������ˣ�Ҳ
*    �𴫳���������½ǵ����ꣻ�������ķ���� libgraphics �Ķ�����ͬ����
*    ��������Ϊ�� 
* 7. ������ʱ���ܹ����� GetClock() ������¼�Դ� GUI_Start() ����ִ�к�����
*    �ĺ����� 
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
* ���� 1�����༭���� 
* ���ϵ��·�Ϊ���У��ֱ�Ϊ�˵�������ť���ʹ�༭������༭��Ӧռ�󲿷����� 
* 
* �˵������������������ 
* 1. �ļ������桢�򿪡��½������Ϊ���ر� 
* 2. ���ߣ�ͳ�ơ���������� 
* 3. ���ڣ����á�����
*
* ��ť������һ�鰴ť�����ݷֱ����£����û�����ĺ��һ��˵����������ť��ֱ��ת����Ϣ 
* 1. ��ɫ����Ҫ�����л���������棨���� 2������ȡ������ɫ
* 2. �����ɫ��������ɫΪ�ϴ�ѡ��ķǺ�ɫ��ɫ��Ĭ��Ϊ��ɫ�� 
*    �����ɫ��ť��Ҫ��ʾĿǰ�Ŀ����ɫ��ʲô����������ı���ɫΪԤ�ڵĿ����ɫ�� 
* 4. �Ӵ�
* 5. б�� 
* 6. �ֺţ������л���������棨���� 2������ȡ�����ֺ� 
* 7. ���壬�����л���������棨���� 2������ȡ��������
* 8. ͼƬ����
* �����ť̫�ർ���Ű����ѣ����Կ���ȥ�� 2 ��һ����ť
*
* ��༭���������ҷ�Ϊ�������֣��ֱ�Ϊ��������༭���͹����������Ʒ����ֱ�Ϊ��
* �������ʹ��ע��� ExplorerDraw(double, double, double, double)��������������������Ͻ�
*         �����½�������ɻ��ơ� 
* �༭����ʹ��ע��� EditorGraphicDraw(double, double, double, double)����������༭����
*         ���Ͻ������½�������ɻ��ơ� 
* ��������ʹ��ע��� RollerUpperBound() �� RollerLowerBound() ������֪�������ֶε���ʼ
*         �����λ�ã����л��ƣ���ʼ������λ�ø�ʽΪռ�������ȵı������� 0 �� 1 ֮�䣩 
*
* ������¼�λ�ڱ༭��ʱ�����ߵ������¼�����ʱ����Ҫת��һЩ��Ϣ��
* 1. ��ݼ���Ϣ���� RegisterEditorHotKey(char*, ButtonEvent)���壬�����¿�ݼ�ʱ������Ӧ
*    ������ 
* 2. ����϶���Ϣ�������ָ��λ�ڱ༭���������������ʱ�������κ�����ƶ���Ϣ����ע���
*    EditorMouseLeftDown(double,double) ������굱ǰ����λ������ڱ༭�����Ͻǵ����ꡣ 
* 3. ���̧����Ϣ�������ָ���ɿ�ʱ���� EditorMouseLeftUp() �¼������޸ģ�����������Ƴ�
*    ���������Ϊ���ɿ���꣩ 
* 4. �Ҽ��������ͬ����������ͬʱ����ʱ��Ϊ���ɿ�ԭ�����ŵļ��ٰ����¼�
* 5. �м��Ĺ���Ҳ��Ҫ����¼ 
* 4. �����������ҡ�Enter��Backspace��Delete ��Ϣ��������Ӧ�������ɡ� 
* 5. ����������Ϣ�����ڼ���������Ϣ��˵����Ҫ�����¼ 1 �� 
*
* ����¼�λ���������������ʱҲ��Ҫת��һЩ��Ϣ��������༭����ͬ��������ͬ
* ������¼���Խ�������༭���Ľ��ޣ��ӱ༭���϶����������֮���򲻽�����¼�ת���� 
*   ��Խ������򣬶����Ծ�ת������Խǰ�������ճ���������������꣨��β��ڵ��������ˣ� 
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

void RegisterEditorKeyboardSpecial(KeyboardEventMethod); //1�� 2�� 3�� 4�� 5�س� 6�˸� 7Delete 

void RegisterExplorerMouseLeftDown(MouseEventMethod);
void RegisterExplorerMouseLeftUp(ButtonEvent);
void RegisterExplorerMouseRightDown(MouseEventMethod);
void RegisterExplorerMouseRightUp(ButtonEvent);

void RegisterRollerMouseLeftDown(MouseEventMethod);
void RegisterRollerMouseLeftUp(ButtonEvent);

/*
*
* ���� 2���������
* �����༭���������ͬ����������ĳ�������Ű�������������ڰ�Ŧ���·���ҳ����ײ�������
*   һ����������ͬʱ�����������༭����ص�ת���¼��� 
* ������Ӧ�����ؼ����ַ����루����¼ 1��ת��Ϊ����ĳ�����壨�ɸ��ģ���ʾ�������ݲ���¼ 
* ����������ĳ������ȷ�Ϻ��˳���ť�����س�Ҳ��ֱ��ȷ���������ݡ� 
* ȷ���������ݺ�Ӧ������Ӧ�ص���������������н�����Ӧ��ȡ��Ӧֵ��Ϊ��Ϣ������ 
* �˳������Ӧ�����������ݣ�������Ӧ�ص�����������ַ�������������н�����ִ���κ��ж�
* �����������������˵�����ѡ����߰�ť���а�ť��ͬ��ֱ���˳���ͬʱ������Ӧ��ť����Ҫ
*   ע���������༭���������޹�����Ӧ���ԣ���Ϊ��Ϣת���ѱ����á� 
*
*/

void ChangeDisplayMethodToInput(ButtonEventWithInput callback);

/*
*
* ���� 3(1)�������������
* ��������������ͬ�����������������Ե�����ʽ���֣����ƶ�����༭�����롣 
* ����Ӧ���������������Ϸ���ʾһ���ı��� 
* �༭����Ϣͬ�������ã�ͬʱ���Բ����Ʊ༭�����ݣ���Ϊ�в�����ܷ�ͬʱ��ʾ�༭��������
*   �������ݲ��Ҳ��໥���ӣ����extgraphic�е�erase��ط�����ʵ�ֲ��໥���ӣ��������
*   ʾ�༭���������ۡ� 
*
*/

void ChangeDisplayMethodToMajorInput(char* inputMessage, ButtonEventWithInput callback);

/*
*
* ���� 3(2)��������ť���� 
* �뵯��������������ͬ�����뷽ʽ�����ı�����ǰ�ť
* ����Ϊ����ʾ���ı�����ť������0/1��2����������ť�ı����ַ������0��ʼ��
* ����ֵΪ��ѡ��ť����ţ���Ŵ�1��ʼ������ֱ���˳��򷵻� 0 
*
*/

int ChangeDisplayMethodToMajorButton(char* inputMessage, int textNum, const char** textValue);

/*
*
* ���� 4���������棨��������湲�ã� 
* ����������˵����������������������ʾ��������
* �˵������������������༭������ͬ
* �������߼��뵯�����������ͬ������ע��������������������Ҫʵʱת����ÿ����һ�μ�����
*   һ����Ӧ�ص�������
* ���������ʾʹ���ⲿģ����ƣ��߼��������ⲿģ����ƵĽ�����ͬ
* ���������ʾ������Ҫת��������¼� 
* ͬʱ��Ҫ���˳���ť���˳��󷵻�ԭ���棨�������ʵ�ֿ��Ը�Ϊ�������༭���棩 
*
*/

void ChangeDisplayMethodToSearch();

void RegisterSearchDisplayMethod(AreaPaintMethod);
void RegisterSearchInputMethod(ButtonEventWithInput);

/*
*
* ���� 5��ͳ�ƽ���
* ����������˵�����ͳ������������֣��˵������������������༭������ͬ��ͳ�����
*   ���ⲿģ�����
* ����������˳���ť 
*
*/

void ChangeDisplayMethodToStat();

void RegisterStatDisplayMethod(AreaPaintMethod);

/*
*
* ���� 6�����ý��� 
* �����������������
* 1. ������ɫ��������ɫ����RGB�ĸ�������
* 2. �������������壬����һ��������������������
* 3. ����ѡ����� 
*
*/

void ChangeDisplayMethodToSetting();

void RegisterColorDifinitionMethod(ColorDefinitionMethod);

/*
*
* ��¼1�����������߼�
* �Ϸ��������ַ������������κο���ʾ�ַ����ո񣬻س���Ϊ�����
* ��һ�������¼�������Ϊ�ַ����룬���ҽ���û��Ctrl��Alt����һ���£����һ������Ӧ��
*   ��Ϊ��ݼ��������û����Ӧ�Ŀ�ݼ���Ӧ������
* ת������������Ϣʱ����Ҫ�����ַ��Ĵ�Сд�����CAPSLOCK���ڿ���״̬�����ַ�Ĭ�ϴ�д����
*   ���ڼ����ַ���ͬʱShift���ڰ���״̬����ת�����ַ��Ĵ�Сд
* ת������������Ϣʱ����Shiftͬʱ���£���Ӧת����Shiftת�������Ӧ�ַ�����Shit+1ת��Ϊ!)
* �༴�����̼���ص���������Ĳ���Ӧ������ͼ������Ǹ��ַ� 
*
*/

/*
*
* ��¼2����ݼ���ʽ
* һ����ݼ��� Ctrl��Alt��Shift ��ĳ���ַ����壬д��������������+�ַ�����λ�ַ���
* ���磬Ctrl+Alt+EдΪ"110e"��Alt+TABдΪ"010\t"��Ctrl+Shift+EnterдΪ"101\n"
* �ַ�Ĭ��Сд�������Ƕ�Ӧ�����ϲ���shiftʱ������Ǹ��� 
*
*/

#endif
