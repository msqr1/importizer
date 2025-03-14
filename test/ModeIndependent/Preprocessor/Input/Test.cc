/* Comment before directive */ #define A 2
const char* str{"A string literal""followed by another one"};

// Here is a singleline comment
const char* str1{R"(A raw string literal)"};

/*
This
is
a
multiline
comment
*/
const char* str2{R"hi(Raw string literal with arbitrary delimeter)hi"};

/*Chained multiline
*//*comment*/
const char* str3{"A string literal with escaped char\n"};
char c{'a'};
char c1{'\''};
char c2{'\32'};

// An separated integer literal
int oneThousand{1'000};

// Possible false positives for integer literals
char u{'2'};
char c3{u8'1'};

// A main function
int          main        () {

}