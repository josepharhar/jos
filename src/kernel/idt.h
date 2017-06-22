#ifndef IDT_H_
#define IDT_H_

#ifdef __cplusplus
extern "C" {
#endif

void load_idt();
void load_tss();

#ifdef __cplusplus
}
#endif

#endif  // IDT_H_
