// Input Method

#include "types.h"

#define IM_MAX_STRING_LENGTH 256  // TODO: 可変長にしたい

struct preeditchar_t {
    char surface[IM_MAX_STRING_LENGTH];
    char raw[IM_MAX_STRING_LENGTH];
} typedef PreeditChar;


struct candidate_string_t {
    char string[IM_MAX_STRING_LENGTH];
} typedef CandidateString;


struct candidate_t {
    PreeditChar preeditString[IM_MAX_STRING_LENGTH];
    unsigned int preedit_len;
    unsigned int idx;
    CandidateString candidate[IM_MAX_STRING_LENGTH];
    unsigned int candidate_len;
} typedef Candidate;


struct preedit_state_t {
    PreeditChar preedit[IM_MAX_STRING_LENGTH];  // preedit文字列
    unsigned int preedit_len;
    // TODO: 予測変換
} typedef PreeditState;


struct select_state_t {
    Candidate candidates[IM_MAX_STRING_LENGTH];
    unsigned int candidates_len;
    unsigned int idx;
} typedef SelectState;


struct done_state_t {
    char result[IM_MAX_STRING_LENGTH];  // 結果
    unsigned int result_len;
} typedef DoneState;


union _State {
    PreeditState preeditState;
    SelectState selectState;
    DoneState doneState;
};

enum state_type_t {
    state_preedit,
    state_select,
    state_done,
} typedef StateType;


struct state_t {
    StateType type;
    union _State state;
} typedef State;


struct prev_state_t {
    StateType type;
    uint64 remove_len;
} typedef PrevState;


enum request_type_t {
    req_input,
} typedef RequestType;

union request_body_t {
    char input;
} typedef RequestBody;

struct request_t {
    RequestType type;
    RequestBody body;
} typedef Request;


// Functions

void handleInput(State* state, char input);
void initIM(State* state, PrevState* prev_state);
void setPrevState(State* state, PrevState* prev_state);


// Keys
#define SPACE_KEY 32
#define RIGHT_KEY 67
#define LEFT_KEY 68
#define UP_KEY 65
#define DOWN_KEY 66
#define ENTER_KEY 10
