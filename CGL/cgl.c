/*  ENGINE FUNCTIONS  */
#ifdef _WIN64
    #include "cgl_win.c"
#elif __wasm__
    #include "cgl_wasm.c"
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


string StrC(char* cstr) {
    string ret = {0};

    ret.buffer = cstr;
    while(*cstr++)
        ret.length++;

    return ret;
}
string StrInt(slong n) {
    sint length = 0;
    slong d = n;
    if(n < 0) {
        length++;
        d = -n;
    }
    do {
        length++;
        d /= 10;
    } while(d >= 1);

    string ret = StrNew(MemTemp, length);
    if(ret.buffer_size) {
        ret.length = length;
        if(n < 0) {
            ret.buffer[0] = '-';
            n = -n;
        }
        do {
            ret.buffer[--length] = '0' + n % 10;
            n /= 10;
        } while(n >= 1);
    }
    return ret;
}
string StrFloat(double f) {
    union {
        double f;
        ulong ll;
    } fll = {f};
    if((fll.ll & 0x7ff0'0000'0000'0000) == 0x7ff0'0000'0000'0000) {
        if(fll.ll & 0x000f'ffff'ffff'ffff) {
            if(fll.ll & 0x8000'0000'0000'0000)
                return StrC("-nan");
            else
                return StrC("+nan");
        } else {
            if(fll.ll & 0x8000'0000'0000'0000)
                return StrC("-inf");
            else
                return StrC("+inf");
        }
    }

    sint length = 0;
    sint length_frac = 6;
    double d = f;
    if(f < 0) {
        length++;
        d = -f;
    }
    do {
        length++;
        d /= 10;
    } while(d >= 1);

    if(length >= 9)
        return StrCat(MemTemp, StrFloat(d * 10), StrC("e"), StrInt(length - 1));

    string ret = StrNew(MemTemp, length + length_frac + sizeof(char));
    if(ret.buffer_size) {
        ret.length = length;
        d = f;
        if(f < 0) {
            ret.buffer[0] = '-';
            f = -f;
            d = -f;
        }
        d -= (sint)f;
        do {
            ret.buffer[--length] = '0' + (sint)f % 10;
            f /= 10;
        } while(f >= 1);
        ret.buffer[ret.length++] = '.';
        for(sint i = 0; i < length_frac; i++) {
            d *= 10;
            ret.buffer[ret.length++] = '0' + (sint)d % 10;
        }
    }
    return ret;
}
string StrV2di(v2di p) {
    return StrCat(MemTemp, StrC("v2di("), StrInt(p.x), StrC(", "), StrInt(p.y), StrC(")"));
}

string StrNew(alloc alloc, sint size) {
    if(size <= 0)
        return StrC("");
    string ret = {0};
    ret.buffer = alloc(size);
    ret.buffer_size = size;
    if(ret.buffer)
        return ret;
    return StrC("");
}
string StrAppend(string* out, string append) {
    sint size = IntMin(out->buffer_size - out->length, append.length);

    for(sint i = 0; i < size; i++)
        out->buffer[out->length + i] = append.buffer[i];
    out->length += size;

    return *out;
}
string StrCatList(alloc alloc, uint count, string* str) {
    uint size = 0;
    for(uint i = 0; i < count; i++)
        size += str[i].length;
    
    string ret = StrNew(alloc, size);
    for(uint i = 0; i < count; i++)
        StrAppend(&ret, str[i]);
    
    return ret;
}
string StrCut(string* str, sint pos) {
    string ret = StrSub(*str, 0, pos);
    *str = StrSub(*str, pos, str->length - pos);
    return ret;
}
string StrSub(string str, sint start, sint length) {
    start  = IntMax(start,  0);
    length = IntMax(length, 0);

    str.buffer += start;
    str.buffer_size = 0;

    str.length = IntClamp(0, str.length - start, length);
    return str;
}

sint StrFind(string str, string look_for) {
    for(sint i = 0; i < str.length; i++)
        for(sint j = 0; str.buffer[i + j] == look_for.buffer[j]; j++)
            if(look_for.length <= j + 1)
                return i;
    return str.length;
}
bool StrIsEqual(string a, string b) {
    if(a.length != b.length)
        return false;
    
    for(sint i = 0; i < a.length; i++)
        if(a.buffer[i] != b.buffer[i])
            return false;
    
    return true;
}


slong IntMin(slong a, slong b) {
    return a < b ? a : b;
}
slong IntMax(slong a, slong b) {
    return a > b ? a : b;
}
slong IntClamp(slong min, slong val, slong max) {
    val = IntMin(val, max);
    val = IntMax(val, min);
    return val;
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
bool v2diIsEqual(v2di a, v2di b) {
    return (a.x == b.x) && (a.y == b.y);
}
bool v2diIsInside(v2di p, v2di pos, v2di size) {
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


void draw_pixel(sprite out, v2di pos, pixel color) {
    if(v2diIsInside(pos, out.start, out.size)) {
        GET_PIXEL(out, pos.x, pos.y) = color;
    }
}
void draw_fill(sprite out, pixel color) {
    for(sint y = out.start.y; y < out.size.h; y++)
    for(sint x = out.start.x; x < out.size.w; x++) {
        GET_PIXEL(out, x, y) = color;
    }
}
void draw_spr(sprite out, sprite in) {
    v2di size = v2diMin(in.size, out.size);
    v2di start = v2diMax(in.start, out.start);

    for(sint y = start.y; y < size.h; y++)
    for(sint x = start.x; x < size.w; x++) {
        GET_PIXEL(out, x, y) = GET_PIXEL(in, x, y);
    }
}
void draw_text(sprite out, v2di* cursor, string text, text_set* settings) {
    if(!settings)
        settings = &(text_set){.color = Rgb(0xffffff)};

    pixel color = settings->color;
    font* used_font = settings->font;
    if(!used_font) {
        used_font = &(font){
            .line_height = 7,
            .letter = {
                //  ' ' -> '/'
                {3, 7, 0},
                {1, 2, 0b1'1'1'1'0'1},
                {3, 6, 0b101'101},
                {5, 1, 0b01010'01010'11111'01010'11111'01010'01010},
                {3, 1, 0b010'111'100'111'001'111'010},
                {5, 2, 0b11001'11010'00100'01011'10011},
                {5, 2, 0b01000'10100'01101'10010'01101},
                {1, 6, 0b1'1},
                {2, 1, 0b01'10'10'10'10'10'01},
                {2, 1, 0b10'01'01'01'01'01'10},
                {3, 5, 0b101'010'101},
                {3, 3, 0b010'111'010},
                {2, 1, 0b01'10},
                {3, 4, 0b111},
                {1, 1, 0b1},
                {3, 2, 0b001'001'010'010'100'100},
                //  '0' -> '?'
                {3, 2, 0b111'101'101'101'111},
                {3, 2, 0b110'010'010'010'111},
                {3, 2, 0b111'001'111'100'111},
                {3, 2, 0b111'001'111'001'111},
                {3, 2, 0b101'101'111'001'001},
                {3, 2, 0b111'100'111'001'111},
                {3, 2, 0b111'100'111'101'111},
                {3, 2, 0b111'001'001'001'001},
                {3, 2, 0b111'101'111'101'111},
                {3, 2, 0b111'101'111'001'111},
                {1, 3, 0b1'0'0'1},
                {1, 2, 0b1'0'0'1'1},
                {3, 2, 0b001'010'100'010'001},
                {3, 3, 0b111'000'111},
                {3, 2, 0b100'010'001'010'100},
                {4, 2, 0b1110'0001'0110'0100'0000'0100},
                //  '@' -> 'O'
                {5, 1, 0b01110'10001'10111'10101'10111'10000'01111},
                {4, 2, 0b0110'1001'1111'1001'1001},
                {4, 2, 0b1110'1001'1110'1001'1110},
                {4, 2, 0b0110'1001'1000'1001'0110},
                {4, 2, 0b1110'1001'1001'1001'1110},
                {3, 2, 0b111'100'110'100'111},
                {3, 2, 0b111'100'110'100'100},
                {4, 2, 0b0110'1000'1011'1001'0110},
                {4, 2, 0b1001'1001'1111'1001'1001},
                {3, 2, 0b111'010'010'010'111},
                {3, 2, 0b111'001'001'001'110},
                {4, 2, 0b1001'1010'1100'1010'1001},
                {3, 2, 0b100'100'100'100'111},
                {5, 2, 0b10001'11011'10101'10001'10001},
                {5, 2, 0b10001'11001'10101'10011'10001},
                {4, 2, 0b0110'1001'1001'1001'0110},
                //  'P' -> '_'
                {4, 2, 0b1110'1001'1110'1000'1000},
                {4, 2, 0b0110'1001'1001'1010'0101},
                {4, 2, 0b1110'1001'1110'1001'1001},
                {4, 2, 0b0111'1000'1111'0001'1110},
                {5, 2, 0b11111'00100'00100'00100'00100},
                {4, 2, 0b1001'1001'1001'1001'0110},
                {5, 2, 0b10001'10001'01010'01010'00100},
                {5, 2, 0b10001'10001'10101'11011'10001},
                {3, 2, 0b101'101'010'101'101},
                {3, 2, 0b101'101'010'010'010},
                {4, 2, 0b1111'0001'0110'1000'1111},
                {2, 2, 0b11'10'10'10'10'11},
                {3, 2, 0b100'100'010'010'001'001},
                {2, 2, 0b11'01'01'01'01'11},
                {3, 6, 0b010'101},
                {4, 1, 0b1111},
                //  '`' -> 'o'
                {2, 6, 0b10'01},
                {3, 2, 0b011'101'011},
                {3, 2, 0b100'100'110'101'110},
                {3, 2, 0b011'100'011},
                {3, 2, 0b001'001'011'101'011},
                {3, 1, 0b011'111'100'011},
                {3, 2, 0b001'010'111'010'010},
                {3, 1, 0b110'111'001'110},
                {3, 2, 0b100'100'110'101'101},
                {3, 2, 0b010'000'110'010'111},
                {2, 1, 0b01'00'11'01'01'10},
                {3, 2, 0b100'100'101'110'101},
                {2, 2, 0b10'10'10'10'01},
                {5, 2, 0b11010'10101'10101},
                {3, 2, 0b110'101'101},
                {3, 2, 0b010'101'010},
                //  'p' -> '~'
                {3, 1, 0b110'101'110'100},
                {3, 1, 0b011'101'011'001},
                {3, 2, 0b101'110'100},
                {3, 2, 0b011'100'001'110},
                {3, 2, 0b010'111'010'010'010},
                {3, 2, 0b101'101'011},
                {3, 2, 0b101'101'010},
                {5, 2, 0b10001'10101'01010},
                {3, 2, 0b101'010'101},
                {3, 1, 0b101'101'010'110},
                {3, 2, 0b111'001'100'111},
                {3, 1, 0b001'010'010'100'010'010'001},
                {1, 1, 0b1'1'1'0'1'1'1},
                {3, 1, 0b100'010'010'001'010'010'100},
                {5, 3, 0b01000'10101'00010},
            },
        };
    }

    if(!cursor)
        cursor = &v2di(0, 0);

    for(sint i = 0; i < text.length; i++) {
        sint ch = text.buffer[i];

        bool invert = false;
        if(settings->bit_8_inverts)
            invert = (ch & 0x80) != 0;
        ch &= 0x7f;

        if(ch == ' ' && settings->word_wrap) {
            sint word_width = 0;
            for(sint j = 1; j < text.length; j++) {
                char ch = text.buffer[i + j];
                ch &= 0x7f;
                if(ch <= ' ' || ch > '~')
                    break;
                word_width += used_font->letter[ch - ' '].width + 1;
            }
            if(word_width + cursor->x > out.size.w) {
                cursor->x = 0;
                cursor->y += used_font->line_height + 1;
                continue;
            }
        }
        if(ch < ' ' || ch > '~') {
            if(settings->process_special && ch == '\n') {
                cursor->x = 0;
                cursor->y += used_font->line_height + 1;
            }
            continue;
        }
            
        ch -= ' ';

        v2di size = v2di(
            used_font->letter[ch].width,
            used_font->line_height - used_font->letter[ch].offset_y
        );
        ulong data = used_font->letter[ch].data;

        if(settings->letter_wrap && cursor->x + size.w > out.size.w) {
            cursor->x = 0;
            cursor->y += used_font->line_height + 1;
        }

        if(invert) {
            for(sint y = used_font->line_height; y >= 0; y--) {
            for(sint x = size.x; x >= 0; x--) {
                if(v2diIsInside(v2di(x, y), v2di(0, 0), v2diVV(size, +, v2di(0, 1)))) {
                    if((data & 1) == 0)
                        draw_pixel(out, v2di(x + cursor->x, y + cursor->y), color);
                    data >>= 1;
                } else
                    draw_pixel(out, v2di(x + cursor->x, y + cursor->y), color);
            }}
        } else {
            for(sint y = size.y; data; y--) {
            for(sint x = size.x - 1; x >= 0; x--) {
                if(data & 1)
                    draw_pixel(out, v2di(x + cursor->x, y + cursor->y), color);
                data >>= 1;
            }}
        }
        cursor->x += size.w + 1;
    }
}
