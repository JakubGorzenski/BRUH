/*  For-loop based            */
/*  Immiediete mode           */
/*  Graphical User Interface  */
/*  Assembler                 */

#include "cgl.h"

#define FIGA_NEW_UI(out)\
    for(for_keep fk = figa_init();\
    int_figa.state < 3;\
    figa_next_state(&fk, out))

#define FIGA_CODE\
    for(bool fk = true;\
    int_figa.state == 2 && fk;\
    fk = false)

#define FIGA_WIDGET\
    for(for_keep fk = figa_open_block(false), fk_null =\
    int_figa.state != 0 ?\
        figa.gui->run_block = true,\
    (for_keep){0} : (for_keep){0};\
    int_figa.state != 0 &&\
        figa_run_block(&fk) ||\
        (figa_close_block(&fk), 0);\
    (void)fk_null)

#define FIGA_HOLDER(func)\
    for(for_keep fk = figa_open_block(false), fk_null =\
    int_figa.state != 0 ?\
        (func),\
        figa_post_func(&fk, true),\
    (for_keep){0} : (for_keep){0};\
    figa_run_block(&fk) ||\
        (figa_close_block(&fk), 0);\
    (void)fk_null)

#define FIGA_TOGGLE(func)\
    for(for_keep fk = figa_open_block(false), fk_null =\
    int_figa.state != 0 ?\
        figa_post_func(&fk, (func)),\
    (for_keep){0} : (for_keep){0};\
    figa_run_block(&fk) ||\
        (figa_close_block(&fk), 0);\
    (void)fk_null)

#define FIGA_BUTTON(func)\
    for(for_keep fk = figa_open_block(true), fk_null =\
    int_figa.state != 0 ?\
        figa_post_func(&fk, (func)),\
    (for_keep){0} : (for_keep){0};\
    figa_run_block(&fk) ||\
        (figa_close_block(&fk), 0);\
    (void)fk_null)



typedef struct {
    v2di size;
    sint padding;
    bool is_vertical;
    bool run_block;
} gui;
typedef struct {
    gui*    parent;
    sprite* par_out;
    sprite  out;
    bool    run_block;
    bool    is_button;
} for_keep;
struct {
    gui*  all;
    sint  ptr;
    sint  count;
    sint  state;
} int_figa;
struct {
    gui*    gui;
    sprite* out;
} figa;



for_keep figa_init() {
    int_figa.state = 0;
    int_figa.count = 0;
    return (for_keep){0};
}
void figa_next_state(for_keep* fk, sprite out) {
    switch(int_figa.state++) {
      case 0: { //  allocate all figa_gui
        int_figa.count++;
        int_figa.all = MemTemp(sizeof(gui) * int_figa.count);

        int_figa.ptr = 0;
        fk->parent = figa.gui;
        figa.gui = &int_figa.all[int_figa.ptr++];

        static sprite spr = {0};
        figa.out = &spr;
        break;
    } case 1: { //  get sprite
        fk->out = SprSize(out, figa.gui->size);
        figa.out = &fk->out;
        int_figa.ptr = 1;
        break;
    } case 2: { //  clean up

    }}
}

for_keep figa_open_block(bool is_button) {
    for_keep fk = {0};
    fk.is_button = is_button;
    switch(int_figa.state) {
      case 0: { //  count block
        int_figa.count++;
        break;
    } case 1: { //  indent block
        fk.parent = figa.gui;
        figa.gui = &int_figa.all[int_figa.ptr++];
        *figa.gui = (gui){0};
        break;
    } case 2: { //  draw block
        fk.parent = figa.gui;
        fk.par_out = figa.out;
        figa.gui = &int_figa.all[int_figa.ptr++];
        
        if(fk.parent->is_vertical)
            fk.out = SprCutT(fk.par_out, figa.gui->size.h);
        else
            fk.out = SprCutL(fk.par_out, figa.gui->size.w);
        figa.out = &fk.out;
        break;
    }}
    return fk;
}
void figa_post_func(for_keep* fk, bool func_ret) {
    switch(int_figa.state) {
      case 0: { //  unreachable
        break;
    } case 1: { //  set toggle state
        if(!fk->is_button)
            figa.gui->run_block = func_ret;
        break;
    } case 2: { //  set button state & get inner sprite
        if(fk->is_button)
            figa.gui->run_block = func_ret;
        
        if(figa.gui->is_vertical)
            SprCutT(figa.out, figa.gui->size.h);
        else
            SprCutL(figa.out, figa.gui->size.w);

        *figa.out = SprSize(*figa.out, v2diVN(figa.out->size, -, figa.gui->padding));
        *figa.out = SprMove(*figa.out, v2di(figa.gui->padding, figa.gui->padding));
        break;
    }}
}
bool figa_run_block(for_keep* fk) {
    if(int_figa.state == 0 && fk->is_button)
        return false;
    if(int_figa.state != 0 && !figa.gui->run_block)
        return false;
    fk->run_block = !fk->run_block;
    return fk->run_block;
}
void figa_close_block(for_keep* fk) {
    switch(int_figa.state) {
    case 0: return; //  do nothing
    case 1: break;
    case 2: figa.out = fk->par_out; break;
    }

    gui* par = fk->parent;
    figa.gui->size = v2diVN(figa.gui->size, +, 2 * figa.gui->padding);
    if(par->is_vertical) {
        par->size.w = IntMax(par->size.w, figa.gui->size.w);
        par->size.h += figa.gui->size.h;
    } else {
        par->size.w += figa.gui->size.w;
        par->size.h = IntMax(par->size.h, figa.gui->size.h);
    }
    figa.gui = par;
}
