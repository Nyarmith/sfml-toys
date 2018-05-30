#include <SFML/Graphics.hpp>
#include <functional>
#include <algorithm>
#include <random>
#include <cstdlib>
#include <cmath>
#include <cctype>
#include <string>


//good opportunity for event-driven design rather than continuous real-time. Idea: use either semaphores or libevent
//suggestion: test c++ visitor implementation feasability before continuing implementation of this interpreter

namespace mathlang{
  using std::string;
  enum Tokens { EQ, PLUS, MINUS, MUL, DIV, EXP, LPAREN, RPAREN, ID, NUM };
  struct Token{
    Tokens type;
    string value;
    Token () {}
    Token (Tokens t_, string v_) { type = t_; value = v_;}
  };

  struct AST;
  struct NodeVisitor { string visit(AST); }; //todo finish visitor cases
  struct Visitable { virtual string accept(NodeVisitor); };

  struct AST : Visitable {
    Token token;
    AST() { }
    AST(Token tkn) : token(tkn) { }
    virtual string accept(NodeVisitor v) { return v.visit(*this); }
  };

  struct BinOp : AST {
    AST l,r;
    BinOp(AST l, Token op, AST r) : l(l), token(op), r(r) { };
    string accept(NodeVisitor v){ return v.visit(*this); } //can I avoid defining this explicitly? do test
  };

  struct UnOp : AST {
    AST v;
    UnOp(Token op, AST v) : token(op), v(v) { };
    string accept(NodeVisitor v){ return v.visit(*this); }
  };

  struct Var : AST {
    Var(Token tkn) : token(tkn) { }
    string accept(NodeVisitor v){ return v.visit(*this); }
  };

  struct Int : AST {
    Int(Token tkn) : token(tkn) { }
    string accept(NodeVisitor v){ return v.visit(*this); }
  };

  struct Lexer
  {
    string t;
    unsigned long pos;
    char c;
    Lexer(string text) : t(text) { pos=0; c=t[pos]; }
    void adv()  { ++pos; c = pos >= t.length() ? c = '\0' : c = t[pos]; }
    void err()  { /*throw descriptive error*/ };
    void seek() { while (c != '\0'&&std::isspace(c)) ++c; }
    char peek() { if (pos+1 >= t.length()) return '\0'; return t[pos+1]; }
    string num(){ /*scan current number*/ }
    Token id(){ /*get char at current pos*/ }
    Token next(){ /*get next token*/ }
  };

  struct Parser 
  {
    Lexer l;
    Token cur;
    Parser(Lexer l) : l(l) { cur = l.next(); }
    void err() {/*throw descriptive error*/ };
    void eat(Tokens t) {/*consumes current token, throws error if t is wrog*/ };
    AST factor(){
      Token t = cur;
      switch(t.type){
        case (NUM):
          eat(NUM);
          return Int(t);
          //other cases...
      }
    }
    //other syntax definition...
  };

  struct Interpreter; //TODO
}



//mouse and window events
void handleEvents(sf::RenderWindow &w){
    sf::Event evt;
    while(w.pollEvent(evt)){
      switch(evt.type){
        case sf::Event::Closed:
          w.close();
          break;
          /*
        case sf::Event::MouseButtonPressed:
          if (evt.mouseButton.button == sf::Mouse::Left)
            mousedown=true;
          mousepos = sf::Vector2f(sf::Mouse::getPosition(w));
          break;
        case sf::Event::MouseButtonReleased:
          if (evt.mouseButton.button == sf::Mouse::Left)
            mousedown=false;
          break;
        case sf::Event::MouseMoved:
          mousepos = sf::Vector2f(sf::Mouse::getPosition(w));
          break;
          */
        default:
          break; //do nothing
      }
    }
}

const int SCREEN_WIDTH=1300, SCREEN_HEIGHT=700;
// ==== Main Program Flow ====

int main()
{
  sf::ContextSettings s;
  s.antialiasingLevel = 8;
  sf::RenderWindow win(sf::VideoMode(SCREEN_WIDTH,SCREEN_HEIGHT), "Sfml Grapher Demo", sf::Style::Default, s);

  sf::Clock timer;
  while(win.isOpen()){
    handleEvents(win);
    //some update function with delta thing timer.getElapsedTime().asSeconds() if you want
    timer.restart();  //get a new delta every time
    win.clear(sf::Color::Black);
    //some draw function acting on win
    win.display();
  }
}
