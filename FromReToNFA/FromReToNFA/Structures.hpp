//
//  Structures.hpp
//  FromReToNFA
//
//  Created by 李嘉诚 on 2017/10/8.
//  Copyright © 2017年 李嘉诚. All rights reserved.
//

#ifndef Structures_hpp
#define Structures_hpp

#include <stdio.h>
#include <iostream>
class State
{
public:
    bool accept;
    char key;
    State *child;
    State *next;
};




#endif /* Structures_hpp */
