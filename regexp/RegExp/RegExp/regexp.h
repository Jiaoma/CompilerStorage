/*global data type*/
/*##############################################*/
/*终结符类型*/
typedef enum SymbolType_enum {
    UNKNOWN = 0,
    END_REGEXP,
    START_REGEXP,
    INPUT_ELE,
    REPEAT_ZERO_MORE, REPEAT_ZERO_ONCE, REPEAT_ONCE_MORE,
    REPEAT_RANGE_MN, REPEAT_RANGE_M, REPEAT_RANGE_M_MORE,
    AND_MACHINE_BEGIN, AND_MACHINE_END,
    OR_MACHINE_BEGIN, OR_MACHINE_END,
    NOT_OP, DOT, BACKTRACE,
    NUMBER, NOT_NUMBER,
    ALL_SPACE, NOT_ALL_SPACE,
    AZaz09_, NOT_AZaz09_,
    LETTER_RANGE
}SymbolType;
/*##############################################*/
/*状态*/
typedef struct Statetag {
    struct StateCollectiontag *destStateCollection;    /*目标a状态集合*/
    struct Statetag *next;
}State, *pState;
/*相关操作函数声明*/
pState newState();
void destroyState(pState s);
pState setdestState(pState srcS, int eleindex, pState destS);
pState getdestState(pState srcS, int eleindex);
pState cloneState(pState s);

/*##############################################*/
/*状态集合的元素*/
typedef struct StateCollectionEletag {
    int eleindex;  /*元素*/
    pState destState;  /*目标状态*/
    struct StateCollectionEletag *next;
}StateCollectionEle, *pStateCollectionEle;
/*相关操作函数声明*/
pStateCollectionEle newStateCollectionEle(int eleindex, pState destS);
void destroyStateCollectionEle(pStateCollectionEle pSCEle);
pStateCollectionEle cloneStateCollectionEle(pStateCollectionEle pSCEle);

/*##############################################*/
/*状态集合*/
typedef struct StateCollectiontag {
    int destStateCount;   /*目标状态数组的大小*/
    pStateCollectionEle head;  /*目标状态链表指针*/
}StateCollection, *pStateCollection;
/*相关操作函数声明*/
pStateCollection newStateCollection();    /*新建状态集合*/
void destroyStateCollection(pStateCollection pSC);
pStateCollectionEle appendStateCollection(pStateCollection pSC, pStateCollectionEle pSCEle);
pStateCollection cloneStateCollection(pStateCollection pSC);

/*##############################################*/
/*State table*/
typedef struct StateTabletag {
    int eleCount;
    int stateCount;
    pState head, curState, tail;
}StateTable, *pStateTable;
/*相关操作函数声明*/
pStateTable newStateTable(int eleCount);
void destroyStateTable(pStateTable st);
pState appendStateTable(pStateTable st, pState s);
void showStateTable(pStateTable);
pStateTable cloneSubStateTable(pStateTable pst, pState ps_start, pState ps_end);

/*##############################################*/
/*状态机*/
typedef struct Machinetag {
    pState state_start, state_end;
    int pos_start, pos_end;
    char join_mode;   /*'.'或'|'*/
    char not_mode;   /*'^'或其他的*/
    struct Machinetag *next;
}Machine, *pMachine;
/*相关操作函数声明*/
pMachine newMachine(pState start, pState end,
                    int pos_start, int pos_end,
                    char join_mode, char not_mode);
void destroyMachine(pMachine mac);

/*##############################################*/
/*状态机堆栈*/
typedef struct MachineStacktag {
    pMachine topM;    /*插入点指针*/
    int MachineCount;
}MachineStack, *pMachineStack;
/*相关操作函数声明*/
pMachineStack newMachineStack();
void destroyMachineStack(pMachineStack pms);
pMachine pushMachine(pMachineStack pms, pMachine pMEle);
pMachine popMachine(pMachineStack pms);

/*##############################################*/
/*#######    *函数实现*   ######################*/
/*##############################################*/

/*状态*/
pState newState() {
    pState pS = NULL;
    pS = (pState)malloc(sizeof(State));
    if (pS != NULL) {
        pS->destStateCollection = newStateCollection();
        pS->next = NULL;
    }
    return pS;
}

void destroyState(pState s) {
    if (s != NULL) {
        destroyStateCollection(s->destStateCollection);
        free(s);
    }
}

pState setdestState(pState srcS, int eleindex, pState destS) {
    struct StateCollectionEletag *pSCEle;
    pSCEle = newStateCollectionEle(eleindex, destS);
    if (pSCEle != NULL) {
        appendStateCollection(srcS->destStateCollection, pSCEle);
    }
    return destS;
}

pState getdestState(pState srcS, int eleindex) {
    return NULL;
}

pState cloneState(pState s) {
    pState ps = NULL;
    ps = newState();
    if (ps != NULL) {
        ps->destStateCollection = cloneStateCollection(s->destStateCollection);
        ps->next = s->next;
    }
    return ps;
}

/*##############################################*/
/*状态集合元素*/
pStateCollectionEle newStateCollectionEle(int eleindex, pState destS) {
    pStateCollectionEle pSCEle = NULL;
    pSCEle = (pStateCollectionEle)malloc(sizeof(StateCollectionEle));
    if (pSCEle != NULL) {
        pSCEle->eleindex = eleindex;
        pSCEle->destState = destS;
        pSCEle->next = NULL;
    }
    return pSCEle;
}

void destroyStateCollectionEle(pStateCollectionEle pSCEle) {
    free(pSCEle);
}

pStateCollectionEle cloneStateCollectionEle(pStateCollectionEle pSCEle) {
    pStateCollectionEle psenew = NULL;
    psenew = newStateCollectionEle(pSCEle->eleindex, pSCEle->destState);
    if (psenew != NULL) {
        psenew->next = pSCEle->next;
    }
    return psenew;
}

/*##############################################*/
/*状态集合*/
pStateCollection newStateCollection() {
    pStateCollection pSC = NULL;
    pSC = (pStateCollection)malloc(sizeof(StateCollection));
    if (pSC != NULL) {
        pSC->destStateCount = 0;
        pSC->head = NULL;
    }
    return pSC;
}

void destroyStateCollection(pStateCollection pSC) {
    pStateCollectionEle pSCEle = NULL;
    if (pSC != NULL) {
        while (pSC->head != NULL) {
            pSCEle = pSC->head;
            pSC->head = pSC->head->next;
            pSC->destStateCount--;
            free(pSCEle);
        }
        free(pSC);
    }
}

pStateCollectionEle appendStateCollection(pStateCollection pSC, pStateCollectionEle pSCEle) {
    if (pSCEle != NULL) {
        pSCEle->next = pSC->head;
        pSC->head = pSCEle;
        pSC->destStateCount++;
    }
    return pSCEle;
}

pStateCollection cloneStateCollection(pStateCollection pSC) {
    pStateCollection psc = NULL;
    pStateCollectionEle pse, psenew;
    if (pSC != NULL) {
        psc = newStateCollection();
        if (psc != NULL) {
            pse = pSC->head;
            while (pse != NULL) {
                psenew = cloneStateCollectionEle(pse);
                psenew->next = NULL;
                appendStateCollection(psc, psenew);
                pse = pse->next;
            }
        }
    }
    return psc;
}

/*##############################################*/
/*状态表格*/
pStateTable newStateTable(int eleCount) {
    pStateTable pst = NULL;
    pst = (pStateTable)malloc(sizeof(StateTable));
    if (pst != NULL) {
        pst->eleCount = eleCount;
        pst->stateCount = 0;
        pst->head = NULL;
        pst->curState = NULL;
        pst->tail = NULL;
    }
    return pst;
}

void destroyStateTable(pStateTable st) {
    pState phead = NULL;
    while (st->head != NULL) {
        phead = st->head;
        st->head = st->head->next;
        destroyState(phead);
    }
    free(st);
}

pState appendStateTable(pStateTable st, pState s) {
    if (s != NULL && st != NULL) {
        if (st->tail == NULL) {
            st->tail = s;
        } else {
            st->tail->next = s;
            st->tail = s;
        }
        st->stateCount++;
        
        if (st->head == NULL) {
            st->head = st->tail;   /*只有一个元素*/
        }
    }
    return s;
}

/*输出StateTable,测试用*/
void showStateTable(pStateTable pst) {
    pState tmp;
    pStateCollectionEle pscele;
    
    printf("debug: ################### there is the NFA table basic info: #######################\n");
    printf("debug: StateCount = %d\n", pst->stateCount);
    printf("head->%x\tcur->%x\ttail->%x\n", pst->head, pst->curState, pst->tail);
    printf("debug: ################### there is the NFA table: #######################\n");
    tmp = pst->head;
    while (tmp != NULL) {
        printf("#%x->", tmp);
        pscele = tmp->destStateCollection->head;
        while (pscele != NULL) {
            printf("[%d]%x | ", pscele->eleindex, pscele->destState);
            pscele = pscele->next;
        }
        printf("\n");
        tmp = tmp->next;
    }
    printf("debug: ################### the NFA table ended #######################\n");
    printf("debug: StateCount = %d\n", pst->stateCount);
    printf("head->%x\tcur->%x\ttail->%x\n", pst->head, pst->curState, pst->tail);
    printf("debug: ################### the NFA table basic info ended #######################\n");
}

pStateTable cloneSubStateTable(pStateTable pst, pState ps_start, pState ps_end) {
    pStateTable pstnew = NULL;
    pState ps, psnew;   /*临时指针:原状态和目标状态*/
    pStateCollectionEle pse, psenew;    /*临时的状态集合元素指针,替换目标状态使用*/
    if (pst != NULL) {
        pstnew = newStateTable(pst->eleCount);  /*新建一个状态表*/
        if (pstnew != NULL) {
            ps = ps_start;
            while (ps != NULL) {
                psnew = cloneState(ps); /*克隆当前状态*/
                psnew->next = NULL;
                appendStateTable(pstnew, psnew);    /*加入到新的状态表*/
                /*将新的克隆状态加入原状态的目标状态集合中,一会儿要做目标状态的替换*/
                appendStateCollection(ps->destStateCollection, newStateCollectionEle(NULL, psnew));
                if (ps == ps_end) {
                    break;  /*到了结束的状态,返回结果*/
                }
                ps = ps->next;  /*移动到原状态表的下一个状态*/
            }
            /*start -- end这一段状态链表克隆完毕
             下面将做目标状态的替换*/
            psnew = pstnew->head;
            /*遍历新的状态表*/
            while (psnew != NULL) {
                pse = psnew->destStateCollection->head;
                /*遍历当前状态目标集合中的元素*/
                while (pse != NULL) {
                    /*得到原状态的克隆体的指针,存放于原状态目标集合的头部,见上面的while操作*/
                    psenew = pse->destState->destStateCollection->head;
                    /*用原目标状态的克隆体来替换*/
                    pse->destState = psenew->destState;
                    
                    pse = pse->next; /*下一个状态*/
                }
                psnew = psnew->next;
            }
            /*新状态表中的目标状态集合已经替换完毕,
             下面删除这个克隆体在原状态目标集合中的临时元素*/
            ps = ps_start;
            while (ps != NULL) {
                psenew = ps->destStateCollection->head; /*克隆体状态集合元素指针*/
                ps->destStateCollection->head = psenew->next;  /*移出集合*/
                ps->destStateCollection->destStateCount--;
                destroyStateCollectionEle(psenew);  /*释放空间*/
                if (ps == ps_end) {
                    break;  /*到了结束的状态,返回结果*/
                }
                ps = ps->next;  /*移动到原状态表的下一个状态*/
            }
        }
    }
    /*
     printf("debug: ######## start: %x | end: %x\n", ps_start, ps_end);  /*###  debug:  ####*/
    return pstnew;
}

pStateTable joinStateTable(pStateTable pst, pStateTable psttmp) {
    if (pst == NULL) {
        pst = newStateTable(psttmp->eleCount);
        pst->head = psttmp->head;
        pst->tail = psttmp->tail;
        pst->curState = pst->tail;
        pst->stateCount = psttmp->stateCount;
        pst->eleCount = psttmp->eleCount;
    } else {
        if (psttmp != NULL) {
            /*将临时状态表头接入pst尾的状态表格*/
            setdestState(pst->tail, pst->eleCount, psttmp->head);
            /*合并两个表*/
            psttmp->tail->next = pst->tail->next;
            pst->tail->next = psttmp->head;
            pst->tail = psttmp->tail;
            pst->curState = pst->tail;
            /*调整合并后的状态数目*/
            pst->stateCount += psttmp->stateCount;
        }
    }
    
    return pst;
}

/*##############################################*/
/*状态机*/
pMachine newMachine(pState start, pState end,
                    int pos_start, int pos_end,
                    char join_mode, char not_mode) {
    pMachine pm = NULL;
    pm = (pMachine)malloc(sizeof(Machine));
    if (pm != NULL) {
        pm->state_start = start;
        pm->state_end = end;
        pm->pos_start = pos_start;
        pm->pos_end = pos_end;
        pm->join_mode = join_mode;
        pm->not_mode = not_mode;
    }
    return pm;
}

void destroyMachine(pMachine mac) {
    free(mac);
}

/*##############################################*/
/*状态机堆栈*/
pMachineStack newMachineStack() {
    pMachineStack pms = NULL;
    pms = (pMachineStack)malloc(sizeof(MachineStack));
    if (pms != NULL) {
        pms->topM = NULL;
        pms->MachineCount = 0;
    }
    return pms;
}

void destroyMachineStack(pMachineStack pms) {
    pMachine pm = NULL;
    while (pms->topM != NULL) {
        pm = popMachine(pms);
        destroyMachine(pm);
    }
    free(pms);
}

pMachine pushMachine(pMachineStack pms, pMachine pMEle) {
    pMEle->next = pms->topM;
    pms->topM = pMEle;
    pms->MachineCount++;
    return pMEle;
}

pMachine popMachine(pMachineStack pms) {
    pMachine pMEle = NULL;
    if (pms->MachineCount > 0) {
        pMEle = pms->topM;
        pms->topM = pms->topM->next;
        pms->MachineCount--;
    }
    return pMEle;
}


