#include "Pair.h"

Pair::Pair(string first, string second) : first(first), second(second) {}

string Pair::getFirst(){
    return first;
}
string Pair::getSecond(){
    return second;
}