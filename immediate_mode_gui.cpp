#include <SFML/Graphics.hpp>
#include <random>
#include <cstdlib>
#include <cmath>
#include <utility>


//get unique IMGUI_SRC_ID from a macro
#ifdef IMGUI_SRC_ID
#define GEN_ID ((IMGUI_SRC_ID) + (__LINE__))
#else
#define GEN_ID (__LINE__)
#endif

//notes from the fantastic tutorial on http://sol.gfxile.net/imgui/ch01.html by Jari Komppa

const sf::Vector2u g_screenRes{1024,768}; 
const std::string g_title = "Immediate Mode GUIs";

struct MEvt {
  int x{0},y{0};
  bool lmousedown;
  int hotitem{0};
  int activeitem{0};

  //keyboard data
  int kbditem{0}; //id of widget that has keyboard focus
  int keyentered{0}; //stores pressed eky
  int keymod{0}; //modifier flags (e.g. shift, not using it rn tho)

  int lastwidget{0}; //id of last widget that was processed
} g_mouseState;



//updates the global mouse state
void poll(sf::RenderWindow &w){
    sf::Event evt;
    w.pollEvent(evt);

    if ( evt.type == sf::Event::Closed ) {
      w.close();
    } else if ( evt.type == sf::Event::MouseButtonPressed
             && evt.mouseButton.button == sf::Mouse::Left ) {
      g_mouseState.lmousedown = true;
      g_mouseState.x = evt.mouseButton.x;
      g_mouseState.y = evt.mouseButton.y;
    } else if ( evt.type == sf::Event::MouseButtonReleased
             && evt.mouseButton.button == sf::Mouse::Left ) {
      g_mouseState.lmousedown = false;
      g_mouseState.x = evt.mouseButton.x;
      g_mouseState.y = evt.mouseButton.y;
    } else if (evt.type == sf::Event::MouseMoved) {
      g_mouseState.x = evt.mouseMove.x;
      g_mouseState.y = evt.mouseMove.y;
    } else if (evt.type == sf::Event::TextEntered) {
      g_mouseState.keyentered = evt.TextEntered;
    }

//updates entities based on dt and mouse state
void update(float dt){
}


//general intersection test
bool mouseIntersects(int x, int y, int w, int h) {
  if (g_mouseState.x < x || 
      g_mouseState.y < y || 
      g_mouseState.x >= x + w ||
      g_mouseState.y >= y + h )
    return false;

  return true;
}

void drawrect(int x, int y, int w, int h, sf::Color col, sf::RenderWindow& rw)
{
  sf::RectangleShape rect(sf::Vector2f(w,h));
  rect.setPosition(x,y);
  rect.setFillColor(col);
  rw.draw(rect);
}

//simple button widget
//I would like a version here where I didn't have to pass the render window (facade I guess?)
bool button(int id, int x, int y, sf::RenderWindow& c)
{
  //hovering over mouse? (i.e. should it be hot)
  if (mouseIntersects(x,y,64,48))
  {
    g_mouseState.hotitem = id;
    //should it be active?
    if (g_mouseState.activeitem == 0 && g_mouseState.lmousedown)
      g_mouseState.activeitem = id;
  }

  // -- keybd section --
  //if no widget has keyboard focus, take it
  if (g_mouseState.kbditem == 0)
    g_mouseState.kbditem = id;

  //if we have keyboard focus, show it
  if (g_mouseState.kbditem == id)
    drawrect(x-6,y-6,84,68,sf::Color::Red, c);

  //render button
  drawrect(x+8,y+8,64,48, sf::Color::Blue, c);

  //this is the key state machine
  if (g_mouseState.hotitem == id)
  {
    if (g_mouseState.activeitem == id)
    {
      drawrect(x+2,y+2, 64, 48, sf::Color::Cyan, c);
    } else { //button is merely "hot"
      drawrect(x, y, 64, 48, sf::Color::Cyan, c);
    }
  }
  else //button is not hot, but it may be active
  {
    drawrect(x, y, 64, 48, sf::Color::Green, c);
  }


  //finally, check if button has been triggered
  if (g_mouseState.lmousedown &&
      g_mouseState.hotitem == id &&
      g_mouseState.activeitem == id)
    return 1;

  return 0;
}

// returns true when value has been modified
bool slider(int id, int x, int y, int max, int &value, sf::RenderWindow &c)
{
  int ypos = ((256.0-16.0) * value) / max;

  //check for hotness
  if (mouseIntersects(x+8,y+8, 16, 255))
  {
    g_mouseState.hotitem = id;
    if (g_mouseState.activeitem == 0 && g_mouseState.lmousedown)
      g_mouseState.activeitem = id;
  }

  drawrect(x,y,32,256+16, sf::Color(70,70,70), c);

  if (g_mouseState.activeitem == id || g_mouseState.hotitem == id)
  {
    drawrect(x+8,y+8+ypos, 16, 16, sf::Color::White, c);
  }
  else
  {
    drawrect(x+8, y+8+ypos, 16, 16, sf::Color(200,200,200), c);
  }

  if (g_mouseState.activeitem == id)
  {
    int pos = g_mouseState.y - (y+8);
    if (pos < 0) pos=0;
    if (pos > 255) pos = 255;
    int v = (pos * max)/255;
    if (v != value)
    {
      value = v;
      return true;
    }
  }
  return false;
}

void imgui_prepare()
{
  g_mouseState.hotitem = 0;
}

void imgui_finish()
{
  if ( !g_mouseState.lmousedown ){
    g_mouseState.activeitem = 0;
  }
  else
  {
    if (g_mouseState.activeitem == 0)
      g_mouseState.activeitem = -1;
  }
}

//bg set-up
sf::Color bgColor = sf::Color::White;


int r=0;
int g=0;
int b=0;
//draw all entities
void draw(sf::RenderWindow &w){
  imgui_prepare();
  drawrect(100,100,150,50,sf::Color::Green,w);

  button(GEN_ID,100,200,w);
  button(GEN_ID,100,300,w);

  if (button(GEN_ID,200,150,w)){
    bgColor = sf::Color::Black;
  }


  if (slider(GEN_ID, 300, 100, 255, r, w)){
    bgColor = sf::Color(r,g,b);
  }
  if (slider(GEN_ID, 400, 100, 255, g, w)){
    bgColor = sf::Color(r,g,b);
  }
  if (slider(GEN_ID, 500, 100, 255, b, w)){
    bgColor = sf::Color(r,g,b);
  }

  imgui_finish();
}


void init(){
}


int framenum=0;
int main()
{
  sf::RenderWindow win(sf::VideoMode(g_screenRes.x, g_screenRes.y), g_title);

  init();

  sf::Clock timer;
  while(win.isOpen()){
    poll(win);
    update(timer.getElapsedTime().asSeconds());
    timer.restart();  //get a new delta every time
    win.clear(bgColor);
    draw(win);
    win.display();
    framenum++;
    //sf::sleep(sf::milliseconds(10));
  }
}
