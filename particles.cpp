#include <SFML/Graphics.hpp>
#include <random>
#include <cstdlib>
#include <cmath>


struct particle : sf::CircleShape{
  sf::Vector2f velocity;
  float mass;
};

// globals begin
const int NUM_PARTICLES = 65;
const float PARTICLE_MASS_MEAN = 1.88;
const float PARTICLE_MASS_VARIANCE = 121.0;
const float PARTICLE_MASS_SCALING = 1.0/2.55;
const float PARTICLE_GRAVITY = 6.0;
const float MIN_PARTICLE_MASS = 0.8;
const float MIN_PARTICLE_DIST = 1.0;
const int SCREEN_WIDTH  = 1024;
const int SCREEN_HEIGHT = 768;
const float MOUSE_ATTRACTION = 4200;
//non-const
particle particle_pool[NUM_PARTICLES];
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
  r = r > MIN_PARTICLE_DIST ? r : MIN_PARTICLE_DIST;  //speed limit
  return norm(d)*MOUSE_ATTRACTION/(r*r);
}

//pairwise gravitational influence
void computeParticleGravity(float dt){
  for(int i=0;i<NUM_PARTICLES;++i){
    for(int j=i+1;j<NUM_PARTICLES;++j){
      particle &a = particle_pool[i];
      particle &b = particle_pool[j];
      sf::Vector2f d = b.getPosition() - a.getPosition();
      float r = size(d);
      float mindist = (a.getRadius() + b.getRadius())/2.0f;
      r = r > mindist ? r : mindist;
      float F = dt * a.mass * b.mass * PARTICLE_GRAVITY / (r*r);
      d = norm(d);
      a.velocity += d*F/a.mass;
      b.velocity += (-d)*F/b.mass;
    }
  }
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

    computeParticleGravity(dt);

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
  //distiributions
  std::default_random_engine generator;
  generator.seed(std::random_device()());
  std::uniform_real_distribution<float> xpos_dis(0,SCREEN_WIDTH);
  std::uniform_real_distribution<float> ypos_dis(0,SCREEN_HEIGHT);
  std::exponential_distribution<float> mass_dis(PARTICLE_MASS_MEAN);

  for(int i=0;i<NUM_PARTICLES;++i){
    float mass=-1;
    //while (mass<=MIN_PARTICLE_MASS)  //rejection sampling
    mass = mass_dis(generator)*PARTICLE_MASS_VARIANCE + MIN_PARTICLE_MASS;
    particle_pool[i].setPosition(xpos_dis(generator),ypos_dis(generator));
    particle_pool[i].velocity = sf::Vector2f(0,0);
    particle_pool[i].mass = mass;
    particle_pool[i].setRadius(std::pow(mass,PARTICLE_MASS_SCALING));
    particle_pool[i].setOutlineColor(sf::Color::Blue);
  }
}

// ==== Main Program Flow ====

int framenum=0;
int main()
{
  sf::RenderWindow win(sf::VideoMode(SCREEN_WIDTH,SCREEN_HEIGHT), "Particle Attraction Demo");

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
