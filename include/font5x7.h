#ifndef FONT5X7_H
#define FONT5X7_H

struct Font {
    char letter;
    char code[7][5];
};

extern const struct Font font[];

#endif
