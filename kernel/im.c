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
#include "emoji_dict.h"

void plain_ime(Request req, State* state);
void emoji_ime(Request req, State* state);
int key_match(const char *keyword, const char *value);
uint64 preedit_surface_len(PreeditState* ps);
void next_candidate(Candidate* c);
void before_candidate(Candidate* c);
uint64 len_to_space(const char *);
uint64 min(uint64 a, uint64 b);



// Input Method

void (*current_im)(Request, State*);

void initIM(State* state, PrevState* prev_state) {
    // Input Methodの初期化
    // IMEのスポーン
    current_im = plain_ime;
    // current_im = emoji_ime;
    state->type = state_preedit;
    state->state.preeditState.preedit_len = 0;
    prev_state->type = state_done;
    prev_state->remove_len = 0;
}


void handleInput(State* state, char input) {
    // 入力のハンドリング
    // IMEにルーティングをする
    Request req;
    req.type = req_input;
    req.body.input = input;

    (*current_im)(req, state);
}

void setPrevState(State* state, PrevState* prev_state) {
    prev_state->type = state->type;
    switch (state->type) {
    case state_preedit:
        prev_state->remove_len = preedit_surface_len(&state->state.preeditState);
        break;
    case state_select:
        break;
    case state_done:
        break;
    }
}


// IME

// void plain_ime(Request req, State* state) {
//     state->type = state_done;
//     state->state.doneState.result->len = 1;
//     state->state.doneState.result->data[0] = req.body.input;
//     state->state.doneState.result_len = 1;
// }

void plain_ime(Request req, State* state) {
    if (req.body.input == 'x') {
        state->type = state_done;
        state->state.doneState.result->len = strlen("あ");
        safestrcpy(state->state.doneState.result->data, "あ", strlen("あ")+1);
        state->state.doneState.result_len = 1;
        return;
    }


    state->type = state_done;
    state->state.doneState.result->len = 1;
    state->state.doneState.result->data[0] = req.body.input;
    state->state.doneState.result_len = 1;
    return;
    
}

// void emoji_ime(Request req, State* state) {
//     // PreeditChar preedit[IM_MAX_STRING_LENGTH];
//     unsigned int cs_idx, c_idx;

//     switch (req.type) {
//     case req_input:
//         switch (state->type) {
//         case state_preedit:
//             switch (req.body.input) {
//             case ':':
//                 if (state->state.preeditState.preedit_len < 1) {
//                     state->type = state_preedit;
//                     state->state.preeditState.preedit_len = 1;
//                     state->state.preeditState.preedit[0].raw[0] = ':';
//                     state->state.preeditState.preedit[0].surface[0] = ':';
//                     state->state.preeditState.preedit[0].raw[1] = '\0';
//                     state->state.preeditState.preedit[0].surface[1] = '\0';
//                 } else {
//                     char preedit_surface[IM_MAX_STRING_LENGTH];
//                     int x = 0, n = 0;
//                     for (int i = 1; n < IM_MAX_STRING_LENGTH && i < state->state.preeditState.preedit_len; i++) {
//                         n = strlen(state->state.preeditState.preedit[i].surface);
//                         safestrcpy(preedit_surface+x, state->state.preeditState.preedit[i].surface, IM_MAX_STRING_LENGTH);
//                         x += n;
//                     }

//                     state->type = state_select;
//                     state->state.selectState.candidates_len = 1;
//                     state->state.selectState.idx = 0;
//                     state->state.selectState.candidates[0].candidate_len = 0;
//                     state->state.selectState.candidates[0].idx = 0;

//                     // TODO: 効率的な実装に変える
//                     int i;
//                     i = 0;
//                     while (i < EMOJI_DICT_SIZE) {
//                         if (key_match(preedit_surface, emoji_dictionary[i][0])) {
//                             safestrcpy(state->state.selectState.candidates[0].candidate[state->state.selectState.candidates[0].candidate_len].string, emoji_dictionary[i][1], IM_MAX_STRING_LENGTH);
//                             state->state.selectState.candidates[0].candidate_len++;
//                         }
//                         i++;
//                     }

//                     state->state.selectState.candidates[0].preedit_len = 0;
//                 }
//                 break;
//             default:
//                 if (state->state.preeditState.preedit_len >= 1) {
//                     int next = state->state.preeditState.preedit_len;
//                     state->state.preeditState.preedit[next].raw[0] = req.body.input;
//                     state->state.preeditState.preedit[next].raw[1] = '\0';
//                     state->state.preeditState.preedit[next].surface[0] = req.body.input;
//                     state->state.preeditState.preedit[next].surface[1] = '\0';
//                     state->state.preeditState.preedit_len++;
//                 } else {
//                     state->type = state_done;
//                     state->state.doneState.result[0] = req.body.input;
//                     state->state.doneState.result[1] = '\0';
//                     state->state.doneState.result_len = strlen(state->state.doneState.result);
//                 }
                
//             }
//             break;
//         case state_select:
//             switch (req.body.input) {
//             case UP_KEY:
//                 before_candidate(&state->state.selectState.candidates[state->state.selectState.idx]);
//                 break;
//             case DOWN_KEY:
//                 next_candidate(&state->state.selectState.candidates[state->state.selectState.idx]);
//                 break;
//             case ENTER_KEY:
//                 state->type = state_done;
//                 cs_idx = state->state.selectState.idx;
//                 c_idx = state->state.selectState.candidates[cs_idx].idx;
//                 safestrcpy(state->state.doneState.result, state->state.selectState.candidates[cs_idx].candidate[c_idx].string, IM_MAX_STRING_LENGTH);
//                 state->state.doneState.result_len = strlen(state->state.selectState.candidates[cs_idx].candidate[c_idx].string);
//                 break;
//             default:
//                 break;
//             }
//             break;
//         case state_done:
//             break;
//         }
            
//     }
// }


int key_match(const char *keyword, const char *value) {
    int i = 0;
    while (keyword[i] != '\0') {
        if (keyword[i] != value[i]) return 0;
        i++;
    }
    return 1;
}


// Lib

uint64 preedit_surface_len(PreeditState* ps) {
    uint64 result = 0;
    for (int i = 0; i < ps->preedit_len; i++) {
        for (int j = 0; j < IM_MAX_STRING_LENGTH; j++) {
            result += j;
            if (ps->preedit[i].surface[j] == '\0') {
                break;
            }
        }
    }

    return result;
}

void next_candidate(Candidate* c) {
    if (c->idx < c->candidate_len-1) {
        c->idx++;
    }
}

void before_candidate(Candidate* c) {
    if (0 < c->idx) {
        c->idx--;
    }
}

uint64 len_to_space(const char* c) {
    int i = 0;
    while (c[i] != '\0' || c[i] != ' ') {
        i++;
    }
    return i;
}

uint64 min(uint64 a, uint64 b) {
    if (a < b) return a;
    return b;
}