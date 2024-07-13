//
// Console input and output, to the uart.
// Reads are line at a time.
// Implements special input characters:
//   newline -- end of line
//   control-h -- backspace
//   control-u -- kill line
//   control-d -- end of file
//   control-p -- print process list
//

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
#include "proc.h"
#include "im.h"

#define BACKSPACE 0x100
#define C(x)  ((x)-'@')  // Control-x


void consoleprint(char* str);

//
// send one character to the uart.
// called by printf(), and to echo input characters,
// but not from write().
//
void
consputc(int c)
{
  uartputc_sync(c);
}

struct {
  struct spinlock lock;
  
  // input
#define INPUT_BUF_SIZE 128
  MultiChar buf[INPUT_BUF_SIZE];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} cons;


State im_state;
PrevState im_prev_state;

//
// user write()s to the console go here.
//
int
consolewrite(int user_src, uint64 src, int n)
{
  int i;

  for(i = 0; i < n; i++){
    char c;
    if(either_copyin(&c, user_src, src+i, 1) == -1)
      break;
    uartputc(c);
  }

  return i;
}

//
// user read()s from the console go here.
// copy (up to) a whole input line to dst.
// user_dist indicates whether dst is a user
// or kernel address.
//
int
consoleread(int user_dst, uint64 dst, int n)
{
  uint target;
  int c;

  target = n;
  acquire(&cons.lock);
  while(n > 0){
    // wait until interrupt handler has put some
    // input into cons.buffer.
    while(cons.r == cons.w){
      if(killed(myproc())){
        release(&cons.lock);
        return -1;
      }
      sleep(&cons.r, &cons.lock);
    }

    c = cons.buf[cons.r % INPUT_BUF_SIZE].data[0];

    if(c == C('D')){  // end-of-file
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        cons.r--;
      }
      break;
    }

    // copy the input byte to the user-space buffer.

    if(either_copyout(user_dst, dst, &cons.buf[cons.r % INPUT_BUF_SIZE].data, cons.buf[cons.r % INPUT_BUF_SIZE].len) == -1)
      break;

    dst += cons.buf[cons.r % INPUT_BUF_SIZE].len;
    n -= cons.buf[cons.r % INPUT_BUF_SIZE].len;
    cons.r++;

    if(c == '\n'){
      // a whole line has arrived, return to
      // the user-level read().
      break;
    }
  }

  release(&cons.lock);

  return target - n;
}


void consoleprint(char* str) {
  while (*str != '\0') {
    consputc(*str);
    str++;
  }
}

void m_consoleprint(MultiChar *str) {
  while (str->data[0] != '\0') {
    for (int i = 0; i < str->len; i++) {
      consputc(str->data[i]);
    }
    str++;
  }
}

void consoleprint_uint64(uint64 n) {
    if (n == 0) {
        consputc('0');
        return;
    }

    uint64 divisor = 1;
    // 最上位の桁を見つける
    while (n / divisor >= 10) {
        divisor *= 10;
    }

    // 最上位の桁から順に出力
    while (divisor > 0) {
        int digit = (n / divisor) % 10;
        consputc(digit + '0');
        divisor /= 10;
    }
}


//
// the console input interrupt handler.
// uartintr() calls this for input character.
// do erase/kill processing, append to cons.buf,
// wake up consoleread() if a whole line has arrived.
//
void
consoleintr(int c)
{
  acquire(&cons.lock);

  switch(c){
  case C('P'):  // Print process list.
    procdump();
    break;
  case C('U'):  // Kill line.
    while(cons.e != cons.w &&
          cons.buf[(cons.e-1) % INPUT_BUF_SIZE].data[0] != '\n'){
      cons.e--;
    }
    break;
  case C('H'): // Backspace
  case '\x7f': // Delete key
    if(cons.e != cons.w){
      cons.e--;
      if (cons.buf[cons.e % INPUT_BUF_SIZE].len > 1) {
        // マルチバイト文字っぽい
        // TODO: ホントは文字幅を計算して消したほうが良い
        uartputc_sync('\b');
      }
      uartputc_sync('\b');
      uartputc_sync(' ');
      uartputc_sync('\b');
      
    }
    break;
  default:
    if(c != 0 && cons.e-cons.r < INPUT_BUF_SIZE){
      c = (c == '\r') ? '\n' : c;

      // echo back to the user.
      // consputc(c);

      // store for consumption by consoleread().
      // cons.buf[cons.e++ % INPUT_BUF_SIZE] = c;
      setPrevState(&im_state, &im_prev_state);
      handleInput(&im_state, c);
      switch (im_state.type) {
        int cs_idx, c_idx;

        case state_preedit:
          // すでに描画されているpreeditを消す    
          if (im_prev_state.remove_len == 0) {
            consoleprint(" ");
          }
          consoleprint("\e[");
          consoleprint_uint64(im_prev_state.remove_len);
          consoleprint("D");
          consoleprint("\e[0J");

          // preeditを描画
          consoleprint("\e[4m"); // 下線

          for (int i = 0; i < im_state.state.preeditState.preedit_len; i++) {
            consoleprint(im_state.state.preeditState.preedit[i].surface);
          }

          consoleprint("\e[0m"); // 文字修飾をなくす
          break;
        case state_select:
          // すでに描画されているpreedit/selectを消す    
          if (im_prev_state.type == state_preedit) {
            // 変わったばかりなので、下段にselect lineはやす
            consoleprint("\n");
          }

          consoleprint("\e[0E\e[1K"); // select line消す
          consoleprint("Select: ");

          cs_idx = im_state.state.selectState.idx;
          c_idx = im_state.state.selectState.candidates[cs_idx].idx;
          consoleprint("\e[90m");  // 文字色黒
          consoleprint("\e[47m"); // 背景色白
          for (int i = 0; i < im_state.state.selectState.candidates[cs_idx].candidate_len; i++) {
            consoleprint("\e[47m"); // 背景色白
            consoleprint("|");
            if (c_idx == i) {
              consoleprint("\e[46m"); // 背景色シアン
            }
            consoleprint(im_state.state.selectState.candidates[cs_idx].candidate[i].string);
          }
          consoleprint("|\e[0m"); // 文字修飾をなくす
          break;
        case state_done:
          // select line消す
          if (im_prev_state.type == state_select) {
            consoleprint("\e[0E\e[2K\e[1A\e[0G");
            consoleprint("\e[");
            consoleprint_uint64(cons.e - cons.r + 2);
            consoleprint("C");
            consoleprint("\e[0J");
          }
          m_consoleprint(im_state.state.doneState.result);
          for (int i = 0; i < im_state.state.doneState.result_len; i++) {
            cons.buf[cons.e++ % INPUT_BUF_SIZE] = im_state.state.doneState.result[i];
          }
          im_state.type = state_preedit;
          im_state.state.preeditState.preedit_len = 0;
          break;
      }

      if(c == '\n' || c == C('D') || cons.e-cons.r == INPUT_BUF_SIZE){
        // wake up consoleread() if a whole line (or end-of-file)
        // has arrived.
        cons.w = cons.e;
        wakeup(&cons.r);
      }
    }
    break;
  }
  
  release(&cons.lock);
}

void
consoleinit(void)
{
  initlock(&cons.lock, "cons");

  uartinit();
  initIM(&im_state, &im_prev_state);

  // connect read and write system calls
  // to consoleread and consolewrite.
  devsw[CONSOLE].read = consoleread;
  devsw[CONSOLE].write = consolewrite;
}
