#include <iostream>
#include <chrono>
#include <ncurses.h>
#include <random>
#include <vector>
#include <thread>

using namespace std;
static bool IS_RUNNING = true,IS_PAUSED=false;

class Animal
{
     private:

          int body_length = 3;
          WINDOW* current_window;
          int y_loc, x_loc, y_max, x_max;
          char fruit='@';
          char body='~';
          char head=')';
          vector<pair<int,int>> body_coverage; // y,x
          enum Direction
          {
               UP,
               DOWN,
               LEFT,
               RIGHT
          };
          Direction direction;
          random_device randomize;
          mt19937 gen;
          uniform_int_distribution<> distrib_y;
          uniform_int_distribution<> distrib_x;
          vector<pair<int,int>> fruit_positions;
          const int fruit_count_per_time = 5;
          bool has_all_fruit;

     public:
          Animal(WINDOW* win, int y, int x);
          void move_up();
          void move_down();
          void move_left();
          void move_right();
          int listen();
          void display();
          void handle_body();
          void randomize_fruit();
          void auto_move();
          int get_body_length();
          WINDOW* get_window();
};

Animal::Animal(WINDOW* win, int y, int x)
{
     current_window = win;
     y_loc = y;
     x_loc = x;
     body_coverage = vector<pair<int,int>>();
     fruit_positions = vector<pair<int,int>>();
     nodelay(current_window,true);
     getmaxyx(current_window, y_max, x_max);
     keypad(current_window,true);
     direction = RIGHT;
     gen.seed(randomize());
     distrib_y = uniform_int_distribution<> (1,y_max-2);
     distrib_x = uniform_int_distribution<> (1,x_max-2);
}
void Animal::auto_move()
{
    handle_body();
    switch (direction)
    {
        case UP:if (y_loc<=1)body_length--;else y_loc--; break;
        case DOWN:if (y_loc>=y_max-2)body_length--;else y_loc++;break;
        case LEFT:if (x_loc<=1)body_length--;else x_loc--;break;
        case RIGHT:if (x_loc>=x_max-2)body_length--;else x_loc++;break;
        default:break;
    }
}
int Animal::get_body_length() 
{return body_length;}

WINDOW* Animal::get_window()
{return current_window;}

void Animal::randomize_fruit()
{
     int random_y, random_x ;
     bool overlap = false;
     do
     {
          overlap = false;
          random_y = distrib_y(gen);
          random_x = distrib_x(gen);
          for (const pair<int,int>& position: body_coverage)
          {
               if (random_y==position.first&&random_x==position.second)
               {
                    overlap = true;
                    break;
               }
          }
          for (const pair<int,int>& fruit_p: fruit_positions)
          {
               if (random_y==fruit_p.first&&random_x==fruit_p.second)
               {
                    overlap = true;
                    break;
               }
          }
     } while (overlap);
     if ((int)fruit_positions.size() >= fruit_count_per_time) has_all_fruit = true;
     fruit_positions.push_back({random_y,random_x});
     wattron(current_window,COLOR_PAIR(2));
     mvwaddch(current_window,random_y,random_x,fruit);
     wattroff(current_window,COLOR_PAIR(2));
}
void Animal::handle_body()
{
     if (body_length<=0)IS_RUNNING = false;
     for (const pair<int,int>& body_part: body_coverage)
     {
          if (y_loc==body_part.first&&x_loc==body_part.second)
          {
               if (body_length>0)body_length--;
               else IS_RUNNING = false;
          }
     }
     body_coverage.push_back({y_loc,x_loc});
     for (const pair<int,int>& p:body_coverage)
     {
         wattron(current_window,COLOR_PAIR(3));
         mvwaddch(current_window,p.first,p.second,body);
         wattroff(current_window,COLOR_PAIR(3));
     }

     while ((int)body_coverage.size()>body_length)
     {
          auto tail = body_coverage.front();
          body_coverage.erase(body_coverage.begin());
          mvwaddch(current_window,tail.first,tail.second,' ');
     }
     auto it = fruit_positions.begin();
     while (it!=fruit_positions.end())
     {
          if (it -> first ==y_loc && it ->second==x_loc)
          {
            it = fruit_positions.erase(it);
            body_length++;
            has_all_fruit = false;
          }
          else ++it;
     }
     if (!has_all_fruit)
     {
          randomize_fruit();
     }
}

void Animal::move_up()
{
     if (direction == DOWN) return;
     head = 'v';
     direction = UP;
}

void Animal::move_down()
{
     if (direction == UP) return;
     head = '^';
     direction = DOWN;
}

void Animal::move_left()
{
     if (direction == RIGHT) return;
     head = '>';
     direction = LEFT;
}

void Animal::move_right()
{
     if (direction == LEFT) return;
     head = '<';
     direction = RIGHT;
}

int Animal::listen()
{
     int c = wgetch(current_window);
     switch (tolower(c))
     {
          case KEY_UP:move_up();break;
          case KEY_DOWN:move_down();break;
          case KEY_LEFT:move_left();break;
          case KEY_RIGHT:move_right();break;
          case 'p':
          case 'P': IS_PAUSED = true;
          default:break;
     }
     return c;
}

void Animal::display()
{
     mvprintw(0,5,"SnakeGame--saoli");
     mvprintw(1,3,"score: %i color: on paused: %s(END KEY to quit game)",body_length,IS_PAUSED?"true":"false");
     refresh();
     wattron(current_window,COLOR_PAIR(1));
     mvwaddch(current_window,y_loc,x_loc,head);
     wattroff(current_window,COLOR_PAIR(1));
     wrefresh(current_window);
}

int main()
{
    const double interval_s = .2;
    const auto interval = chrono::duration<double>(interval_s);
    chrono::duration<double> time_since_last_increment = chrono::duration<double>(0.0);
    auto last_time = chrono::steady_clock::now();
    auto start_pause_time = last_time;
    bool was_paused = false;
    cout << "snake game saoli start\n";
    if (!initscr()) cout<<"Could not initialize ssnake.\n";
    noecho();
    cbreak();
    curs_set(0);
     /// color
     start_color();
    if (!has_colors())
    {
          printw("Sorry, Your terminal, does not support colors");
          refresh();
          getch();
          clear();
          refresh();
    }
     init_pair(1,COLOR_RED,COLOR_RED);
     init_pair(2,COLOR_GREEN,COLOR_GREEN);
     init_pair(3,COLOR_BLUE,COLOR_BLUE);
     init_pair(4,COLOR_CYAN,COLOR_RED);
     //init_pair(5,COLOR_CYAN,COLOR_CYAN);
     ///
    int yMax, xMax;
    Animal* snake;
    WINDOW* playfield;
    getmaxyx(stdscr, yMax,xMax);
    playfield = newwin(yMax-5,xMax-4,3,2);
    refresh();
    wrefresh(playfield);
    snake = new Animal(playfield,1,1);
    while (IS_RUNNING)
    {
        auto current_time = chrono::steady_clock::now();
        auto delta_time = current_time - last_time;
        last_time = current_time;
        if (IS_PAUSED)
        {
            if (!was_paused)
            {
                start_pause_time = last_time;
                was_paused = true;
            }
            mvprintw(0,5,"SnakeGame--saoli [PAUSED]");
            mvprintw(1,3,"score: %i color: on paused: %s(END KEY to quit game)",snake->get_body_length(),IS_PAUSED?"true":"false");
            refresh();
            wrefresh(snake -> get_window());
            if (getch()==10)
            {
                IS_PAUSED=false;
                auto pause_duration = chrono::steady_clock::now() - start_pause_time;
                last_time = chrono::steady_clock::now();
            }
            continue;
        }
        box(snake -> get_window(),0,0);
        time_since_last_increment += delta_time;
        if (time_since_last_increment>=interval)
        {
            snake -> auto_move();
            time_since_last_increment -= interval;
            snake ->display();
        }
        if (snake ->listen()==KEY_END)
            IS_RUNNING = false;
    }
    delwin(playfield);
    delete snake;
    clear();
    refresh();
    for (int iy = 0; iy<yMax;iy++)
    {
          for (int ix = 0; ix < xMax; ix ++)
          {
               attron(COLOR_PAIR(1));
               mvprintw(iy,ix, " ");
               attroff(COLOR_PAIR(1));
          }
    }
    refresh();
    attron(COLOR_PAIR(4));
    attron(A_BOLD);
    mvprintw(yMax/2,xMax/3,"Game over");
    attroff(A_BOLD);
    attroff(COLOR_PAIR(4));
    refresh();
    getch();
    endwin();
    cout << "snake game saoli end\n";
    return 0;
}

