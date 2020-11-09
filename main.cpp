#include <string>
#include <iostream>
#include <cstdlib>    // 期望使用system函数：调用中断命令
#include <windows.h>  // 期望使用Beep函数  ：发声
#include <conio.h>    // 期望使用_kbhit和_getch函数：获取键盘输入

/* 声调频率 */
#define LOW_SI  494
#define DO      523
#define RE      587
#define MI      659
#define FA      698
#define SO      784
#define LA      880
#define SI      988
#define HIGH_DO 1046

/* 键码值 (ACSII) */
#define ESC 27
#define A   65
#define S   83
#define D   68
#define F   70
#define SPC 32  // 空格键
#define J   74
#define K   75
#define L   76
#define SEM 59  // ";"

// 琴键结构体
struct PianoKey {
    char upChar;  // 弹起状态的显示字符
    char dwChar;  // 按下状态的显示字符

    bool beeping;  // 是否处于发声状态

    unsigned toneFreq;  // 声调频率
    unsigned keyCode;   // 对应键盘码
};

/* 全局变量声明 */
PianoKey* g_currTargetPianoKey = nullptr;  // 指向当前需要被操作琴键的指针
/* 7个琴键 */
PianoKey g_lowSiKey  = { '1', '.', false, LOW_SI,  A };
PianoKey g_doKey     = { '2', '.', false, DO,      S };
PianoKey g_reKey     = { '3', '.', false, RE,      D };
PianoKey g_miKey     = { '4', '.', false, MI,      F };
PianoKey g_faKey     = { '5', '.', false, FA,      SPC };
PianoKey g_soKey     = { '6', '.', false, SO,      J };
PianoKey g_laKey     = { '7', '.', false, LA,      K };
PianoKey g_siKey     = { '8', '.', false, SI,      L };
PianoKey g_highDoKey = { '9', '.', false, HIGH_DO, SEM };

bool     g_gameRunning   = true;  // 说明游戏是否运行，为false游戏结束
// 当前帧是否需要刷新，Powershell没有双缓冲机制，因使用此变量限制刷新次数，防止过多屏闪
bool     g_needToRefresh = true;
unsigned g_beepDuration  = 150;  // 发生持续时长，单位毫秒
std::string g_pianoPanel = "---------------------------------------------------------------------------------\n"
                           "|                                                                               |\n"
                           "|                     [ A Piano Game developed by 2D_Cat ]                      |\n"
                           "|                                                                               |\n"
                           "|-------------------------------------------------------------------------------|\n"
                           "|___A___||___S___||___D___||___F___||_SPACE_||___J___||___K___||___L___||___;___|\n";
std::string g_pianoKeys  = "|       ||       ||       ||       ||       ||       ||       ||       ||       |\n"
                           "| 11111 || 22222 || 33333 || 44444 || 55555 || 66666 || 77777 || 88888 || 99999 |\n"
                           "| 11111 || 22222 || 33333 || 44444 || 55555 || 66666 || 77777 || 88888 || 99999 |\n"
                           "| 11111 || 22222 || 33333 || 44444 || 55555 || 66666 || 77777 || 88888 || 99999 |\n"
                           "| 11111 || 22222 || 33333 || 44444 || 55555 || 66666 || 77777 || 88888 || 99999 |\n"
                           "| 11111 || 22222 || 33333 || 44444 || 55555 || 66666 || 77777 || 88888 || 99999 |\n"
                           "| 11111 || 22222 || 33333 || 44444 || 55555 || 66666 || 77777 || 88888 || 99999 |\n"
                           "| 11111 || 22222 || 33333 || 44444 || 55555 || 66666 || 77777 || 88888 || 99999 |\n"
                           "| 11111 || 22222 || 33333 || 44444 || 55555 || 66666 || 77777 || 88888 || 99999 |\n"
                           "| 11111 || 22222 || 33333 || 44444 || 55555 || 66666 || 77777 || 88888 || 99999 |\n"
                           "|_______||_______||_______||_______||_______||_______||_______||_______||_______|\n";

/* 函数声明与定义 */

// 将目标琴键字符转换为弹起状态的字符
void changePianoKeyToDownChar(PianoKey* f_pk) {
    for (unsigned i = 0; i < g_pianoKeys.size(); ++i)
        if (g_pianoKeys[i] == f_pk->upChar) g_pianoKeys[i] = f_pk->dwChar;
}

// 将目标琴键字符转换为按下状态的字符
void changePianoKeyToUpChar(PianoKey* f_pk) {
    for (unsigned i = 0; i < g_pianoKeys.size(); ++i)
        if (g_pianoKeys[i] == f_pk->dwChar) g_pianoKeys[i] = f_pk->upChar;
}

// 处理键盘输入
void input(int f_ch) {
    // 检查输入事件，获取输入字符
    if (_kbhit()) f_ch = _getch();  // 有输入
    else          f_ch = -1;        // -1表示无输入

    PianoKey* pk;  // 指向当前输入所对应琴键的临时指针

    switch (toupper(f_ch)) {
        // 退出游戏
        case ESC:
            g_gameRunning = false;
            break;

        /* 临时指针指向对应琴键 */

        case A:
            pk = &g_lowSiKey;
            break;
        case S:
            pk = &g_doKey;
            break;
        case D:
            pk = &g_reKey;
            break;
        case F:
            pk = &g_miKey;
            break;
        case SPC:
            pk = &g_faKey;
            break;
        case J:
            pk = &g_soKey;
            break;
        case K:
            pk = &g_laKey;
            break;
        case L:
            pk = &g_siKey;
            break;
        case SEM:
            pk = &g_highDoKey;
            break;

        // 无输入时，临时指针指向nullptr
        case -1:
        default:
            pk = nullptr;
            break;
    }

    // 若用来指向当前目标琴键的全局指针为nullptr，则指向新目标琴键或nullptr
    if      (g_currTargetPianoKey == nullptr)                                   g_currTargetPianoKey = pk;
    // 若该指针已指向某个目标琴键并且该琴键没有发生，同样也让它指向新目标琴键，或nullptr
    else if (g_currTargetPianoKey != nullptr && !g_currTargetPianoKey->beeping) g_currTargetPianoKey = pk;
}

void update() {
    // 若指向目标琴键的指针非空，且琴键为未发声状态。（琴键按下时）
    if (g_currTargetPianoKey != nullptr && !g_currTargetPianoKey->beeping) {
        changePianoKeyToDownChar(g_currTargetPianoKey);
        g_needToRefresh = true;

    // 若指向目标琴键的指针非空，且琴键状态为正在发声。（琴键弹起时）
    } else if (g_currTargetPianoKey != nullptr && g_currTargetPianoKey->beeping) {
        changePianoKeyToUpChar(g_currTargetPianoKey);
        g_needToRefresh = true;

        g_currTargetPianoKey->beeping = false;    // 将目标琴键设为未发声
        g_currTargetPianoKey          = nullptr;  // 将当前目标琴键指针指向nullptr

    } else {
        g_needToRefresh = false;
    }
}

void render() {
    if (g_needToRefresh) {
        system("CLS");  // 清楚上一帧的屏幕内容
        std::cout << g_pianoPanel << g_pianoKeys << "\n\n";  // 显示当前状态下的钢琴
    }
}

void sound() {
    // 当存在需要操作的目标琴键，并且琴键未处在发声状态时
    if (g_currTargetPianoKey != nullptr && !g_currTargetPianoKey->beeping) {
        // 发声，参数是音调频率和持续时间。注意Beep函数会阻塞进程，因此无法发出和声
        Beep(g_currTargetPianoKey->toneFreq, g_beepDuration);
        g_currTargetPianoKey->beeping = true;  // 将目标琴键设为正在发声
    }
}

int main() {
    system("CLS");
    std::cout << g_pianoPanel << g_pianoKeys << "\n";

    int inChar = -1;

    while (g_gameRunning) {
        sound();

        input(inChar);
        update();
        render();
    }

    return 0;
}
