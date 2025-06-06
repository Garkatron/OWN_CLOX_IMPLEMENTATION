program        → declaration* EOF ;

declaration    → classDecl 
               | funDecl
               | varDecl
               | statement ;

classDecl      → "class" IDENTIFIER ( "<" IDENTIFIER )?
                 "{" function* "}" ;
                 
statement      → exprStmt
               | printStmt
               | block ;

block          → "{" declaration* "}" ;

declaration    → classDecl
               | funDecl
               | varDecl
               | statement ;

returnStmt     → "return" expression? ";" ;

funDecl        → "fun" function ;
function       → IDENTIFIER "(" parameters? ")" block ;

forStmt        → "for" "(" ( varDecl | exprStmt | ";" )
                 expression? ";"
                 expression? ")" statement ; 

parameters     → IDENTIFIER ( "," IDENTIFIER )* ;

whileStmt      → "while" "(" expression ")" statement ;
               ( "else" statement)? ;

ifStmt         → "if" "(" expression ")" statement
               ( "else" statement )? ;

loopStmt       → "loop" statement

breakStmt      → "break" ";" ; 

block          → "{" declaration* "}" ;

exprStmt       → expression ";" ;
printStmt      → "print" expression ";" ;

varDecl        → "var" IDENTIFIER ( "=" expression )? ";" ;

expression     → assignment ;
assignment     → ( call "." )? IDENTIFIER "=" assignment
               | ternary ;
               | logic_or ;

logic_or       → logic_and ( "or" logic_and )* ;
logic_and      → equality ( "and" equality )* ;

ternary        → comma ("?" expression ":" ternary)?;
comma          → equality ( "," equality )* ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary 
               | call
               | primary ;

call           → primary ( "(" arguments? ")" | "." IDENTIFIER )* ;
arguments      → expression ( "," expression )* ;

primary        → "true" | "false" | "nil" | "this"
               | NUMBER | STRING | IDENTIFIER | "(" expression ")"
               | "super" "." IDENTIFIER ;
