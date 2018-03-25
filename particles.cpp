#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <cmath>


struct particle : sf::CircleShape{
  sf::Vector2f velocity;
  float mass;
};

// globals begin
const int NUM_PARTICLES = 100;
const int MAX_PARTICLE_MASS = 5;
particle particle_pool[NUM_PARTICLES];
const int SCREEN_WIDTH  = 1024;
const int SCREEN_HEIGHT = 768;
const float MOUSE_ATTRACTION = 420;
bool mousedown = false;
sf::Vector2f mousepos;
// globals end


// main loop handlers

//mouse and window events
void handleEvents(sf::RenderWindow &w){
    sf::Event evt;
    while(w.pollEvent(evt)){
      switch(evt.type){
        case sf::Event::Closed:
          w.close();
          break;
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
        default:
          break; //do nothing
      }
    }
}

float size(const sf::Vector2f &in){
  return std::sqrt(in.y*in.y+in.x*in.x);
}

sf::Vector2f norm(const sf::Vector2f &in){
  float ss = size(in);
  return sf::Vector2f(in.x/ss,in.y/ss);
}

sf::Vector2f mouseForce(const sf::Vector2f &p){
  sf::Vector2f d =  mousepos - p;
  float r = size(d);
  r = r > .5 ? r : .5;  //speed limit
  return norm(d)*MOUSE_ATTRACTION/(r*r);
}

void updateParticles(float dt){
  for(int i=0;i<NUM_PARTICLES;++i){
    particle &p = particle_pool[i];
    if (mousedown)
      p.velocity += mouseForce(p.getPosition())/p.mass;

    sf::Vector2f nextPos = p.getPosition() + p.velocity*dt;
    //screen bounds
    if (nextPos.x < 0 || nextPos.x > SCREEN_WIDTH)
      p.velocity.x *= -1;
    if (nextPos.y < 0 || nextPos.y > SCREEN_HEIGHT)
      p.velocity.y *= -1;

    p.setPosition(p.getPosition() + p.velocity*dt);
  }
}

//draw particles
void renderParticles(sf::RenderWindow &w){
  for(int i=0;i<NUM_PARTICLES;++i){
    w.draw(particle_pool[i]);
  }
}



void initParticles(){
  srand(time(NULL));
  for(int i=0;i<NUM_PARTICLES;++i){
    float mass = rand() % MAX_PARTICLE_MASS + 1.0;
    particle_pool[i].setPosition(rand()%SCREEN_WIDTH,rand()%SCREEN_HEIGHT);
    particle_pool[i].velocity = sf::Vector2f(0,0);
    particle_pool[i].mass = mass;
    particle_pool[i].setRadius(std::sqrt(mass));
    particle_pool[i].setOutlineColor(sf::Color::Blue);
  }
}

// ==== Main Program Flow ====

int framenum=0;
int main()
{
  sf::RenderWindow win(sf::VideoMode(1024,768), "Particle Attraction Demo");

  initParticles();

  sf::Clock timer;
  while(win.isOpen()){
    handleEvents(win);
    updateParticles(timer.getElapsedTime().asSeconds());
    timer.restart();  //get a new delta every time
    win.clear(sf::Color::Black);
    renderParticles(win);
    win.display();
    framenum++;
  }
}
