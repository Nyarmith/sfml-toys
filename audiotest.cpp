#include <SFML/Audio.hpp>
#include <random>
#include <cstdlib>
#include <cmath>
#include <vector>

double FREQ = 44100;
double PI   = 3.141529;

int main()
{
  std::vector<sf::Int16> samples;
  samples.reserve(FREQ*3);
  for ( double i=0; i<3*PI; i +=  PI/FREQ ){
    samples.push_back(std::sin(i)*(1024));
  }

  sf::SoundBuffer buffer;
  buffer.loadFromSamples(&samples[0], samples.size(), 1, FREQ);
  sf::Sound sound;
  sound.setBuffer(buffer);
  sound.play();
  return 0;
}
