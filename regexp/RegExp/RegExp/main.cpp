#include <stdio.h>
#include <stdlib.h>
#include "regexp.h"

/********************************/
/*global variables*/
char *g_strRegExp;
int g_scan_pos = -1;   /*扫描指针*/
SymbolType g_symbol_type;   /*输入元素类型*/
int g_symbol_charvalue;  /*输入元素的字符值*/
int g_lastchar;    /*用这个备份当前字符,重复运算时就得备份*/
int g_repeat_m, g_repeat_n;   /*重复运算的上界和下界*/
pMachineStack g_mstk;   /*状态机调用堆栈*/
pStateTable g_st;   /*NFA表格*/
/*将要用到的临时变量*/
pState psnew;    /*新的状态指针*/
pMachine pmnew;    /*新的状态机指针*/
pStateTable pst_needed, pst_optional;   /*重复运算时返回的指针*/
char g_ele_ary[256];    /*用来做字母表有效置1,否则置0*/
int while_notfinish; /*表达式扫描循环条件*/

/*****************************猪头***/
/*void showStateTable(pStateTable pst);*/
int regexp_main();
SymbolType getSymbol();   /*词法分析函数,返回一个符号的类型和它的值*/
int dealState(SymbolType symboltype);  /*处理输入状态*/
void repeat_needed(int m, int eleindex);  /*{m,n}生成必选的m次循环状态表*/
void repeat_optional(int n, int eleindex);    /*{m,n}生成可选的n次循环状态表*/
pStateTable repeatMachine_needed(int m, pMachine pm);  /*A{m,n}生成必选的m次循环状态表*/
pStateTable repeatMachine_optional(int n, pMachine pm);    /*A{m,n}生成可选的n次循环状态表*/
int getASCII(char c); /*得到c的ASCII码*/
/********************************/
int main(int argc, char* argv[]) {
    /*测试,输出所有参数
     int i;
     for (i=0; i<argc; i++) {
     printf("debug: %d => %s\n", i, argv[i]);
     }
     */
    if (argc > 1) {
        g_strRegExp = argv[1];
    } else {
        g_strRegExp = "x(a[^\\S]b)+y";
        printf("Example: Shell>regexp \"%s\"\n", g_strRegExp);
    }
    /*
     strcpy(g_strRegExp, "xa{2,4}b{2}c{0,2}d{2,}y(a(bb)*a)+y");
     strcpy(g_strRegExp, "x(a){4,8}y");
     strcpy(g_strRegExp, "x(a(bb){2}a){1,4}y");
     strcpy(g_strRegExp, "x((ab)*|(bc)+)y");
     strcpy(g_strRegExp, "x[^(a)b]+y");
     strcpy(g_strRegExp, "x{2,5}(ab+|ac*){10,12}y\\x30\\001.{30,}\\xg{20}[^abc]?\\d\\D");
     strcpy(g_strRegExp, "[^\\S]");
     */
    
    printf("debug: JUST A TEST BEGIN...\n");
    regexp_main();
    printf("debug: JUST A TEST END...\n");
    return 0;
}

/*核心解析函数: 语法分析部分*/
int regexp_main() {
    /*初始化 g_st & g_mstk*/
    g_st = newStateTable(256);  /*0-255, 256是epsilon(空)*/
    appendStateTable(g_st, newState()); /*开始状态*/
    g_st->curState = g_st->head;
    g_mstk = newMachineStack();  /*堆栈空*/
    
    g_scan_pos = -1;
    g_symbol_type = START_REGEXP;
    while_notfinish = 1; /*表达式扫描循环条件,在END_REGEXP中会置0*/
    while (while_notfinish) {
        if (dealState(g_symbol_type) == 1) {
            /*下一个符号,在前面的switch()中,
             有些预先读取一个词的分枝会用continue语句会跳过这里*/
            getSymbol();
        }
    }
    
    /*输出StateTable,测试用*/
    showStateTable(g_st);
    destroyStateTable(g_st);
    destroyMachineStack(g_mstk);
    /*
     printf("debug: press anykey to continue ...\n");
     getch();
     */
    return 0;
}
/********************************/
/*状态机输入响应部分,定义了各个状态的具体行为*/
int dealState(SymbolType symboltype) {
    int OR_MACHINE_i;
    switch (symboltype) {
        case DOT:
        case NUMBER:
        case NOT_NUMBER:
        case AZaz09_:
        case NOT_AZaz09_:
        case ALL_SPACE:
        case NOT_ALL_SPACE:
            
            /*模拟输入字母表中的每个元素*/
            pmnew = newMachine(NULL, NULL, g_scan_pos, -1, '|', NULL);
            pmnew->state_start = newState();    /*状态机开始状态*/
            pmnew->state_end = newState();    /*状态机开始状态*/
            /*将开始状态与当前状态连接*/
            appendStateTable(g_st, pmnew->state_start);    /*添加到状态表格*/
            setdestState(g_st->curState, g_st->eleCount, pmnew->state_start);
            g_st->curState = pmnew->state_start; /*移动到新的状态*/
            
            /*调用这一个状态机,将它的断点,运算模式压入堆栈*/
            pushMachine(g_mstk, pmnew);
            
            /*将这些分支加入到这个子状态机*/
            if (symboltype == DOT) {
                for (OR_MACHINE_i=0; OR_MACHINE_i<(int)'\n'; OR_MACHINE_i++) {
                    g_ele_ary[OR_MACHINE_i] = 1;
                }
                for (OR_MACHINE_i=(int)'\n' + 1; OR_MACHINE_i<=sizeof(g_ele_ary); OR_MACHINE_i++) {
                    g_ele_ary[OR_MACHINE_i] = 1;
                }
            }
            
            if (symboltype == NUMBER) {
                for (OR_MACHINE_i=0; OR_MACHINE_i<sizeof(g_ele_ary); OR_MACHINE_i++) {
                    if (OR_MACHINE_i < '0' || OR_MACHINE_i > '9') {
                        g_ele_ary[OR_MACHINE_i] = 0;
                    } else {
                        g_ele_ary[OR_MACHINE_i] = 1;
                    }
                }
            }
            
            if (symboltype == NOT_NUMBER) {
                for (OR_MACHINE_i=0; OR_MACHINE_i<sizeof(g_ele_ary); OR_MACHINE_i++) {
                    if (OR_MACHINE_i < '0' || OR_MACHINE_i > '9') {
                        g_ele_ary[OR_MACHINE_i] = 1;
                    } else {
                        g_ele_ary[OR_MACHINE_i] = 0;
                    }
                }
            }
            
            if (symboltype == AZaz09_) {
                for (OR_MACHINE_i=0; OR_MACHINE_i<sizeof(g_ele_ary); OR_MACHINE_i++) {
                    if ((OR_MACHINE_i>='0' && OR_MACHINE_i<='9')
                        || (OR_MACHINE_i>='A' && OR_MACHINE_i<='Z')
                        || (OR_MACHINE_i>='a' && OR_MACHINE_i<='z')) {
                        g_ele_ary[OR_MACHINE_i] = 1;
                    } else {
                        g_ele_ary[OR_MACHINE_i] = 0;
                    }
                }
            }
            
            if (symboltype == NOT_AZaz09_) {
                for (OR_MACHINE_i=0; OR_MACHINE_i<sizeof(g_ele_ary); OR_MACHINE_i++) {
                    if ((OR_MACHINE_i>='0' && OR_MACHINE_i<='9')
                        || (OR_MACHINE_i>='A' && OR_MACHINE_i<='Z')
                        || (OR_MACHINE_i>='a' && OR_MACHINE_i<='z')) {
                        g_ele_ary[OR_MACHINE_i] = 0;
                    } else {
                        g_ele_ary[OR_MACHINE_i] = 1;
                    }
                }
            }
            
            if (symboltype == ALL_SPACE) {
                for (OR_MACHINE_i=0; OR_MACHINE_i<sizeof(g_ele_ary); OR_MACHINE_i++) {
                    if (OR_MACHINE_i==' ' || OR_MACHINE_i=='\n'
                        || OR_MACHINE_i=='\r' || OR_MACHINE_i=='\t'
                        || OR_MACHINE_i=='\f' || OR_MACHINE_i=='\v') {
                        g_ele_ary[OR_MACHINE_i] = 1;
                    } else {
                        g_ele_ary[OR_MACHINE_i] = 0;
                    }
                }
            }
            
            if (symboltype == NOT_ALL_SPACE) {
                for (OR_MACHINE_i=0; OR_MACHINE_i<sizeof(g_ele_ary); OR_MACHINE_i++) {
                    if (OR_MACHINE_i==' ' || OR_MACHINE_i=='\n'
                        || OR_MACHINE_i=='\r' || OR_MACHINE_i=='\t'
                        || OR_MACHINE_i=='\f' || OR_MACHINE_i=='\v') {
                        g_ele_ary[OR_MACHINE_i] = 0;
                    } else {
                        g_ele_ary[OR_MACHINE_i] = 1;
                    }
                }
            }
            
            return dealState(OR_MACHINE_END);
            break;
        case BACKTRACE:
            /*将当前状态与子状态机的结束状态连接*/
            setdestState(g_st->curState, (int)g_st->eleCount, g_mstk->topM->state_end);
            /*将当前状态回退到子状态机的头部,开始一个新的分支*/
            g_st->curState = g_mstk->topM->state_start;
            break;
        case INPUT_ELE:
            /*是一个终结符,连上一条弧*/
            g_lastchar = g_symbol_charvalue;
            psnew = newState();
            appendStateTable(g_st, psnew);  /*添加到状态表*/
            setdestState(g_st->curState, (int)g_symbol_charvalue, psnew);
            
            /*重复运算符*/
            getSymbol();
            switch (g_symbol_type) {
                case REPEAT_ZERO_MORE:
                    /*(a*)*/
                    setdestState(g_st->curState, g_st->eleCount, psnew);
                    setdestState(psnew, g_st->eleCount, g_st->curState);
                    /*移动到新的状态*/
                    g_st->curState = psnew;
                    printf("debug: ZEOR_MORE\n");
                    break;
                case REPEAT_ZERO_ONCE:
                    /*(a?)*/
                    setdestState(g_st->curState, g_st->eleCount, psnew);
                    /*移动到新的状态*/
                    g_st->curState = psnew;
                    printf("debug: ZEOR_ONCE\n");
                    break;
                case REPEAT_ONCE_MORE:
                    /*(a+)*/
                    setdestState(psnew, g_st->eleCount, g_st->curState);
                    /*移动到新的状态*/
                    g_st->curState = psnew;
                    printf("debug: ONCE_MORE\n");
                    break;
                case REPEAT_RANGE_MN:
                    /*(a{m,n})*/
                    if (g_repeat_m <= 0) {
                        /*处理{0,n}的情况,把刚刚的a变成(a?)*/
                        setdestState(g_st->curState, g_st->eleCount, psnew);
                        /*移动到新的状态*/
                        g_st->curState = psnew;
                        /*重复n-1次(a?)*/
                        repeat_optional(g_repeat_n-1, g_lastchar);
                    } else {
                        /*移动到新的状态*/
                        g_st->curState = psnew;
                        repeat_needed(g_repeat_m-1 , g_lastchar);
                        repeat_optional(g_repeat_n-g_repeat_m, g_lastchar);
                    }
                    printf("debug: type -> %d\t|  m -> %d  |  n -> %d\n", symboltype, g_repeat_m, g_repeat_n);
                    break;
                case REPEAT_RANGE_M_MORE:
                    /*(a{m,})*/
                    /*移动到新的状态*/
                    g_st->curState = psnew;
                    repeat_needed(g_repeat_m-1, g_lastchar);
                    /*做一个（a*）*/
                    psnew = newState();
                    appendStateTable(g_st, psnew);
                    setdestState(g_st->curState, g_lastchar, psnew);
                    setdestState(g_st->curState, g_st->eleCount, psnew);
                    setdestState(psnew, g_st->eleCount, g_st->curState);
                    /*移动新的状态*/
                    g_st->curState = psnew;
                    printf("debug: type -> %d\t|  m -> %d  |  n -> max_int\n", symboltype, g_repeat_m);
                    break;
                case REPEAT_RANGE_M:
                    /*(a{m})*/
                    /*移动到新的状态*/
                    g_st->curState = psnew;
                    repeat_needed(g_repeat_m-1, g_lastchar);
                    printf("debug: type -> %d\t|  m -> %d\n", symboltype, g_repeat_m);
                    break;
                default:
                    /*不是重复运算符,移动到新的状态,然后重新循环*/
                    g_st->curState = psnew;
                    return 0;
            }/*重复运算符 -- 结束*/
            
            break;/*INPUT_ELE -- 结束*/
        case START_REGEXP:
            printf("debug: start scan the regexp:\n\t\"%s\"\n", g_strRegExp);
            /*做一次状态机开始操作*/
        case AND_MACHINE_BEGIN:
            /*开始一个子状态机,左括号: '('*/
            pmnew = newMachine(NULL, NULL, g_scan_pos, -1, '.', NULL);
            pmnew->state_start = newState();    /*状态机开始状态*/
            pmnew->state_end = newState();    /*状态机开始状态*/
            /*将开始状态与当前状态连接*/
            appendStateTable(g_st, pmnew->state_start);    /*添加到状态表格*/
            setdestState(g_st->curState, g_st->eleCount, pmnew->state_start);
            g_st->curState = pmnew->state_start; /*移动到新的状态*/
            
            /*调用这一个状态机,将它的断点,运算模式压入堆栈*/
            pushMachine(g_mstk, pmnew);
            break;    /*AND_MACHINE_BEGIN -- 结束*/
        case OR_MACHINE_BEGIN:
            /*开始一个或运算子状态机(字母表),左括号: '['*/
            pmnew = newMachine(NULL, NULL, g_scan_pos, -1, '|', NULL);
            pmnew->state_start = newState();    /*状态机开始状态*/
            pmnew->state_end = newState();    /*状态机开始状态*/
            /*将开始状态与当前状态连接*/
            appendStateTable(g_st, pmnew->state_start);    /*添加到状态表格*/
            setdestState(g_st->curState, g_st->eleCount, pmnew->state_start);
            g_st->curState = pmnew->state_start; /*移动到新的状态*/
            
            /*调用这一个状态机,将它的断点,运算模式压入堆栈*/
            pushMachine(g_mstk, pmnew);
            
            getSymbol();
            /*处理子状态机的否定模式*/
            if (g_symbol_type == NOT_OP) {
                g_mstk->topM->not_mode = '^';
                getSymbol();    /*继续下一个符号*/
            }
            /*初始化*/
            for (OR_MACHINE_i=0; OR_MACHINE_i<sizeof(g_ele_ary); OR_MACHINE_i++) {
                g_ele_ary[OR_MACHINE_i] = 0;
            }
            while (g_symbol_type != OR_MACHINE_END && g_symbol_type != END_REGEXP) {
                switch (g_symbol_type) {
                    case INPUT_ELE:
                        g_ele_ary[(int)g_symbol_charvalue] = 1;
                        /*下面处理一下[a-z]这种形式*/
                        g_lastchar = g_symbol_charvalue;    /*备份开始元素*/
                        /*检查下一个元素是不是LETTER_RANGE*/
                        getSymbol();
                        if (g_symbol_type == LETTER_RANGE) {
                            /*是一个连接符,检查下一个字符是不是INPUT_ELE*/
                            getSymbol();
                            if (g_symbol_type == INPUT_ELE) {
                                /*已经找到一个完整连接符语法,填充这个字母表范围*/
                                OR_MACHINE_i = (g_lastchar<g_symbol_charvalue) ? (int)g_lastchar : (int)g_symbol_charvalue;
                                if (g_lastchar<=g_symbol_charvalue) {
                                    g_lastchar = g_symbol_charvalue;
                                }
                                for (; OR_MACHINE_i<=(int)g_lastchar; OR_MACHINE_i++) {
                                    g_ele_ary[OR_MACHINE_i] = 1;
                                }
                            } else {
                                /*连接符后面找不到INPUT_ELE*/
                                /*忽略这个错误的连接符语法,继续循环*/
                                continue;
                            }
                        } else {
                            /*不是一个连接符*/
                            continue;
                        }
                        break;
                    case DOT:
                        for (OR_MACHINE_i=0; OR_MACHINE_i<(int)'\n'; OR_MACHINE_i++) {
                            g_ele_ary[OR_MACHINE_i] = 1;
                        }
                        for (OR_MACHINE_i=(int)'\n' + 1; OR_MACHINE_i<=sizeof(g_ele_ary); OR_MACHINE_i++) {
                            g_ele_ary[OR_MACHINE_i] = 1;
                        }
                        break;
                    case NUMBER:
                        for (OR_MACHINE_i=(int)'0'; OR_MACHINE_i<=(int)'9'; OR_MACHINE_i++) {
                            g_ele_ary[OR_MACHINE_i] = 1;
                        }
                        break;
                    case NOT_NUMBER:
                        for (OR_MACHINE_i=0; OR_MACHINE_i<=(int)'0'; OR_MACHINE_i++) {
                            g_ele_ary[OR_MACHINE_i] = 1;
                        }
                        for (OR_MACHINE_i=(int)'9' + 1; OR_MACHINE_i<=sizeof(g_ele_ary); OR_MACHINE_i++) {
                            g_ele_ary[OR_MACHINE_i] = 1;
                        }
                        break;
                    case AZaz09_:
                        for (OR_MACHINE_i=(int)'A'; OR_MACHINE_i<=(int)'Z'; OR_MACHINE_i++) {
                            g_ele_ary[OR_MACHINE_i] = 1;
                        }
                        for (OR_MACHINE_i=(int)'a'; OR_MACHINE_i<=(int)'z'; OR_MACHINE_i++) {
                            g_ele_ary[OR_MACHINE_i] = 1;
                        }
                        for (OR_MACHINE_i=(int)'0'; OR_MACHINE_i<=(int)'9'; OR_MACHINE_i++) {
                            g_ele_ary[OR_MACHINE_i] = 1;
                        }
                        g_ele_ary[(int)'_'] = 1;
                        break;
                    case NOT_AZaz09_:
                        for (OR_MACHINE_i=0; OR_MACHINE_i<(int)'0'; OR_MACHINE_i++) {
                            g_ele_ary[OR_MACHINE_i] = 0;
                        }
                        for (OR_MACHINE_i=(int)'9' + 1; OR_MACHINE_i<(int)'A'; OR_MACHINE_i++) {
                            g_ele_ary[OR_MACHINE_i] = 0;
                        }
                        for (OR_MACHINE_i=(int)'Z' + 1; OR_MACHINE_i<(int)'_'; OR_MACHINE_i++) {
                            g_ele_ary[OR_MACHINE_i] = 0;
                        }
                        for (OR_MACHINE_i=(int)'_' + 1; OR_MACHINE_i<(int)'a'; OR_MACHINE_i++) {
                            g_ele_ary[OR_MACHINE_i] = 0;
                        }
                        for (OR_MACHINE_i=(int)'z' + 1; OR_MACHINE_i<sizeof(g_ele_ary); OR_MACHINE_i++) {
                            g_ele_ary[OR_MACHINE_i] = 0;
                        }
                        break;
                    case ALL_SPACE:
                        for (OR_MACHINE_i=0; OR_MACHINE_i<sizeof(g_ele_ary); OR_MACHINE_i++) {
                            if (OR_MACHINE_i==' ' || OR_MACHINE_i=='\n'
                                || OR_MACHINE_i=='\r' || OR_MACHINE_i=='\t'
                                || OR_MACHINE_i=='\f' || OR_MACHINE_i=='\v') {
                                g_ele_ary[OR_MACHINE_i] = 1;
                            }
                        }
                        break;
                    case NOT_ALL_SPACE:
                        for (OR_MACHINE_i=0; OR_MACHINE_i<sizeof(g_ele_ary); OR_MACHINE_i++) {
                            if (OR_MACHINE_i==' ' || OR_MACHINE_i=='\n'
                                || OR_MACHINE_i=='\r' || OR_MACHINE_i=='\t'
                                || OR_MACHINE_i=='\f' || OR_MACHINE_i=='\v') {
                                ;
                            } else {
                                g_ele_ary[OR_MACHINE_i] = 1;
                            }
                        }
                        break;
                    default:
                        break;
                }
                
                getSymbol();
            }
            if (g_symbol_type == END_REGEXP) {
                dealState(OR_MACHINE_END);
                g_symbol_type = END_REGEXP;
            }
            return 0;   /*通知调用者不必再getSymbol(),这里还有一个未处理*/
            break;    /*AND_MACHINE_BEGIN -- 结束*/
        case END_REGEXP:
            while_notfinish = 0; /*一会儿退出循环*/
            printf("debug: end scan the regexp:\n\t\"%s\"\n", g_strRegExp);
            /*做一次状态机结束操作*/
        case OR_MACHINE_END:
            if (symboltype == OR_MACHINE_END) {
                /*将g_ele_ary[]连接到OR_MACHINE中*/
                for (OR_MACHINE_i=0; OR_MACHINE_i<sizeof(g_ele_ary); OR_MACHINE_i++) {
                    if (g_mstk->topM->not_mode == '^') {
                        /*否定模式*/
                        if (g_ele_ary[OR_MACHINE_i] == 0) {
                            setdestState(g_mstk->topM->state_start, OR_MACHINE_i, g_mstk->topM->state_end);
                        }
                    } else {
                        if (g_ele_ary[OR_MACHINE_i] == 1) {
                            setdestState(g_mstk->topM->state_start, OR_MACHINE_i, g_mstk->topM->state_end);
                        }
                    }
                }
                g_st->curState = g_mstk->topM->state_end;
            }
        case AND_MACHINE_END:
            /*关闭一个子状态机,右括号: ')'*/
            /*得到当前状态机参数*/
            pmnew = g_mstk->topM;
            /*将状态机的结束状态加入状态表*/
            appendStateTable(g_st, pmnew->state_end);
            /*结束当前状态机: 最后一个状态连到状态机的结束状态*/
            setdestState(g_st->curState, g_st->eleCount, pmnew->state_end);
            g_st->curState = pmnew->state_end;  /*移动到新的状态*/
            if (symboltype != END_REGEXP) {
                /*如果表达式没有结束,向后看一个词,判断是不是重复运算,
                 如果是则执行相应的重复运算,否则继续解析表达式*/
                getSymbol();
            }
            switch (g_symbol_type) {
                case REPEAT_ZERO_MORE:
                    /*(A*)*/
                    /*将状态机开始和结束的状态做epsilon连接*/
                    setdestState(pmnew->state_start, g_st->eleCount, pmnew->state_end);
                    setdestState(pmnew->state_end, g_st->eleCount, pmnew->state_start);
                    printf("debug: (A*)ZEOR_MORE\tstart: %x | end: %x\n", pmnew->state_start, pmnew->state_end);
                    break;
                case REPEAT_ZERO_ONCE:
                    /*(A?)*/
                    setdestState(pmnew->state_start, g_st->eleCount, pmnew->state_end);
                    printf("debug: (A?)ZEOR_ONCE\n");
                    break;
                case REPEAT_ONCE_MORE:
                    /*(A+)*/
                    setdestState(pmnew->state_end, g_st->eleCount, pmnew->state_start);
                    printf("debug: (A+)ONCE_MORE\n");
                    break;
                case REPEAT_RANGE_MN:
                    /*(A{m,n})*/
                    if (g_repeat_m <= 0) {
                        /*处理{0,n}的情况,把刚刚的A变成(A?)*/
                        setdestState(pmnew->state_start, g_st->eleCount, pmnew->state_end);
                        /*重复n-1次(A?)*/
                        pst_optional = repeatMachine_optional(g_repeat_n-1, pmnew);
                        if (pst_optional != NULL) {
                            g_st = joinStateTable(g_st, pst_optional);
                            pst_optional->head = NULL;
                            pst_optional->tail = NULL;
                            pst_optional->curState = NULL;
                            destroyStateTable(pst_optional);
                        }
                    } else {
                        pst_needed = repeatMachine_needed(g_repeat_m-1 , pmnew);
                        pst_optional = repeatMachine_optional(g_repeat_n-g_repeat_m, pmnew);
                        if (pst_needed != NULL) {
                            g_st = joinStateTable(g_st, pst_needed);
                            
                            pst_needed->head = NULL;
                            pst_needed->tail = NULL;
                            pst_needed->curState = NULL;
                            destroyStateTable(pst_needed);
                        }
                        if (pst_optional != NULL) {
                            g_st = joinStateTable(g_st, pst_optional);
                            pst_optional->head = NULL;
                            pst_optional->tail = NULL;
                            pst_optional->curState = NULL;
                            destroyStateTable(pst_optional);
                        }
                    }
                    printf("debug: type -> %d\t|  m -> %d  |  n -> %d\n", symboltype, g_repeat_m, g_repeat_n);
                    break;
                case REPEAT_RANGE_M_MORE:
                    /*(A{m,})*/
                    pst_needed = repeatMachine_needed(g_repeat_m-1, pmnew);
                    if (pst_needed != NULL) {
                        g_st = joinStateTable(g_st, pst_needed);
                        pst_needed->head = NULL;
                        pst_needed->tail = NULL;
                        pst_needed->curState = NULL;
                        destroyStateTable(pst_needed);
                    }
                    /*做一个(A*)*/
                    psnew = g_st->curState; /*保存当前状态*/
                    pst_optional = repeatMachine_optional(1, pmnew);   /*做一个(A?)*/
                    if (pst_optional != NULL) {
                        g_st = joinStateTable(g_st, pst_optional);
                        pst_optional->head = NULL;
                        pst_optional->curState = NULL;
                        pst_optional->tail = NULL;
                        destroyStateTable(pst_optional);
                    }
                    psnew = psnew->next;    /*最后一次重复运算的开始状态*/
                    setdestState(psnew, g_st->eleCount, g_st->curState);    /*修改成(A*)*/
                    printf("debug: type -> %d\t|  m -> %d  |  n -> max_int\n", symboltype, g_repeat_m);
                    break;
                case REPEAT_RANGE_M:
                    /*(A{m})*/
                    pst_needed = repeatMachine_needed(g_repeat_m-1, pmnew);
                    if (pst_needed != NULL) {
                        g_st = joinStateTable(g_st, pst_needed);
                        pst_needed->head = NULL;
                        pst_needed->tail = NULL;
                        pst_needed->curState = NULL;
                        destroyStateTable(pst_needed);
                    }
                    printf("debug: type -> %d\t|  m -> %d\n", symboltype, g_repeat_m);
                    break;
                default:
                    /*不是重复运算符,从堆栈中弹出当前状态机,然后重新循环*/
                    pmnew = popMachine(g_mstk);
                    destroyMachine(pmnew);
                    return 0;
            }/*重复运算符 -- 结束*/
            
            pmnew = popMachine(g_mstk);
            destroyMachine(pmnew);
            break;
        case UNKNOWN:
            while_notfinish = 0;
            printf("debug: UNKNOWN  --   ######################");
            break;
        default:
            printf("debug: type -> %d\t|  char -> %c\n", symboltype, g_symbol_charvalue);
            break;
    }
    
    return 1;    /*通知调用者传递下一个符号,即:调用get_symbol()*/
}
/********************************/
/*{m,n}生成必选的m次循环状态表*/
void repeat_needed(int m, int eleindex) {
    pState psnew;
    int i;
    for (i=0; i<m; i++) {
        psnew = newState();
        appendStateTable(g_st, psnew);
        /*连接运算*/
        setdestState(g_st->curState, eleindex, psnew);
        g_st->curState = psnew;
    }
}
/********************************/
/*{m,n}生成可选的n次循环状态表*/
void repeat_optional(int n, int eleindex) {
    pState psnew;
    int i;
    for (i=0; i<n; i++) {
        psnew = newState();
        appendStateTable(g_st, psnew);
        /*做1次ZERO_ONCE运算*/
        setdestState(g_st->curState, eleindex, psnew);
        setdestState(g_st->curState, g_st->eleCount, psnew);
        g_st->curState = psnew;
    }
}
/********************************/
/*{m,n}生成必选的m次循环状态表*/
pStateTable repeatMachine_needed(int m, pMachine pm) {
    pStateTable pst = NULL, psttmp = NULL;
    int i;
    /*将重复m次生成的表放在一起，组成一个子表*/
    for (i=0; i<m; i++) {
        psttmp = cloneSubStateTable(g_st, pm->state_start, pm->state_end);
        if (psttmp != NULL) {
            psttmp->tail->next = NULL;
            
            pst = joinStateTable(pst, psttmp);
            /*释放这个临时的状态表的空间*/
            psttmp->head = NULL;
            /*将头指针断开,否则destroy操作会释放他所指向的状态链表的空间*/
            psttmp->tail = NULL;
            psttmp->curState = NULL;
            destroyStateTable(psttmp);
        }
    }
    return pst;
    /*将这个包含m次的大子表与g_st合并*/
    g_st = joinStateTable(g_st, pst);
    /*释放这个临时的状态表的空间*/
    pst->head = NULL;
    /*将头指针断开,否则destroy操作会释放他所指向的状态链表的空间*/
    pst->tail = NULL;
    pst->curState = NULL;
    destroyStateTable(pst);
}
/********************************/
/*{m,n}生成可选的n次循环状态表*/
pStateTable repeatMachine_optional(int n, pMachine pm) {
    pStateTable pst = NULL, psttmp = NULL;
    int i;
    /*将重复n次生成的表放在一起，组成一个子表*/
    for (i=0; i<n; i++) {
        psttmp = cloneSubStateTable(g_st, pm->state_start, pm->state_end);
        if (psttmp != NULL) {
            psttmp->tail->next = NULL;
            /*对临时表做一次(A?)运算*/
            setdestState(psttmp->head, g_st->eleCount, psttmp->tail);
            
            pst = joinStateTable(pst, psttmp);
            /*释放这个临时的状态表的空间*/
            psttmp->head = NULL;
            /*将头指针断开,否则destroy操作会释放他所指向的状态链表的空间*/
            psttmp->tail = NULL;
            psttmp->curState = NULL;
            destroyStateTable(psttmp);
        }
    }
    return pst;
    /*将这个包含m次的大子表与g_st合并*/
    g_st = joinStateTable(g_st, pst);
    /*释放这个临时的状态表的空间*/
    /*将头指针断开,否则destroy操作会释放他所指向的状态链表的空间*/
    pst->head = NULL;
    pst->tail = NULL;
    pst->curState = NULL;
    destroyStateTable(pst);
}
/********************************/
/*词法分析,取得一个符号*/
SymbolType getSymbol() {
    int *cc = &g_symbol_charvalue;
    char tmp; /*临时变量*/
    
    *cc = getASCII(g_strRegExp[++g_scan_pos]);
    /*正则表达式结尾*/
    if (*cc == NULL) {return g_symbol_type=END_REGEXP;}
    if (*cc == '(') {return g_symbol_type=AND_MACHINE_BEGIN;}
    if (*cc == ')') {return g_symbol_type=AND_MACHINE_END;}
    if (*cc == '[') {return g_symbol_type=OR_MACHINE_BEGIN;}
    if (*cc == ']') {return g_symbol_type=OR_MACHINE_END;}
    if (*cc == '|') {return g_symbol_type=BACKTRACE;}
    if (*cc == '^') {return g_symbol_type=NOT_OP;}
    if (*cc == '.') {return g_symbol_type=DOT;}
    if (*cc == '-') {return g_symbol_type=LETTER_RANGE;}
    /*转义字符*/
    if (*cc == '\\') {
        *cc = getASCII(g_strRegExp[++g_scan_pos]);
        if (*cc == 'd') {return g_symbol_type=NUMBER;}
        if (*cc == 'D') {return g_symbol_type=NOT_NUMBER;}
        if (*cc == 'f') {*cc = '\f'; return g_symbol_type=INPUT_ELE;}
        if (*cc == 'n') {*cc = '\n'; return g_symbol_type=INPUT_ELE;}
        if (*cc == 'r') {*cc = '\r'; return g_symbol_type=INPUT_ELE;}
        if (*cc == 't') {*cc = '\t'; return g_symbol_type=INPUT_ELE;}
        if (*cc == 'v') {*cc = '\v'; return g_symbol_type=INPUT_ELE;}
        if (*cc == 's') {return g_symbol_type=ALL_SPACE;}
        if (*cc == 'S') {return g_symbol_type=NOT_ALL_SPACE;}
        if (*cc == 'w') {return g_symbol_type=AZaz09_;}
        if (*cc == 'W') {return g_symbol_type=NOT_AZaz09_;}
        /*16进制*/
        if (*cc == 'x') {
            tmp = getASCII(g_strRegExp[++g_scan_pos]);
            if (tmp>='0' && tmp<='9') {
                *cc = tmp - '0';
            } else if (tmp>='a' && tmp<='f') {
                *cc = tmp - 'a' + 10;
            } else if (tmp>='A' && tmp<='F') {
                *cc = tmp - 'A' + 10;
            } else {
                /*不是16进制数字,返回原字符'x'*/
                g_scan_pos--;
                return g_symbol_type=INPUT_ELE;
            }
            tmp = getASCII(g_strRegExp[++g_scan_pos]);
            if (tmp>='0' && tmp<='9') {
                *cc *= 16;
                *cc += tmp - '0';
            } else if (tmp>='a' && tmp<='f') {
                *cc *= 16;
                *cc += tmp - 'a' + 10;
            } else if (tmp>='A' && tmp<='F') {
                *cc *= 16;
                *cc += tmp - 'A' + 10;
            } else {
                /*这个不是16进制数字,只有一位:\xF*/
                g_scan_pos--;
                return g_symbol_type=INPUT_ELE;
            }
            
            return g_symbol_type=INPUT_ELE;
        }
        /*8进制*/
        /*以0-3开头的8进制数可以有3位*/
        if (*cc>='0' && *cc<='3') {
            *cc -= '0';
            tmp = getASCII(g_strRegExp[++g_scan_pos]);
            if (tmp>='0' && tmp<='7') {
                *cc *= 8;
                *cc += tmp - '0';
            } else {
                /*只有一位8进制数字:\7*/
                g_scan_pos--;
                return g_symbol_type=INPUT_ELE;
            }
            tmp = getASCII(g_strRegExp[++g_scan_pos]);
            if (tmp>='0' && tmp<='7') {
                *cc *= 8;
                *cc += tmp - '0';
            } else {
                /*只有两位8进制数字:\77*/
                g_scan_pos--;
                return g_symbol_type=INPUT_ELE;
            }
            return g_symbol_type=INPUT_ELE;
        }
        /*以4-7开头的8进制可以有2位*/
        if (*cc>='4' && *cc<='7') {
            *cc -= '0';
            tmp = getASCII(g_strRegExp[++g_scan_pos]);
            if (tmp>='0' && tmp<='7') {
                *cc *= 8;
                *cc += tmp - '0';
            } else {
                /*只有一位8进制数字:\7*/
                g_scan_pos--;
                return g_symbol_type=INPUT_ELE;
            }
            return g_symbol_type=INPUT_ELE;
        } else {
            /*[^x0-7dDfnrtvsSwW]*/
            return g_symbol_type=INPUT_ELE;
        }
    }
    /*重复运算*/
    if (*cc == '*') {return g_symbol_type=REPEAT_ZERO_MORE;}
    if (*cc == '+') {return g_symbol_type=REPEAT_ONCE_MORE;}
    if (*cc == '?') {return g_symbol_type=REPEAT_ZERO_ONCE;}
    if (*cc == '{') {
        *cc = getASCII(g_strRegExp[++g_scan_pos]);
        if (*cc>'9' || *cc<'0') {return g_symbol_type=UNKNOWN;}
        g_repeat_m = 0;   /*取{m,n}的m*/
        while (*cc>='0' && *cc<='9') {
            g_repeat_m *= 10;
            g_repeat_m += *cc-'0';
            *cc = getASCII(g_strRegExp[++g_scan_pos]);
        }
        /*{m}*/
        if (*cc == '}') {return g_symbol_type=REPEAT_RANGE_M;}
        if (*cc != ',') {return g_symbol_type=UNKNOWN;}
        
        *cc = getASCII(g_strRegExp[++g_scan_pos]);
        /*{m,}*/
        if (*cc == '}') {return g_symbol_type=REPEAT_RANGE_M_MORE;}
        /*{m,n}*/
        if (*cc>'9' || *cc<'0') {return g_symbol_type=UNKNOWN;}
        g_repeat_n = 0;   /*取{m,n}的n*/
        while (*cc>='0' && *cc<='9') {
            g_repeat_n *= 10;
            g_repeat_n += *cc-'0';
            *cc = getASCII(g_strRegExp[++g_scan_pos]);
        }
        if (*cc == '}') {
            return g_symbol_type=REPEAT_RANGE_MN;
        } else {
            return g_symbol_type=UNKNOWN;
        }
    }
    
    /*其他的普通字符*/
    return g_symbol_type=INPUT_ELE;
    
    return g_symbol_type=UNKNOWN;
}

/*得到c的ASCII码*/
int getASCII(char c) {
    int ascii;
    ascii = (int)c;
    if (ascii <0) {
        ascii += 256;
    }
    return ascii;
}
/*
 老公兜儿， 那回你给我收拾脚鸭的事情，我经常会想起来，一想起来就觉得老公 对我真好。。。。 哈哈
 岱儿爱老公兜儿。。。
 */
