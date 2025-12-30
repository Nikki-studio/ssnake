#include <iostream>
#include <SDL2/SDL.h>
#include <vector>
#include <algorithm>
#include <deque>
#include <cstdlib>
#include <ctime>

using namespace std;

const int GRID_SIZE   = 10;
const int GRID_WIDTH  = 100;
const int GRID_HEIGHT = 100;
const int WINDOW_SIZE = 1000;
int main (void)
{
   srand(time(nullptr));
   SDL_Init(SDL_INIT_EVERYTHING);
   auto window = SDL_CreateWindow("Snake",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WINDOW_SIZE,WINDOW_SIZE,0);
   auto renderer = SDL_CreateRenderer(window, -1,0);
   SDL_Event e;
   bool is_running = true;
   enum Direction
   {
      LEFT,
      RIGHT,
      UP,
      DOWN
   };
   int size = 10, dir  = 1; 
   SDL_Rect head {WINDOW_SIZE,WINDOW_SIZE,GRID_SIZE,GRID_SIZE};
   deque<SDL_Rect> rq;
   deque<SDL_Rect> apples;
   for (int i = 0; i < 100; i++)
   {
      SDL_Rect apple;
      do 
      {
         apple = {(rand() % GRID_WIDTH * GRID_SIZE),(rand() % GRID_WIDTH * GRID_SIZE),GRID_SIZE,GRID_SIZE};
      }
      while (apple.x == head.x && apple.y == head.y);
      apples.push_back(apple);
   }
   while (is_running)
   {
      while (SDL_PollEvent(&e))
      {
         if (e.type == SDL_QUIT)
         {
            is_running = false;
         }
         if (e.type == SDL_KEYDOWN)
         {
            switch (e.key.keysym.sym)
            {
            case SDLK_DOWN: 
            {
               if (dir != UP) dir = DOWN;
            }
            break;
            case SDLK_UP: 
            {
               if (dir != DOWN) dir = UP;
            }
            break;
            case SDLK_LEFT: 
            {
               if (dir != RIGHT) dir = LEFT;
            }
            break;
            case SDLK_RIGHT: 
            {
               if (dir != LEFT) dir = RIGHT;
            }
            break;
            
            default:
               break;
            }
         }
      }
      switch (dir)
      {
         case DOWN  :head.y  += 10;break;
         case UP    :head.y  -= 10;break;
         case LEFT  :head.x  -= 10;break;
         case RIGHT :head.x  += 10;break;
      }
      if (head.x >= 1000)head.x = 0  ;
      if (head.x <   0  )head.x = 990;
      if (head.y >= 1000)head.y = 0  ;
      if (head.y <   0  )head.y = 990;
      for_each(apples.begin(), apples.end(),[&](auto& apple)
      {
         if (head.x==apple.x &&head.y==apple.y)
         {
            size += 10;
            apple.x = -10;
            apple.y = -10;
         }
      });
      for_each(rq.begin(),rq.end(),[&](auto& segment)
      {
         if (head.x == segment.x&&head.y==segment.y)
         {
            auto it = find_if(rq.begin(),rq.end(),[&](auto& segment)
            {
               return head.x == segment.x && head.y==segment.y;
            });
            if (it != rq.end())
            {
               size_t index = distance(rq.begin(),it);
               size = (size > 10)? size - index : 10;
            }
         }
      });
      rq.push_front(head);
      while (rq.size() >= size)
      {
         rq.pop_back();
      }
      SDL_SetRenderDrawColor(renderer,0,0,0,255);
      SDL_RenderClear(renderer);

      SDL_SetRenderDrawColor(renderer,0, 255, 0,255);
      for_each(apples.begin(),apples.end(),[&](auto& apple)
      {
         SDL_RenderFillRect(renderer,&apple);
      });
      SDL_SetRenderDrawColor(renderer,165, 42, 42,255);
      for_each(rq.begin(),rq.end(),[&](auto& segment)
      {
         SDL_RenderFillRect(renderer,&segment);
      });
      SDL_SetRenderDrawColor(renderer,250,0,0,255);
      SDL_RenderFillRect(renderer,&head);
      SDL_RenderPresent(renderer);
      SDL_Delay(50);
   }
   return 0;
}