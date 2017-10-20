/*global data type*/
/*##############################################*/
/*�ս������*/
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
/*״̬*/
typedef struct Statetag {
    struct StateCollectiontag *destStateCollection;    /*Ŀ��a״̬����*/
    struct Statetag *next;
}State, *pState;
/*��ز�����������*/
pState newState();
void destroyState(pState s);
pState setdestState(pState srcS, int eleindex, pState destS);
pState getdestState(pState srcS, int eleindex);
pState cloneState(pState s);

/*##############################################*/
/*״̬���ϵ�Ԫ��*/
typedef struct StateCollectionEletag {
    int eleindex;  /*Ԫ��*/
    pState destState;  /*Ŀ��״̬*/
    struct StateCollectionEletag *next;
}StateCollectionEle, *pStateCollectionEle;
/*��ز�����������*/
pStateCollectionEle newStateCollectionEle(int eleindex, pState destS);
void destroyStateCollectionEle(pStateCollectionEle pSCEle);
pStateCollectionEle cloneStateCollectionEle(pStateCollectionEle pSCEle);

/*##############################################*/
/*״̬����*/
typedef struct StateCollectiontag {
    int destStateCount;   /*Ŀ��״̬����Ĵ�С*/
    pStateCollectionEle head;  /*Ŀ��״̬����ָ��*/
}StateCollection, *pStateCollection;
/*��ز�����������*/
pStateCollection newStateCollection();    /*�½�״̬����*/
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
/*��ز�����������*/
pStateTable newStateTable(int eleCount);
void destroyStateTable(pStateTable st);
pState appendStateTable(pStateTable st, pState s);
void showStateTable(pStateTable);
pStateTable cloneSubStateTable(pStateTable pst, pState ps_start, pState ps_end);

/*##############################################*/
/*״̬��*/
typedef struct Machinetag {
    pState state_start, state_end;
    int pos_start, pos_end;
    char join_mode;   /*'.'��'|'*/
    char not_mode;   /*'^'��������*/
    struct Machinetag *next;
}Machine, *pMachine;
/*��ز�����������*/
pMachine newMachine(pState start, pState end,
                    int pos_start, int pos_end,
                    char join_mode, char not_mode);
void destroyMachine(pMachine mac);

/*##############################################*/
/*״̬����ջ*/
typedef struct MachineStacktag {
    pMachine topM;    /*�����ָ��*/
    int MachineCount;
}MachineStack, *pMachineStack;
/*��ز�����������*/
pMachineStack newMachineStack();
void destroyMachineStack(pMachineStack pms);
pMachine pushMachine(pMachineStack pms, pMachine pMEle);
pMachine popMachine(pMachineStack pms);

/*##############################################*/
/*#######    *����ʵ��*   ######################*/
/*##############################################*/

/*״̬*/
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
/*״̬����Ԫ��*/
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
/*״̬����*/
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
/*״̬���*/
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
            st->head = st->tail;   /*ֻ��һ��Ԫ��*/
        }
    }
    return s;
}

/*���StateTable,������*/
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
    pState ps, psnew;   /*��ʱָ��:ԭ״̬��Ŀ��״̬*/
    pStateCollectionEle pse, psenew;    /*��ʱ��״̬����Ԫ��ָ��,�滻Ŀ��״̬ʹ��*/
    if (pst != NULL) {
        pstnew = newStateTable(pst->eleCount);  /*�½�һ��״̬��*/
        if (pstnew != NULL) {
            ps = ps_start;
            while (ps != NULL) {
                psnew = cloneState(ps); /*��¡��ǰ״̬*/
                psnew->next = NULL;
                appendStateTable(pstnew, psnew);    /*���뵽�µ�״̬��*/
                /*���µĿ�¡״̬����ԭ״̬��Ŀ��״̬������,һ���Ҫ��Ŀ��״̬���滻*/
                appendStateCollection(ps->destStateCollection, newStateCollectionEle(NULL, psnew));
                if (ps == ps_end) {
                    break;  /*���˽�����״̬,���ؽ��*/
                }
                ps = ps->next;  /*�ƶ���ԭ״̬�����һ��״̬*/
            }
            /*start -- end��һ��״̬�����¡���
             ���潫��Ŀ��״̬���滻*/
            psnew = pstnew->head;
            /*�����µ�״̬��*/
            while (psnew != NULL) {
                pse = psnew->destStateCollection->head;
                /*������ǰ״̬Ŀ�꼯���е�Ԫ��*/
                while (pse != NULL) {
                    /*�õ�ԭ״̬�Ŀ�¡���ָ��,�����ԭ״̬Ŀ�꼯�ϵ�ͷ��,�������while����*/
                    psenew = pse->destState->destStateCollection->head;
                    /*��ԭĿ��״̬�Ŀ�¡�����滻*/
                    pse->destState = psenew->destState;
                    
                    pse = pse->next; /*��һ��״̬*/
                }
                psnew = psnew->next;
            }
            /*��״̬���е�Ŀ��״̬�����Ѿ��滻���,
             ����ɾ�������¡����ԭ״̬Ŀ�꼯���е���ʱԪ��*/
            ps = ps_start;
            while (ps != NULL) {
                psenew = ps->destStateCollection->head; /*��¡��״̬����Ԫ��ָ��*/
                ps->destStateCollection->head = psenew->next;  /*�Ƴ�����*/
                ps->destStateCollection->destStateCount--;
                destroyStateCollectionEle(psenew);  /*�ͷſռ�*/
                if (ps == ps_end) {
                    break;  /*���˽�����״̬,���ؽ��*/
                }
                ps = ps->next;  /*�ƶ���ԭ״̬�����һ��״̬*/
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
            /*����ʱ״̬��ͷ����pstβ��״̬���*/
            setdestState(pst->tail, pst->eleCount, psttmp->head);
            /*�ϲ�������*/
            psttmp->tail->next = pst->tail->next;
            pst->tail->next = psttmp->head;
            pst->tail = psttmp->tail;
            pst->curState = pst->tail;
            /*�����ϲ����״̬��Ŀ*/
            pst->stateCount += psttmp->stateCount;
        }
    }
    
    return pst;
}

/*##############################################*/
/*״̬��*/
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
/*״̬����ջ*/
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


