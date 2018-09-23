#include <SFML/Graphics.hpp>
#include <random>
#include <cstdlib>
#include <cmath>
#include <utility>
#include <functional>

//get unique IMGUI_SRC_ID from a macro
#ifdef IMGUI_SRC_ID
#define GEN_ID ((IMGUI_SRC_ID) + (__LINE__))
#else
#define GEN_ID (__LINE__)
#endif


const sf::Vector2u g_screenRes{1024,768}; 
const std::string g_title = "Nice Immediate Mode GUIs";


void drawrect(int x, int y, int w, int h, sf::Color col, sf::RenderWindow& rw)
{
  sf::RectangleShape rect(sf::Vector2f(w,h));
  rect.setPosition(x,y);
  rect.setFillColor(col);
  rw.draw(rect);
}


struct UIState{
  int hotitem{0};
  int activeitem{0};
};

struct MEvt {
  int x{0},y{0};
  bool lmousedown;
  UIState ui;
  void startUI()     { ui.hotitem = 0;    } //sets up a new frame
  void finishUI()    { if (!lmousedown) ui.activeitem = 0;} //use when active item is disengaged
} g_mouseState;

struct IEntity
{
  virtual void update(float dt)=0;
  virtual void draw(sf::RenderWindow& r)=0;
  virtual void handle(MEvt& mevt)=0;
};

std::vector<IEntity*> Entities;

struct GUIEntity : IEntity {

  //new retained-mode stuff
  std::string string_id_;
  static GUIEntity* getElementById(std::string search_id_) {
    //iterate through all types, and if it's a GUIEntity type that matches the id, then return that
    GUIEntity *ret{nullptr};
    for (IEntity* t : Entities) {
      GUIEntity* gent = dynamic_cast<GUIEntity*>(t);
      if (gent && gent->string_id_ == search_id_)
        ret = gent;
    }
    return ret;
  }
  //useful globals
  static int id_ctr_;
  static UIState& uistate_;
  static int active_id_;

  //info all GUIEntities must have
  int id_;
  sf::Vector2i pos_;
  sf::Vector2i size_;
  GUIEntity(std::string strid="") //slightly modified c-tor
  {
    id_ = id_ctr_++;
    string_id_ = strid;
  }

  bool intersect(sf::Vector2i pos)
  {
    if ( pos.x < pos_.x || pos.x >= pos_.x + size_.x ||
         pos.y < pos_.y || pos.y >= pos_.y + size_.y )
      return false;
    return true;
  }
};


int GUIEntity::id_ctr_ = 1;
UIState& GUIEntity::uistate_ = g_mouseState.ui;

struct Button : GUIEntity
{
  std::function<void()> cb_;
  void update(float) { /*do nothing*/ };
  Button(int x, int y, std::function<void()> callback, std::string strid="") 
    : GUIEntity(strid), cb_(callback)
  {
    pos_ = {x,y};
    size_ = {64,48};
  };

  void draw(sf::RenderWindow& r)
  {
    if (uistate_.activeitem == id_) //being clicked
    {
      drawrect(pos_.x+2, pos_.y+2, size_.x, size_.y, sf::Color::Cyan, r);
      cb_();
    }
    else if (uistate_.hotitem == id_ && uistate_.activeitem != id_)
    {
      drawrect(pos_.x, pos_.y, size_.x, size_.y, sf::Color::Cyan, r);
    }
    else //I am neither hot nor active
    {
      drawrect(pos_.x, pos_.y, size_.x, size_.y, sf::Color::Green, r);
    }
  }

  void handle (MEvt& mevt)
  {
    if ( intersect({mevt.x,mevt.y}) )
    {
      if (uistate_.hotitem == 0)
        uistate_.hotitem = id_;
      if (mevt.lmousedown && uistate_.activeitem == 0)
        uistate_.activeitem = id_;
    }
  }
};

struct Slider : GUIEntity {
  int min_;
  int max_;
  int &ref_;
  std::function<void(int)> cb_; //optional callback
  Slider(int x, int y, int min, int max, int &value, std::function<void(int)> cb=nullptr, std::string strid="") 
    : GUIEntity(strid), min_(min), max_(max), ref_(value), cb_(cb)
  {
    pos_  = {x,y};
    size_ = {32,256};
  }
  void update(float) { /*do nothing*/ }
  void handle(MEvt& mevt)
  {
    if ( intersect({mevt.x,mevt.y}) )
    {
      if (uistate_.hotitem == 0)
        uistate_.hotitem = id_;
      if (mevt.lmousedown && uistate_.activeitem == 0){
        uistate_.activeitem = id_;
      }
    }
    //while I am the active item
    if (uistate_.activeitem == id_){
      //do slider action
      int pos = mevt.y - (pos_.y+8);
      if (pos < 0) pos = 0;
      if (pos >= size_.y) pos = size_.y-1;
      int v = (max_-min_)*(pos/255.0) + min_;
      if (v != ref_){
        ref_ = v;
        if (cb_ != nullptr){ cb_(ref_); };
      }
    }
  }

  void draw(sf::RenderWindow& rw)
  {
    //main bar always gets drawn the same way
    drawrect(pos_.x, pos_.y, size_.x, size_.y + size_.x/2, sf::Color(70,70,70), rw);
    int sliderDrawPos = ((ref_ - min_) / (float)(max_ - min_))*size_.y + (pos_.y + size_.x/4); //recompute slider's pos from its referenced val

    if (uistate_.hotitem == id_ && uistate_.activeitem == id_) 
      drawrect(pos_.x + size_.x/4, sliderDrawPos, size_.x/2, size_.x/2, sf::Color::White, rw);
    else 
      drawrect(pos_.x + size_.x/4, sliderDrawPos, size_.x/2, size_.x/2, sf::Color(200,200,200), rw);
  }
};


//updates the global mouse state
void poll(sf::RenderWindow &w)
{
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
    }

    //begin UI state
    g_mouseState.startUI();

    for (auto it=Entities.rbegin(); it != Entities.rend(); ++it)
      (*it)->handle(g_mouseState);
}

//updates entities based on dt and mouse state
void update(float dt){
  for (auto it : Entities)
    it->update(dt);
}
void draw(sf::RenderWindow &w)
{
  for (auto it : Entities)
    it->draw(w);
  g_mouseState.finishUI();
}



int framenum=0;
int main()
{
  sf::RenderWindow win(sf::VideoMode(g_screenRes.x, g_screenRes.y), g_title);
  int r=120,g=120,b=120;
  sf::Color bgColor(r,g,b);

  //rgb sliders, first one doesn't actually trigger the BG color value's update
  Entities.push_back(
    new Slider(300,100,0,255,r, [](int){},"red")
  );
  Entities.push_back(
    new Slider(400,100,0,255,g,[&](int){bgColor = sf::Color(r,g,b);}, "green")
  );
  Entities.push_back(
    new Slider(500,100,0,255,b,[&](int){bgColor = sf::Color(r,g,b);}, "blue")
  );

  //now we give this button functionality
  Entities.push_back(
    new Button(100,200, [](){
      Slider *rslider = dynamic_cast<Slider*>(GUIEntity::getElementById("red"));
      Slider *gslider = dynamic_cast<Slider*>(GUIEntity::getElementById("green"));
      rslider->ref_ = 120;
      gslider->ref_ = 120;
      gslider->cb_(120);
    })
  );
  
  //resets background to black
  Entities.push_back(
    new Button(100,300,[&](){bgColor = sf::Color::Black;})
  );

  sf::Clock timer;
  while(win.isOpen())
  {
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

