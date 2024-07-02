// Input Method

#define IM_MAX_STRING_LENGTH 256  // TODO: 可変長にしたい

struct preeditchar_t {
    char* surface;
    char* raw;
} typedef PreeditChar;


struct candidate_string_t {
    char string[IM_MAX_STRING_LENGTH];
    char annotation[IM_MAX_STRING_LENGTH];
} typedef CandidateString;


struct candidate_t {
    PreeditChar preeditString[IM_MAX_STRING_LENGTH];
    unsigned int idx;
    CandidateString candidate[IM_MAX_STRING_LENGTH];
} typedef Candidate;


struct preedit_state_t {
    PreeditChar preedit[IM_MAX_STRING_LENGTH];  // preedit文字列
    // TODO: 予測変換
} typedef PreeditState;


struct select_state_t {
    Candidate candidates[IM_MAX_STRING_LENGTH];
    unsigned int idx;
} typedef SelectState;


struct done_state_t {
    char result[IM_MAX_STRING_LENGTH];  // 結果
} typedef DoneState;


union _State {
    PreeditState preeditState;
    SelectState selectState;
    DoneState doneState;
};

enum state_type_t {
    preedit,
    select,
    done,
} typedef StateType;


struct state_t {
    StateType type;
    union _State state;
} typedef State;


enum request_type_t {
    input,
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
void initIM();
