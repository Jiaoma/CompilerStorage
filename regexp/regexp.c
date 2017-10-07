#include <stdio.h>
#include <stdlib.h>
#include "regexp.h"

/********************************/
/*global variables*/
char *g_strRegExp;
int g_scan_pos = -1;   /*ɨ��ָ��*/
SymbolType g_symbol_type;   /*����Ԫ������*/
int g_symbol_charvalue;  /*����Ԫ�ص��ַ�ֵ*/
int g_lastchar;    /*��������ݵ�ǰ�ַ�,�ظ�����ʱ�͵ñ���*/
int g_repeat_m, g_repeat_n;   /*�ظ�������Ͻ���½�*/
pMachineStack g_mstk;   /*״̬�����ö�ջ*/
pStateTable g_st;   /*NFA���*/
/*��Ҫ�õ�����ʱ����*/
pState psnew;	/*�µ�״ָ̬��*/
pMachine pmnew;	/*�µ�״̬��ָ��*/
pStateTable pst_needed, pst_optional;   /*�ظ�����ʱ���ص�ָ��*/
char g_ele_ary[256];    /*��������ĸ����Ч��1,������0*/
int while_notfinish; /*���ʽɨ��ѭ������*/

/*****************************��ͷ***/
/*void showStateTable(pStateTable pst);*/
SymbolType getSymbol();   /*�ʷ���������,����һ�����ŵ����ͺ�����ֵ*/
int dealState(SymbolType symboltype);  /*��������״̬*/
void repeat_needed(int m, int eleindex);  /*{m,n}���ɱ�ѡ��m��ѭ��״̬��*/
void repeat_optional(int n, int eleindex);    /*{m,n}���ɿ�ѡ��n��ѭ��״̬��*/
pStateTable repeatMachine_needed(int m, pMachine pm);  /*A{m,n}���ɱ�ѡ��m��ѭ��״̬��*/
pStateTable repeatMachine_optional(int n, pMachine pm);    /*A{m,n}���ɿ�ѡ��n��ѭ��״̬��*/
int getASCII(char c); /*�õ�c��ASCII��*/
/********************************/
int main(int argc, char* argv[]) {
    /*����,������в���
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

/*���Ľ�������: �﷨��������*/
int regexp_main() {
    /*��ʼ�� g_st & g_mstk*/
    g_st = newStateTable(256);  /*0-255, 256��epsilon(��)*/
    appendStateTable(g_st, newState()); /*��ʼ״̬*/
    g_st->curState = g_st->head;
    g_mstk = newMachineStack();  /*��ջ��*/

    g_scan_pos = -1;
    g_symbol_type = START_REGEXP;
    while_notfinish = 1; /*���ʽɨ��ѭ������,��END_REGEXP�л���0*/
    while (while_notfinish) {
        if (dealState(g_symbol_type) == 1) {
            /*��һ������,��ǰ���switch()��,
  		    ��ЩԤ�ȶ�ȡһ���ʵķ�֦����continue������������*/
            getSymbol();
        }
    }

    /*���StateTable,������*/
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
/*״̬��������Ӧ����,�����˸���״̬�ľ�����Ϊ*/
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

                /*ģ��������ĸ���е�ÿ��Ԫ��*/
				pmnew = newMachine(NULL, NULL, g_scan_pos, -1, '|', NULL);
				pmnew->state_start = newState();	/*״̬����ʼ״̬*/
				pmnew->state_end = newState();	/*״̬����ʼ״̬*/
				/*����ʼ״̬�뵱ǰ״̬����*/
				appendStateTable(g_st, pmnew->state_start);	/*��ӵ�״̬���*/
                setdestState(g_st->curState, g_st->eleCount, pmnew->state_start);
				g_st->curState = pmnew->state_start; /*�ƶ����µ�״̬*/
				
				/*������һ��״̬��,�����Ķϵ�,����ģʽѹ���ջ*/
				pushMachine(g_mstk, pmnew);

                /*����Щ��֧���뵽�����״̬��*/
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
                /*����ǰ״̬����״̬���Ľ���״̬����*/
                setdestState(g_st->curState, (int)g_st->eleCount, g_mstk->topM->state_end);
                /*����ǰ״̬���˵���״̬����ͷ��,��ʼһ���µķ�֧*/
                g_st->curState = g_mstk->topM->state_start;
                break;
            case INPUT_ELE:
                /*��һ���ս��,����һ����*/
                g_lastchar = g_symbol_charvalue;
                psnew = newState();
                appendStateTable(g_st, psnew);  /*��ӵ�״̬��*/
                setdestState(g_st->curState, (int)g_symbol_charvalue, psnew);

                /*�ظ������*/
                getSymbol();
                switch (g_symbol_type) {
                    case REPEAT_ZERO_MORE:
                        /*(a*)*/
                        setdestState(g_st->curState, g_st->eleCount, psnew);
                        setdestState(psnew, g_st->eleCount, g_st->curState);
                        /*�ƶ����µ�״̬*/
                        g_st->curState = psnew;
                        printf("debug: ZEOR_MORE\n");
                        break;
                    case REPEAT_ZERO_ONCE:
                        /*(a?)*/
                        setdestState(g_st->curState, g_st->eleCount, psnew);
                        /*�ƶ����µ�״̬*/
                        g_st->curState = psnew;
                        printf("debug: ZEOR_ONCE\n");
                        break;
                    case REPEAT_ONCE_MORE:
                        /*(a+)*/
                        setdestState(psnew, g_st->eleCount, g_st->curState);
                        /*�ƶ����µ�״̬*/
                        g_st->curState = psnew;
                        printf("debug: ONCE_MORE\n");
                        break;
                    case REPEAT_RANGE_MN:
                        /*(a{m,n})*/
                        if (g_repeat_m <= 0) {
                            /*����{0,n}�����,�Ѹոյ�a���(a?)*/
                            setdestState(g_st->curState, g_st->eleCount, psnew);
                            /*�ƶ����µ�״̬*/
                            g_st->curState = psnew;
                            /*�ظ�n-1��(a?)*/
                            repeat_optional(g_repeat_n-1, g_lastchar);
                        } else {
                            /*�ƶ����µ�״̬*/
                            g_st->curState = psnew;
                            repeat_needed(g_repeat_m-1 , g_lastchar);
                            repeat_optional(g_repeat_n-g_repeat_m, g_lastchar);
                        }
                        printf("debug: type -> %d\t|  m -> %d  |  n -> %d\n", symboltype, g_repeat_m, g_repeat_n);
                        break;
                    case REPEAT_RANGE_M_MORE:
                        /*(a{m,})*/
                        /*�ƶ����µ�״̬*/
                        g_st->curState = psnew;
                        repeat_needed(g_repeat_m-1, g_lastchar);
                        /*��һ����a*��*/
                        psnew = newState();
                        appendStateTable(g_st, psnew);
                        setdestState(g_st->curState, g_lastchar, psnew);
                        setdestState(g_st->curState, g_st->eleCount, psnew);
                        setdestState(psnew, g_st->eleCount, g_st->curState);
                        /*�ƶ��µ�״̬*/
                        g_st->curState = psnew;
                        printf("debug: type -> %d\t|  m -> %d  |  n -> max_int\n", symboltype, g_repeat_m);
                        break;
                    case REPEAT_RANGE_M:
                        /*(a{m})*/
                        /*�ƶ����µ�״̬*/
                        g_st->curState = psnew;
                        repeat_needed(g_repeat_m-1, g_lastchar);
                        printf("debug: type -> %d\t|  m -> %d\n", symboltype, g_repeat_m);
                        break;
                    default:
                        /*�����ظ������,�ƶ����µ�״̬,Ȼ������ѭ��*/
                        g_st->curState = psnew;
                        return 0;
                }/*�ظ������ -- ����*/

                break;/*INPUT_ELE -- ����*/
            case START_REGEXP:
                printf("debug: start scan the regexp:\n\t\"%s\"\n", g_strRegExp);
                /*��һ��״̬����ʼ����*/
            case AND_MACHINE_BEGIN:
				/*��ʼһ����״̬��,������: '('*/
			    pmnew = newMachine(NULL, NULL, g_scan_pos, -1, '.', NULL);
				pmnew->state_start = newState();	/*״̬����ʼ״̬*/
				pmnew->state_end = newState();	/*״̬����ʼ״̬*/
				/*����ʼ״̬�뵱ǰ״̬����*/
				appendStateTable(g_st, pmnew->state_start);	/*��ӵ�״̬���*/
                setdestState(g_st->curState, g_st->eleCount, pmnew->state_start);
				g_st->curState = pmnew->state_start; /*�ƶ����µ�״̬*/
				
				/*������һ��״̬��,�����Ķϵ�,����ģʽѹ���ջ*/
				pushMachine(g_mstk, pmnew);
				break;	/*AND_MACHINE_BEGIN -- ����*/
			case OR_MACHINE_BEGIN:
				/*��ʼһ����������״̬��(��ĸ��),������: '['*/
				pmnew = newMachine(NULL, NULL, g_scan_pos, -1, '|', NULL);
				pmnew->state_start = newState();	/*״̬����ʼ״̬*/
				pmnew->state_end = newState();	/*״̬����ʼ״̬*/
				/*����ʼ״̬�뵱ǰ״̬����*/
				appendStateTable(g_st, pmnew->state_start);	/*��ӵ�״̬���*/
                setdestState(g_st->curState, g_st->eleCount, pmnew->state_start);
				g_st->curState = pmnew->state_start; /*�ƶ����µ�״̬*/
				
				/*������һ��״̬��,�����Ķϵ�,����ģʽѹ���ջ*/
				pushMachine(g_mstk, pmnew);

                getSymbol();
                /*������״̬���ķ�ģʽ*/
                if (g_symbol_type == NOT_OP) {
                    g_mstk->topM->not_mode = '^';
                    getSymbol();    /*������һ������*/
                }
                /*��ʼ��*/
                for (OR_MACHINE_i=0; OR_MACHINE_i<sizeof(g_ele_ary); OR_MACHINE_i++) {
                    g_ele_ary[OR_MACHINE_i] = 0;
                }
                while (g_symbol_type != OR_MACHINE_END && g_symbol_type != END_REGEXP) {
                    switch (g_symbol_type) {
                        case INPUT_ELE:
                            g_ele_ary[(int)g_symbol_charvalue] = 1;
                            /*���洦��һ��[a-z]������ʽ*/
                            g_lastchar = g_symbol_charvalue;    /*���ݿ�ʼԪ��*/
                            /*�����һ��Ԫ���ǲ���LETTER_RANGE*/
                            getSymbol();
                            if (g_symbol_type == LETTER_RANGE) {
                                /*��һ�����ӷ�,�����һ���ַ��ǲ���INPUT_ELE*/
                                getSymbol();
                                if (g_symbol_type == INPUT_ELE) {
                                    /*�Ѿ��ҵ�һ���������ӷ��﷨,��������ĸ��Χ*/
                                    OR_MACHINE_i = (g_lastchar<g_symbol_charvalue) ? (int)g_lastchar : (int)g_symbol_charvalue;
                                    if (g_lastchar<=g_symbol_charvalue) {
                                        g_lastchar = g_symbol_charvalue;
                                    }
                                    for (; OR_MACHINE_i<=(int)g_lastchar; OR_MACHINE_i++) {
                                        g_ele_ary[OR_MACHINE_i] = 1;
                                    }
                                } else {
                                    /*���ӷ������Ҳ���INPUT_ELE*/
                                    /*���������������ӷ��﷨,����ѭ��*/
                                    continue;
                                }
                            } else {
                                /*����һ�����ӷ�*/
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
                    }

                    getSymbol();
                }
                if (g_symbol_type == END_REGEXP) {
                    dealState(OR_MACHINE_END);
                    g_symbol_type = END_REGEXP;
                }
                return 0;   /*֪ͨ�����߲�����getSymbol(),���ﻹ��һ��δ����*/
				break;	/*AND_MACHINE_BEGIN -- ����*/
            case END_REGEXP:
                while_notfinish = 0; /*һ����˳�ѭ��*/
                printf("debug: end scan the regexp:\n\t\"%s\"\n", g_strRegExp);
                /*��һ��״̬����������*/
			case OR_MACHINE_END:
                if (symboltype == OR_MACHINE_END) {
                    /*��g_ele_ary[]���ӵ�OR_MACHINE��*/
                    for (OR_MACHINE_i=0; OR_MACHINE_i<sizeof(g_ele_ary); OR_MACHINE_i++) {
                        if (g_mstk->topM->not_mode == '^') {
                            /*��ģʽ*/
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
				/*�ر�һ����״̬��,������: ')'*/
				/*�õ���ǰ״̬������*/
				pmnew = g_mstk->topM;
				/*��״̬���Ľ���״̬����״̬��*/
				appendStateTable(g_st, pmnew->state_end);
				/*������ǰ״̬��: ���һ��״̬����״̬���Ľ���״̬*/
				setdestState(g_st->curState, g_st->eleCount, pmnew->state_end);
				g_st->curState = pmnew->state_end;  /*�ƶ����µ�״̬*/
				if (symboltype != END_REGEXP) {
    				/*������ʽû�н���,���һ����,�ж��ǲ����ظ�����,
    				�������ִ����Ӧ���ظ�����,��������������ʽ*/
    				getSymbol();
                }
                switch (g_symbol_type) {
                    case REPEAT_ZERO_MORE:
                        /*(A*)*/
                        /*��״̬����ʼ�ͽ�����״̬��epsilon����*/
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
                            /*����{0,n}�����,�Ѹոյ�A���(A?)*/
                            setdestState(pmnew->state_start, g_st->eleCount, pmnew->state_end);
                            /*�ظ�n-1��(A?)*/
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
                        /*��һ��(A*)*/
                        psnew = g_st->curState; /*���浱ǰ״̬*/
                        pst_optional = repeatMachine_optional(1, pmnew);   /*��һ��(A?)*/
                        if (pst_optional != NULL) {
                            g_st = joinStateTable(g_st, pst_optional);
                            pst_optional->head = NULL;
                            pst_optional->curState = NULL;
                            pst_optional->tail = NULL;
                            destroyStateTable(pst_optional);
                        }
                        psnew = psnew->next;    /*���һ���ظ�����Ŀ�ʼ״̬*/
                        setdestState(psnew, g_st->eleCount, g_st->curState);    /*�޸ĳ�(A*)*/
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
                        /*�����ظ������,�Ӷ�ջ�е�����ǰ״̬��,Ȼ������ѭ��*/
                        pmnew = popMachine(g_mstk);
                        destroyMachine(pmnew);
                        return 0;
                }/*�ظ������ -- ����*/

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

        return 1;    /*֪ͨ�����ߴ�����һ������,��:����get_symbol()*/
}
/********************************/
/*{m,n}���ɱ�ѡ��m��ѭ��״̬��*/
void repeat_needed(int m, int eleindex) {
    pState psnew;
    int i;
    for (i=0; i<m; i++) {
        psnew = newState();
        appendStateTable(g_st, psnew);
        /*��������*/
        setdestState(g_st->curState, eleindex, psnew);
        g_st->curState = psnew;
    }
}
/********************************/
/*{m,n}���ɿ�ѡ��n��ѭ��״̬��*/
void repeat_optional(int n, int eleindex) {
    pState psnew;
    int i;
    for (i=0; i<n; i++) {
        psnew = newState();
        appendStateTable(g_st, psnew);
        /*��1��ZERO_ONCE����*/
        setdestState(g_st->curState, eleindex, psnew);
        setdestState(g_st->curState, g_st->eleCount, psnew);
        g_st->curState = psnew;
    }
}
/********************************/
/*{m,n}���ɱ�ѡ��m��ѭ��״̬��*/
pStateTable repeatMachine_needed(int m, pMachine pm) {
    pStateTable pst = NULL, psttmp = NULL;
    int i;
    /*���ظ�m�����ɵı����һ�����һ���ӱ�*/
    for (i=0; i<m; i++) {
        psttmp = cloneSubStateTable(g_st, pm->state_start, pm->state_end);
        if (psttmp != NULL) {
            psttmp->tail->next = NULL;

            pst = joinStateTable(pst, psttmp);
            /*�ͷ������ʱ��״̬��Ŀռ�*/
            psttmp->head = NULL;
            /*��ͷָ��Ͽ�,����destroy�������ͷ�����ָ���״̬����Ŀռ�*/
            psttmp->tail = NULL;
            psttmp->curState = NULL;
            destroyStateTable(psttmp);
        }
    }
    return pst;
    /*���������m�εĴ��ӱ���g_st�ϲ�*/
    g_st = joinStateTable(g_st, pst);
    /*�ͷ������ʱ��״̬��Ŀռ�*/
    pst->head = NULL;
    /*��ͷָ��Ͽ�,����destroy�������ͷ�����ָ���״̬����Ŀռ�*/
    pst->tail = NULL;
    pst->curState = NULL;
    destroyStateTable(pst);
}
/********************************/
/*{m,n}���ɿ�ѡ��n��ѭ��״̬��*/
pStateTable repeatMachine_optional(int n, pMachine pm) {
    pStateTable pst = NULL, psttmp = NULL;
    int i;
    /*���ظ�n�����ɵı����һ�����һ���ӱ�*/
    for (i=0; i<n; i++) {
        psttmp = cloneSubStateTable(g_st, pm->state_start, pm->state_end);
        if (psttmp != NULL) {
            psttmp->tail->next = NULL;
            /*����ʱ����һ��(A?)����*/
            setdestState(psttmp->head, g_st->eleCount, psttmp->tail);

            pst = joinStateTable(pst, psttmp);
            /*�ͷ������ʱ��״̬��Ŀռ�*/
            psttmp->head = NULL;
            /*��ͷָ��Ͽ�,����destroy�������ͷ�����ָ���״̬����Ŀռ�*/
            psttmp->tail = NULL;
            psttmp->curState = NULL;
            destroyStateTable(psttmp);
        }
    }
    return pst;
    /*���������m�εĴ��ӱ���g_st�ϲ�*/
    g_st = joinStateTable(g_st, pst);
    /*�ͷ������ʱ��״̬��Ŀռ�*/
    /*��ͷָ��Ͽ�,����destroy�������ͷ�����ָ���״̬����Ŀռ�*/
    pst->head = NULL;
    pst->tail = NULL;
    pst->curState = NULL;
    destroyStateTable(pst);
}
/********************************/
/*�ʷ�����,ȡ��һ������*/
SymbolType getSymbol() {
    int *cc = &g_symbol_charvalue;
    char tmp; /*��ʱ����*/
    
    *cc = getASCII(g_strRegExp[++g_scan_pos]);
    /*������ʽ��β*/
    if (*cc == NULL) {return g_symbol_type=END_REGEXP;}
    if (*cc == '(') {return g_symbol_type=AND_MACHINE_BEGIN;}
    if (*cc == ')') {return g_symbol_type=AND_MACHINE_END;}
    if (*cc == '[') {return g_symbol_type=OR_MACHINE_BEGIN;}
    if (*cc == ']') {return g_symbol_type=OR_MACHINE_END;}
    if (*cc == '|') {return g_symbol_type=BACKTRACE;}
    if (*cc == '^') {return g_symbol_type=NOT_OP;}
    if (*cc == '.') {return g_symbol_type=DOT;}
    if (*cc == '-') {return g_symbol_type=LETTER_RANGE;}
    /*ת���ַ�*/
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
        /*16����*/
        if (*cc == 'x') {
            tmp = getASCII(g_strRegExp[++g_scan_pos]);
            if (tmp>='0' && tmp<='9') {
                *cc = tmp - '0';
            } else if (tmp>='a' && tmp<='f') {
                *cc = tmp - 'a' + 10;
            } else if (tmp>='A' && tmp<='F') {
                *cc = tmp - 'A' + 10;
            } else {
                /*����16��������,����ԭ�ַ�'x'*/
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
                /*�������16��������,ֻ��һλ:\xF*/
                g_scan_pos--;
                return g_symbol_type=INPUT_ELE;
            }

            return g_symbol_type=INPUT_ELE;
        }
        /*8����*/
        /*��0-3��ͷ��8������������3λ*/
        if (*cc>='0' && *cc<='3') {
            *cc -= '0';
            tmp = getASCII(g_strRegExp[++g_scan_pos]);
            if (tmp>='0' && tmp<='7') {
                *cc *= 8;
                *cc += tmp - '0';
            } else {
                /*ֻ��һλ8��������:\7*/
                g_scan_pos--;
                return g_symbol_type=INPUT_ELE;
            }
            tmp = getASCII(g_strRegExp[++g_scan_pos]);
            if (tmp>='0' && tmp<='7') {
                *cc *= 8;
                *cc += tmp - '0';
            } else {
                /*ֻ����λ8��������:\77*/
                g_scan_pos--;
                return g_symbol_type=INPUT_ELE;
            }
            return g_symbol_type=INPUT_ELE;
        }
        /*��4-7��ͷ��8���ƿ�����2λ*/
        if (*cc>='4' && *cc<='7') {
            *cc -= '0';
            tmp = getASCII(g_strRegExp[++g_scan_pos]);
            if (tmp>='0' && tmp<='7') {
                *cc *= 8;
                *cc += tmp - '0';
            } else {
                /*ֻ��һλ8��������:\7*/
                g_scan_pos--;
                return g_symbol_type=INPUT_ELE;
            }
            return g_symbol_type=INPUT_ELE;
        } else {
            /*[^x0-7dDfnrtvsSwW]*/
            return g_symbol_type=INPUT_ELE;
        }
    }
    /*�ظ�����*/
    if (*cc == '*') {return g_symbol_type=REPEAT_ZERO_MORE;}
    if (*cc == '+') {return g_symbol_type=REPEAT_ONCE_MORE;}
    if (*cc == '?') {return g_symbol_type=REPEAT_ZERO_ONCE;}
    if (*cc == '{') {
        *cc = getASCII(g_strRegExp[++g_scan_pos]);
        if (*cc>'9' || *cc<'0') {return g_symbol_type=UNKNOWN;}
        g_repeat_m = 0;   /*ȡ{m,n}��m*/
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
        g_repeat_n = 0;   /*ȡ{m,n}��n*/
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
    
    /*��������ͨ�ַ�*/
    return g_symbol_type=INPUT_ELE;

    return g_symbol_type=UNKNOWN;
}

/*�õ�c��ASCII��*/
int getASCII(char c) {
    int ascii;
    ascii = (int)c;
    if (ascii <0) {
        ascii += 256;
    }
    return ascii;
}
/*
�Ϲ������� �ǻ��������ʰ��Ѽ�����飬�Ҿ�������������һ�������;����Ϲ� ������á������� ����
᷶����Ϲ�����������
*/
