/*  ENGINE FUNCTIONS  */
#ifdef _WIN64
    #include "bruh_win.c"
#elif __wasm__
    #include "bruh_wasm.c"
#endif


//  fix with spr_move/spr_size ? maybe ?
//  or remove bc useless?
/*sprite SprFlip(sprite spr) {   
    spr.buffer += spr.real_width * (spr.size.h - 1);
    spr.real_width = -spr.real_width;
    return spr;
}*/
pixel Rgb(uint rgb) {
    uchar r = rgb >> 16, g = rgb >> 8, b = rgb >> 0;
    return Rgb3(r, g, b);
}


v2di v2diMin(v2di a, v2di b) {
    a.x = a.x < b.x ? a.x : b.x;
    a.y = a.y < b.y ? a.y : b.y;
    return a;
}
v2di v2diMax(v2di a, v2di b) {
    a.x = a.x > b.x ? a.x : b.x;
    a.y = a.y > b.y ? a.y : b.y;
    return a;
}
v2di v2diClamp(v2di min, v2di val, v2di max) {
    val = v2diMin(val, max);
    val = v2diMax(val, min);
    return val;
}

bool is_inside(v2di p, v2di pos, v2di size) {
    p = v2diVV(p, -, pos);

    if(v2diVN(p, <, 0).all_bits || v2diVV(p, >=, size).all_bits)
        return false;
    else
        return true;
}


sprite SprMove(sprite spr, v2di pos) {
    spr.size = v2diVV(spr.size, -, pos);
    spr.start = v2diMax(v2diVV(spr.start, -, pos), v2di(0, 0));
    spr.buffer += pos.x + pos.y * spr.real_width;
    return spr;
}
sprite SprSize(sprite spr, v2di size) {
    spr.size = v2diMin(spr.size, size);
    return spr;
}
sprite SprWin(sprite spr, v2di pos, v2di size) {
    spr = SprMove(spr, pos);
    spr = SprSize(spr, size);
    return spr;
}

sprite SprCutL(sprite* spr, sint cut_by) {
    if(cut_by < 0)
        return (sprite){0};
    sprite out = SprSize(*spr, v2di(cut_by, spr->size.h));
    *spr = SprMove(*spr, v2di(cut_by, 0));
    return out;
}
sprite SprCutR(sprite* spr, sint cut_by) {
    sprite out = *spr;
    *spr = SprCutL(&out, spr->size.w - cut_by);
    return out;
}
sprite SprCutT(sprite* spr, sint cut_by) {
    if(cut_by < 0)
        return (sprite){0};
    sprite out = SprSize(*spr, v2di(spr->size.w, cut_by));
    *spr = SprMove(*spr, v2di(0, cut_by));
    return out;
}
sprite SprCutB(sprite* spr, sint cut_by) {
    sprite out = *spr;
    *spr = SprCutT(&out, spr->size.h - cut_by);
    return out;
}


pixel* get_pixel(sprite spr, sint x, sint y) {
    return &spr.buffer[x + y * spr.real_width];
}

void draw_pixel(sprite out, v2di pos, pixel color) {
    if(is_inside(pos, out.start, out.size)) {
        *get_pixel(out, pos.x, pos.y) = color;
    }
}
void draw_fill(sprite out, pixel color) {
    for(sint y = out.start.y; y < out.size.h; y++)
    for(sint x = out.start.x; x < out.size.w; x++) {
        *get_pixel(out, x, y) = color;
    }
}
void draw_spr(sprite out, sprite in) {
    v2di size = v2diMin(in.size, out.size);
    v2di start = v2diMax(in.start, out.start);

    for(sint y = start.y; y < size.h; y++)
    for(sint x = start.x; x < size.w; x++) {
        *get_pixel(out, x, y) = *get_pixel(in, x, y);
    }
}
/*void draw_text(sprite out, uint color, cstr str) {
    out = spr_move(out, out.start);

    if(out.size.w < 3) return;
    if(out.size.h < 5) return;

    ushort font[95] = {
    000000, 022202, 055000, 057575, 036736, 041241, 024367, 022000, 012221, 042224, 052500, 002720, 000024, 000700, 000002, 011244,
    075557, 026227, 061247, 061216, 055711, 074616, 074757, 071222, 075757, 075717, 020020, 020022, 012421, 007070, 042124, 061202,
    075747, 025755, 065656, 034443, 065556, 074647, 074644, 034553, 055755, 072227, 071116, 055655, 044447, 057555, 057755, 025552,
    065644, 025563, 065655, 034716, 072222, 055557, 055552, 055575, 055255, 055222, 071247, 032223, 044211, 062226, 025000, 000007,
    042000, 000353, 044656, 000343, 011353, 003743, 012722, 007716, 044655, 020222, 020224, 004565, 022221, 000675, 000655, 00757,
    006564, 002531, 000564, 000326, 027222, 000556, 000552, 000577, 000525, 000522, 007247, 032423, 022222, 062126, 000471,
    };
    uchar c;
    sint x = 0;
    sint y = 0;
    for(uint i = 0; (c = str[i]); i++) {
        c -= ' ';
        if(c >= 95)
            continue;
        ushort data = font[c];
        if(data & 040000) *get_pixel(out, 0 + x, 0 + y) = color;
        if(data & 020000) *get_pixel(out, 1 + x, 0 + y) = color;
        if(data & 010000) *get_pixel(out, 2 + x, 0 + y) = color;
        if(data & 004000) *get_pixel(out, 0 + x, 1 + y) = color;
        if(data & 002000) *get_pixel(out, 1 + x, 1 + y) = color;
        if(data & 001000) *get_pixel(out, 2 + x, 1 + y) = color;
        if(data & 000400) *get_pixel(out, 0 + x, 2 + y) = color;
        if(data & 000200) *get_pixel(out, 1 + x, 2 + y) = color;
        if(data & 000100) *get_pixel(out, 2 + x, 2 + y) = color;
        if(data & 000040) *get_pixel(out, 0 + x, 3 + y) = color;
        if(data & 000020) *get_pixel(out, 1 + x, 3 + y) = color;
        if(data & 000010) *get_pixel(out, 2 + x, 3 + y) = color;
        if(data & 000004) *get_pixel(out, 0 + x, 4 + y) = color;
        if(data & 000002) *get_pixel(out, 1 + x, 4 + y) = color;
        if(data & 000001) *get_pixel(out, 2 + x, 4 + y) = color;
        x += 4;
        if(x + 2 >= out.size.w) {
            y += 6;
            x = 0;
            if(y + 4 >= out.size.h)
                return;
        }
    }
}*/


