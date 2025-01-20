#include "ctype.hp"

function islower(char chr)
{
    return chr >= 'a' && chr <= 'z';
}

function toupper(char chr)
{
    return islower(chr) ? (chr - 'a' + 'A') : chr;
}
