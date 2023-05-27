#include<bits/stdc++.h>
using namespace std;
enum class Type {
    Int,
    Float,
    IntLiteral,
    FloatLiteral,
    IntPtr,
    FloatPtr,
    null
};

struct Operand {
    string name;
    Type type;
    Operand( string = "null", Type = Type::null );
};
Operand::Operand( string s = "null", Type type ) {
    this->name = s;
    this->type = Type::null;
}
int main() {
    Operand operand;
    operand.name = "111";
    //operand.type = Type::Int;
    cout << int( operand.type ) << endl;
}