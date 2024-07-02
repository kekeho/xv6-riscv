#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "im.h"

void plain_ime(Request req, State* state);


// Input Method

void (*current_im)(Request, State*);

void initIM() {
    // Input Methodの初期化
    // IMEのスポーン
    current_im = plain_ime;
}


void handleInput(State* state, char input) {
    // 入力のハンドリング
    // IMEにルーティングをする
    Request req;
    req.type = input;
    req.body.input = input;
    (*current_im)(req, state);
}


// IME

void plain_ime(Request req, State* state) {
    state->type = done;
    state->state.doneState.result[0] = req.body.input;
    state->state.doneState.result[1] = '\0';
}
