#define GLM_ENABLE_EXPERIMENTAL
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <random>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <utility>
#include <functional>

const sf::Vector2u g_screenDim {800,640};

float rotation=1.0f;
glm::vec3 rotation_axis {.7071,.7071,0};
const std::vector<glm::vec3> g_vertices {{1,1,0},{-1,1,0},{0,-1,-1},{0,-1,1}};
const std::vector<float> g_indices {0,1,2,0,2,3,0,3,1,1,3,2};
std::vector<glm::vec3> g_normals;
const glm::vec3 g_cameraPos {0,0,-5};
const glm::vec3 g_cameraGaze {0,0,1};


void init()
{
  g_normals.reserve(4);
  for (int i=0; i<g_indices.size()/3; ++i){
    glm::vec3 a,b;
    a = g_vertices[ g_indices[i*3+1] ] - g_vertices[ g_indices[i*3+0] ];
    b = g_vertices[ g_indices[i*3+2] ] - g_vertices[ g_indices[i*3+1] ];
    g_normals.push_back( glm::normalize(glm::cross(a,b)) );
  }
}

void poll(sf::RenderWindow &win)
{
}

float twoPi = 2*glm::pi<float>();
void update(float)
{
  rotation += .1;
  if (rotation >= twoPi)
    rotation -= twoPi;
}


glm::vec3 intersect(glm::vec3 c, glm::vec3 g, std::vector<glm::vec3> &newnormals, std::vector<glm::vec3> &newvertices)
{
  //for each triangle
  for (int i=0; i<g_indices.size()/3; ++i){
    //does it even show up?
    float K = glm::dot(newnormals[i],g);
    if (K > 0)
      continue;

    //compute a,b and test if I intersect
    glm::vec3 a = newvertices[g_indices[i*3+2]] - newvertices[g_indices[i*3]];
    glm::vec3 b = newvertices[g_indices[i*3+1]] - newvertices[g_indices[i*3]];

    glm::vec3 res = glm::inverse(glm::mat3(a,b,-g))*(c-newvertices[g_indices[i*3]]);
    if (res.y+res.x <= 1 && res.x >= 0 && res.y >=0 && res.z >= 0){
      K = (glm::dot(glm::normalize(glm::vec3(-.3,-.3,1)),newnormals[i]) + 1.0) / 2.0; //light
      return {K,K,K};
    }
  }
  return {0,0,0};
}

void draw(sf::RenderWindow &win)
{
  win.clear(sf::Color::Black);
  std::vector<glm::vec3> new_vertices, new_normals;
  sf::RectangleShape pixelRect;
  pixelRect.setSize({2,2});
  new_vertices.reserve(4);
  new_normals.reserve(4);

  for (const glm::vec3 &v : g_vertices)
    new_vertices.push_back( glm::rotate(v,rotation,rotation_axis) );

  for (const glm::vec3 &v : g_normals)
    new_normals.push_back( glm::rotate(v,rotation, rotation_axis) );

  // the camera, which will be further back on the negative-z,
  // will be pointing towards the positive-z and tracing rays.
  // we will make the shorter dimension of length 1 and the other will scale

  //start 1 z in-front of you
  glm::vec3 ray_dir = g_cameraGaze;
  glm::vec3 x_step  = -(glm::vec3{1,0,0} / ((float)g_screenDim.x));
  glm::vec3 y_step  = -(glm::vec3{0,1,0} / ((float)g_screenDim.y));
  //now start in the top-left
  ray_dir += glm::vec3(.5,.5,0);

  for (float x=0; x<g_screenDim.x; ++x) {
    for (float y=0; y<g_screenDim.y; ++y) {
      glm::vec3 this_steps_gaze = glm::normalize(ray_dir + x*x_step + y*y_step);
      //solve system
      glm::vec3 color = intersect(g_cameraPos, this_steps_gaze, new_normals, new_vertices);
      pixelRect.setPosition({x,y});
      sf::Color col = sf::Color(color.x * 256, color.y * 256, color.z * 256);
      pixelRect.setFillColor(col);
      pixelRect.setOutlineColor(col);
      win.draw(pixelRect);
    }
  }
  win.display();
}

int main()
{
  sf::RenderWindow win(sf::VideoMode(g_screenDim.x, g_screenDim.y), "Raytracer Demo");

  sf::Clock timer;
  init();
  while( win.isOpen() )
  {
    poll(win);
    update(timer.getElapsedTime().asSeconds());
    timer.restart();
    draw(win);
  }
}
