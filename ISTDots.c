/*Known bugs:
It might be necessary to click once again after playing a turn for the program to recognize the MOUSEBUTTONUP command. It is uncertain wether this problem
results from an already known problem with my computer or if it stems from the code.
Errors:
Shuffle creates new dots, does not shuffle the existing ones.

Notes:
This programs considers a 'square' to be any line that closes upon itself and will thus apply all square rules when it is done.
A sixth colour was added as colour 0 and it is used to mark dots for replacement.
The program automatially shuffles at the beggining in order to prevent an initial impossible board.
A defeat by clicking n, q or closing the window will not be announced on the screen due to personal choice, but it will still be counted towards the number of defeats.
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#define MAX(a,b)    (((a)>(b))?(a):(b))
#define SQR(a)      (a)*(a)
#define M_PI 3.14159265
#define STRING_SIZE 100       // max size for some strings
#define TABLE_SIZE 850        // main game space size
#define LEFT_BAR_SIZE 150     // left white bar size
#define WINDOW_POSX 200       // initial position of the window: x
#define WINDOW_POSY 200       // initial position of the window: y
#define SQUARE_SEPARATOR 8    // square separator in px
#define BOARD_SIZE_PER 0.7f   // board size in % wrt to table size
#define MAX_BOARD_POS 15      // maximum size of the board
#define MAX_COLORS 6
#define MARGIN 5
#define NUMBER 500

// declaration of the functions related to graphical issues
void InitEverything(int , int , TTF_Font **, SDL_Surface **, SDL_Window ** , SDL_Renderer ** );
void InitSDL();
void InitFont();
SDL_Window* CreateWindow(int , int );
SDL_Renderer* CreateRenderer(int , int , SDL_Window *);
int RenderText(int, int, const char *, TTF_Font *, SDL_Color *, SDL_Renderer *);
int RenderLogo(int, int, SDL_Surface *, SDL_Renderer *);
int RenderTable(int, int, int [], TTF_Font *, SDL_Surface **, SDL_Renderer *);
void ProcessMouseEvent(int , int , int [], int , int *, int * );
void RenderPoints(int [][MAX_BOARD_POS], int, int, int [], int, SDL_Renderer *);
void RenderStats( SDL_Renderer *, int [], int);
void filledCircleRGBA(SDL_Renderer * , int , int , int , int , int , int );


//declaration of my own functions
void parametros(int *, int *, int *, int [], char *);
void randomizer(int [][MAX_BOARD_POS], int, int, int);
void ligacao(int , int , int [], int , int , SDL_Renderer*, int , int, int);
void resultados(char *, int, int, int [], int);
SDL_Rect Goal( int );
SDL_Rect Misc(int, int);
void ProcessEndTurn(int , int, int *, int *, int [], int, int, int, int [], int [][NUMBER], int [][MAX_BOARD_POS], int *, int [][MAX_BOARD_POS], int [], int);
void ProcessTurn(SDL_Renderer *, int [], int, int [][NUMBER], int [], int [][MAX_BOARD_POS], int *, int *, int, int, int *, int, int);
void undof(int, int, int [][MAX_BOARD_POS], int [][MAX_BOARD_POS], int [], int [], int *, int *);
int sizetest(char *, int);
int isnumber(char *);
int charToint(char *);
int intsizetest(int, int, int);
int points(int);
int detectShuffle(int [][MAX_BOARD_POS], int, int);
void WinLossProcessor(SDL_Renderer *, int [], int [], int*, int*, int*, int*, int*, int*, int*, int*, int*);
void shuffletxt(SDL_Renderer*);


// definition of some strings: they cannot be changed when the program is executed !
const char myName[] = "Ricarte Ribeiro";
const char myNumber[] = "IST89998";
const char victory[] = "Venceu! Clique no 'n' para um novo jogo.";
const char defeat[] = "Perdeu! Clique no 'n' para um novo jogo.";
const char shuffletext[] = "Jogo impossivel! Executando shuffle...";
const int colors[3][MAX_COLORS] = {{0, 3, 24, 255, 250, 252},{0, 65, 242, 0, 250, 20},{0, 255, 53, 0, 19, 236}};

/**
 * main function: entry point of the program
 * only to invoke other functions !
 */
int main( void )
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font *serif = NULL;
    SDL_Surface *imgs[2];
    SDL_Event event;
    int delay = 300;
    int quit = 0;
    int width = (TABLE_SIZE + LEFT_BAR_SIZE);
    int height = TABLE_SIZE;
    int square_size_px = 0, board_size_px[2] = {0};
    int board_pos_x = 0, board_pos_y = 0;
    int board[MAX_BOARD_POS][MAX_BOARD_POS] = {{0}};
    int pt_x = 0, pt_y = 0;

    /*my variables
    -n_cor - n colours chosen
    -jogada - number of turn played in current game
    -n - number of dots chosen in the current turn
    -close - if 1 then the line has been closed
    -valid - validity of turn
    -lose - determines loss
    -shuffle - if 0 then the board will be shuffled.
    -count_l - n losses
    -count_v - n victories
    -n_g - new game
    -count_j - n turns played in game
    -count_jog - n games played
    -undo - if 0 then the player will be allowed to undo 1 turn
    -e_g - end game
    -lose_sp - triggered when the player quits or starts a new game without finishing the current one after having at least started a turn.
     Will not show the defeat text unlike a true defeat but will still increment the counter
    -losetxt - determines together with lose_sp wether or not to show the defeat text.
    -goal_i - vector that stores the initial objectives - 0 is the turns available while 1 to 5 are the number of dots of each colour.
    -goal_j - goal vector for every game
    -goal_u - goal vector for undo
    -pos_i - vector that stores the dots chosen in a turn. pos[0][i] is the x while pos[1][i] is the y.
    -pos_c - vector that stores the colours of each dot chosen.
    -vit_j - vector that stores the number of turns before a victory or a 0 in case of defeat
    -board_u - stores the previous board state in case of undo
    -pname - player's name*/

    int n_cor = 0, jogada = 0, n = 0, close = 0, valid = 0, lose = 0, shuffle=0;
    int count_l=0, count_v=0, count_j = 0, count_jog=0;
    int undo=1, lose_sp=0, losetxt = 0;
    int n_g=1, e_g=0;
    int goal_i[6]={0}, goal_j[6]={0}, goal_u[6]={0};
    int pos_i[2][NUMBER] = {{0}}, pos_c[NUMBER] = {0};
    int vit_j[NUMBER]= {0};
    int board_u[MAX_BOARD_POS][MAX_BOARD_POS]={{0}};
    char pname[STRING_SIZE];

    parametros(&board_pos_x, &board_pos_y, &n_cor, goal_i, pname);
    for(int i=0; i<=5; i++)
    {
        goal_j[i]=goal_i[i];
    }

    srand(1234);

    randomizer(board, n_cor, board_pos_x, board_pos_y);

    // initialize graphics
    InitEverything(width, height, &serif, imgs, &window, &renderer);

    while( quit == 0 )
    {
        // while there's events to handle
        while( SDL_PollEvent( &event ) )
        {
            if( event.type == SDL_QUIT )
            {
                if (n_g == 0)
                {
                    lose=1;
                    lose_sp=1;
                }
                quit=1;
            }
            else if ( event.type == SDL_KEYDOWN )
            {
                switch ( event.key.keysym.sym )
                {
                    case SDLK_n:
                    {
                        if (n_g == 0)
                        {
                            lose=1;
                            lose_sp=1;
                        }
                        else
                        {
                            lose=0;
                            n_g=1;
                        }

                        for(int i = 0; i<=5; i++)
                        {
                            goal_j[i]=goal_i[i];
                        }
                        randomizer(board, n_cor, board_pos_x, board_pos_y);
                        losetxt=0;
                        e_g=0; break;
                    }
                    case SDLK_q:
                    {
                        if (n_g == 0)
                        {
                            lose=1;
                            lose_sp=1;
                        }
                        quit=1; break;
                    }
                    case SDLK_u:
                    {
                        if (undo == 0)
                        {
                            undof(board_pos_x, board_pos_y, board, board_u, goal_j, goal_u, &undo, &count_j);
                        }
                    }
                    default:
                        break;
                }
            }

            else if ( event.type == SDL_MOUSEBUTTONDOWN )
            {
                ProcessMouseEvent(event.button.x, event.button.y, board_size_px, square_size_px, &pt_x, &pt_y);
                //printf("Button down: %d %d\n", pt_x, pt_y);

                if (e_g == 0)
                {
                    jogada=1;
                }
            }
            else if ( event.type == SDL_MOUSEBUTTONUP )
            {
                ProcessMouseEvent(event.button.x, event.button.y, board_size_px, square_size_px, &pt_x, &pt_y);
                //printf("Button up: %d %d\n", pt_x, pt_y);

                if (valid == 1)
                {
                    n_g=0;
                }

                ProcessEndTurn(valid, n_cor, &lose, &count_j, goal_j, close, board_pos_x, board_pos_y, pos_c, pos_i, board, &n, board_u, goal_u, n_g);

                if (valid == 1)
                {
                    undo=0;
                }
                valid=0;
                n=0;
                close=0;
                jogada=0;

                shuffle = detectShuffle(board, board_pos_x, board_pos_y);


            }
            else if ( event.type == SDL_MOUSEMOTION )
            {
                ProcessMouseEvent(event.button.x, event.button.y, board_size_px, square_size_px, &pt_x, &pt_y);
                //printf("Moving Mouse: %d %d\n", pt_x, pt_y);
                if (jogada == 1)
                {
                    ProcessMouseEvent(event.button.x, event.button.y, board_size_px, square_size_px, &pt_x, &pt_y);
                    ProcessTurn(renderer, board_size_px, square_size_px, pos_i, pos_c, board, &n, &close, board_pos_x, board_pos_y, &valid, pt_x, pt_y);
                }
            }
        }

        //render game table - only when a turn is not ongoing
        if(jogada == 0)
        {
            square_size_px = RenderTable( board_pos_x, board_pos_y, board_size_px, serif, imgs, renderer);
        }
        // render board

        RenderPoints(board, board_pos_x, board_pos_y, board_size_px, square_size_px, renderer);

        //renders the objectives
        RenderStats(renderer, goal_j, n_cor);

        //processes the win/loss scenarios
        WinLossProcessor(renderer, goal_j, vit_j, &count_v, &count_l, &count_j, &lose, &count_jog, &lose_sp, &e_g, &losetxt, &n_g);

        //processes shuffle scenarios
        while (shuffle != 2)
        {
            shuffle++;

            shuffletxt(renderer);

            if (shuffle != 2)
            {
                randomizer(board, n_cor, board_pos_x, board_pos_y);
            }

            shuffle = detectShuffle(board, board_pos_x, board_pos_y);
        }

        // render in the screen all changes above
        SDL_RenderPresent(renderer);

        // add a delay
        SDL_Delay( delay );

    }

    resultados(pname, count_v, count_l, vit_j, count_jog);

    // free memory allocated for images and textures and closes everything including fonts
    TTF_CloseFont(serif);
    SDL_FreeSurface(imgs[0]);
    SDL_FreeSurface(imgs[1]);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}

/**
 * ProcessMouseEvent: gets the square pos based on the click positions !
 * \param _mouse_pos_x position of the click on pixel coordinates
 * \param _mouse_pos_y position of the click on pixel coordinates
 * \param _board_size_px size of the board !
 * \param _square_size_px size of each square
 * \param _pt_x square nr
 * \param _pt_y square nr
 */
void ProcessMouseEvent(int _mouse_pos_x, int _mouse_pos_y, int _board_size_px[], int _square_size_px,
        int *_pt_x, int *_pt_y )
{
    // corner of the board
    int sq_x=0, sq_y=0, circleX = 0, circleY = 0, circleR = 0, dist = 0;
    int x_corner = (TABLE_SIZE - _board_size_px[0]) >> 1;
    int y_corner = (TABLE_SIZE - _board_size_px[1] - 15);

    // verify if valid cordinates
    if (_mouse_pos_x < x_corner || _mouse_pos_y < y_corner || _mouse_pos_x > (x_corner + _board_size_px[0])
        || _mouse_pos_y > (y_corner + _board_size_px[1]) )
    {
        *_pt_x = -1;
        *_pt_y = -1;
        return;
    }

    // computes the square where the mouse position is

    sq_x = (_mouse_pos_x - x_corner) / (_square_size_px + SQUARE_SEPARATOR);
    sq_y = (_mouse_pos_y - y_corner) / (_square_size_px + SQUARE_SEPARATOR);

    circleX = x_corner + (sq_x+1)*SQUARE_SEPARATOR + sq_x*(_square_size_px)+(_square_size_px>>1);
    circleY = y_corner + (sq_y+1)*SQUARE_SEPARATOR + sq_y*(_square_size_px)+(_square_size_px>>1);
    circleR = (int)(_square_size_px*0.4f);

    dist = (int)floor(sqrt( SQR(_mouse_pos_x - circleX) + SQR(_mouse_pos_y - circleY)));

    if (dist < circleR)
    {
        *_pt_x = sq_x;
        *_pt_y = sq_y;
    }
    else
    {
        *_pt_x = -1;
        *_pt_y = -1;
    }
}

/**
 * RenderPoints: renders the board
 * \param _board 2D array with integers representing board colors
 * \param _board_pos_x number of positions in the board (x axis)
 * \param _board_pos_y number of positions in the board (y axis)
 * \param _square_size_px size of each square
 * \param _board_size_px size of the board
 * \param _renderer renderer to handle all rendering in a window
 */
void RenderPoints(int _board[][MAX_BOARD_POS], int _board_pos_x, int _board_pos_y,
        int _board_size_px[], int _square_size_px, SDL_Renderer *_renderer)
{
    int clr, x_corner, y_corner, circleX, circleY, circleR;

    // corner of the board
    x_corner = (TABLE_SIZE - _board_size_px[0]) >> 1;
    y_corner = (TABLE_SIZE - _board_size_px[1] - 15);

    // renders the squares where the dots will appear
    for (int i = 0; i < _board_pos_x; i++ )
    {
        for (int j = 0; j < _board_pos_y; j++ )
        {
                // define the size and copy the image to display
                circleX = x_corner + (i+1)*SQUARE_SEPARATOR + i*(_square_size_px)+(_square_size_px>>1);
                circleY = y_corner + (j+1)*SQUARE_SEPARATOR + j*(_square_size_px)+(_square_size_px>>1);
                circleR = (int)(_square_size_px*0.4f);
                // draw a circle

                clr = _board[i][j];
                filledCircleRGBA(_renderer, circleX, circleY, circleR, colors[0][clr], colors[1][clr], colors[2][clr]);
        }
    }
}

/**
 * filledCircleRGBA: renders a filled circle
 * \param _circleX x pos
 * \param _circleY y pos
 * \param _circleR radius
 * \param _r red
 * \param _g gree
 * \param _b blue
 */
void filledCircleRGBA(SDL_Renderer * _renderer, int _circleX, int _circleY, int _circleR, int _r, int _g, int _b)
{
    int off_x = 0;
    int off_y = 0;
    float degree = 0.0;
    float step = M_PI / (_circleR*8);

    SDL_SetRenderDrawColor(_renderer, _r, _g, _b, 255);

    while (_circleR > 0)
    {
        for (degree = 0.0; degree < M_PI/2; degree+=step)
        {
            off_x = (int)(_circleR * cos(degree));
            off_y = (int)(_circleR * sin(degree));
            SDL_RenderDrawPoint(_renderer, _circleX+off_x, _circleY+off_y);
            SDL_RenderDrawPoint(_renderer, _circleX-off_y, _circleY+off_x);
            SDL_RenderDrawPoint(_renderer, _circleX-off_x, _circleY-off_y);
            SDL_RenderDrawPoint(_renderer, _circleX+off_y, _circleY-off_x);
        }
        _circleR--;
    }
}

/**
 * Shows some information about the game:
 * - Goals of the game
 * - Number of moves
 * \param _renderer renderer to handle all rendering in a window
 * \param _font font to display the scores
 * \param _goals goals of the game
 * \param _ncolors number of colors
 * \param _moves number of moves remaining
 */
void RenderStats( SDL_Renderer * _renderer, int _goals[], int _ncolors)
{
    //font declared inside the function for less possibility of conflict with existing functions
    TTF_Font *_font;
    char joga[10], cor1[10], cor2[2], cor3[10], cor4[10], cor5[10];

    _font = TTF_OpenFont("OpenSans.ttf", 30);
    if(!_font)
    {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    sprintf(joga, "%d", _goals[0]);
    sprintf(cor1, "%d", _goals[1]);
    sprintf(cor2, "%d", _goals[2]);
    sprintf(cor3, "%d", _goals[3]);
    sprintf(cor4, "%d", _goals[4]);
    sprintf(cor5, "%d", _goals[5]);

    SDL_Color black = { 0, 0, 0 };


    SDL_Rect jog, c1, c2, c3, c4, c5;
    jog.x=30;
    jog.y=50;
    jog.w=60;
    jog.h=60;

    c1= Goal(160);
    c2= Goal(270);
    c3= Goal(380);
    c4= Goal(490);
    c5= Goal(600);

    SDL_SetRenderDrawColor( _renderer, 205, 193, 181, 100 );

    SDL_RenderFillRect(_renderer, &jog);

    switch(_ncolors)
    {
        case(5):
        SDL_RenderFillRect(_renderer, &c5);
        case(4):
        SDL_RenderFillRect(_renderer, &c4);
        case(3):
        SDL_RenderFillRect(_renderer, &c3);
        case(2):
        SDL_RenderFillRect(_renderer, &c2);
        case(1):
        SDL_RenderFillRect(_renderer, &c1);
        break;
    }

    for (int i=1; i<=_ncolors; i++)
    {
        filledCircleRGBA(_renderer, 70+(110*i), 80, 15, colors[0][i], colors[1][i], colors[2][i]);
    }

    switch(_ncolors)
    {
        case(5):
        RenderText(650, 58, cor5, _font, &black, _renderer );
        case(4):
        RenderText(540, 58, cor4, _font, &black, _renderer );
        case(3):
        RenderText(430, 58, cor3, _font, &black, _renderer );
        case(2):
        RenderText(320, 58, cor2, _font, &black, _renderer );
        case(1):
        RenderText(43, 58, joga, _font, &black, _renderer );
        RenderText(210, 58, cor1, _font, &black, _renderer );
        break;
    }

    TTF_CloseFont(_font);
    /* To Be Done */
}

/**
 * RenderTable: Draws the table where the game will be played, namely:
 * -  some texture for the background
 * -  the right part with the IST logo and the student name and number
 * -  the grid for game board with squares and seperator lines
 * \param _board_pos_x number of positions in the board (x axis)
 * \param _board_pos_y number of positions in the board (y axis)
 * \param _board_size_px size of the board
 * \param _font font used to render the text
 * \param _img surfaces with the table background and IST logo (already loaded)
 * \param _renderer renderer to handle all rendering in a window
 */
int RenderTable( int _board_pos_x, int _board_pos_y, int _board_size_px[],
        TTF_Font *_font, SDL_Surface *_img[], SDL_Renderer* _renderer )
{
    SDL_Color black = { 0, 0, 0 }; // black
    SDL_Color light = { 205, 193, 181 };
    SDL_Color dark = { 120, 110, 102 };
    SDL_Texture *table_texture;
    SDL_Rect tableSrc, tableDest, board, board_square;
    int height, board_size, square_size_px, max_pos;

    // set color of renderer to some color
    SDL_SetRenderDrawColor( _renderer, 255, 255, 255, 255 );

    // clear the window
    SDL_RenderClear( _renderer );

    tableDest.x = tableSrc.x = 0;
    tableDest.y = tableSrc.y = 0;
    tableSrc.w = _img[0]->w;
    tableSrc.h = _img[0]->h;
    tableDest.w = TABLE_SIZE;
    tableDest.h = TABLE_SIZE;

    // draws the table texture
    table_texture = SDL_CreateTextureFromSurface(_renderer, _img[0]);
    SDL_RenderCopy(_renderer, table_texture, &tableSrc, &tableDest);

    // render the IST Logo
    height = RenderLogo(TABLE_SIZE, 0, _img[1], _renderer);

    // render the student name
    height += RenderText(TABLE_SIZE+3*MARGIN, height, myName, _font, &black, _renderer);

    // this renders the student number
    RenderText(TABLE_SIZE+3*MARGIN, height, myNumber, _font, &black, _renderer);

    // compute and adjust the size of the table and squares
    max_pos = MAX(_board_pos_x, _board_pos_y);
    board_size = (int)(BOARD_SIZE_PER*TABLE_SIZE);
    square_size_px = (board_size - (max_pos+1)*SQUARE_SEPARATOR) / max_pos;
    _board_size_px[0] = _board_pos_x*(square_size_px+SQUARE_SEPARATOR)+SQUARE_SEPARATOR;
    _board_size_px[1] = _board_pos_y*(square_size_px+SQUARE_SEPARATOR)+SQUARE_SEPARATOR;

    // renders the entire board background
    SDL_SetRenderDrawColor(_renderer, dark.r, dark.g, dark.b, dark.a );
    board.x = (TABLE_SIZE - _board_size_px[0]) >> 1;
    board.y = (TABLE_SIZE - _board_size_px[1] - 15);
    board.w = _board_size_px[0];
    board.h = _board_size_px[1];
    SDL_RenderFillRect(_renderer, &board);

    // renders the squares where the numbers will appear
    SDL_SetRenderDrawColor(_renderer, light.r, light.g, light.b, light.a );

    // iterate over all squares
    for ( int i = 0; i < _board_pos_x; i++ )
    {
        for ( int j = 0; j < _board_pos_y; j++ )
        {
            board_square.x = board.x + (i+1)*SQUARE_SEPARATOR + i*square_size_px;
            board_square.y = board.y + (j+1)*SQUARE_SEPARATOR + j*square_size_px;
            board_square.w = square_size_px;
            board_square.h = square_size_px;
            SDL_RenderFillRect(_renderer, &board_square);
        }
    }

    // destroy everything
    SDL_DestroyTexture(table_texture);
    // return for later use
    return square_size_px;
}

/**
 * RenderLogo function: Renders the IST logo on the app window
 * \param x X coordinate of the Logo
 * \param y Y coordinate of the Logo
 * \param _logoIST surface with the IST logo image to render
 * \param _renderer renderer to handle all rendering in a window
 */
int RenderLogo(int x, int y, SDL_Surface *_logoIST, SDL_Renderer* _renderer)
{
    SDL_Texture *text_IST;
    SDL_Rect boardPos;

    // space occupied by the logo
    boardPos.x = x;
    boardPos.y = y;
    boardPos.w = _logoIST->w;
    boardPos.h = _logoIST->h;

    // render it
    text_IST = SDL_CreateTextureFromSurface(_renderer, _logoIST);
    SDL_RenderCopy(_renderer, text_IST, NULL, &boardPos);

    // destroy associated texture !
    SDL_DestroyTexture(text_IST);
    return _logoIST->h;
}

/**
 * RenderText function: Renders some text on a position inside the app window
 * \param x X coordinate of the text
 * \param y Y coordinate of the text
 * \param text string with the text to be written
 * \param _font TTF font used to render the text
 * \param _color color of the text
 * \param _renderer renderer to handle all rendering in a window
 */
int RenderText(int x, int y, const char *text, TTF_Font *_font, SDL_Color *_color, SDL_Renderer* _renderer)
{
    SDL_Surface *text_surface;
    SDL_Texture *text_texture;
    SDL_Rect solidRect;


    solidRect.x = x;
    solidRect.y = y;
    // create a surface from the string text with a predefined font
    text_surface = TTF_RenderText_Blended(_font,text,*_color);
    if(!text_surface)
    {
        printf("TTF_RenderText_Blended: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    // create texture
    text_texture = SDL_CreateTextureFromSurface(_renderer, text_surface);
    // obtain size
    SDL_QueryTexture( text_texture, NULL, NULL, &solidRect.w, &solidRect.h );
    // render it !
    SDL_RenderCopy(_renderer, text_texture, NULL, &solidRect);
    // clear memory
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);
    return solidRect.h;
}

/**
 * InitEverything: Initializes the SDL2 library and all graphical components: font, window, renderer
 * \param width width in px of the window
 * \param height height in px of the window
 * \param _font font that will be used to render the text
 * \param _img surface to be created with the table background and IST logo
 * \param _window represents the window of the application
 * \param _renderer renderer to handle all rendering in a window
 */
void InitEverything(int width, int height, TTF_Font **_font, SDL_Surface *_img[], SDL_Window** _window, SDL_Renderer** _renderer)
{
    InitSDL();
    InitFont();
    *_window = CreateWindow(width, height);
    *_renderer = CreateRenderer(width, height, *_window);

    // load the table texture
    _img[0] = IMG_Load("table_texture.png");
    if (_img[0] == NULL)
    {
        printf("Unable to load image: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // load IST logo
    _img[1] = SDL_LoadBMP("ist_logo.bmp");
    if (_img[1] == NULL)
    {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    // this opens (loads) a font file and sets a size
    *_font = TTF_OpenFont("FreeSerif.ttf", 16);
    if(!*_font)
    {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}

/**
 * InitSDL: Initializes the SDL2 graphic library
 */
void InitSDL()
{
    // init SDL library
    if ( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        printf(" Failed to initialize SDL : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

/**
 * InitFont: Initializes the SDL2_ttf font library
 */
void InitFont()
{
    // Init font library
    if(TTF_Init()==-1)
    {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}

/**
 * CreateWindow: Creates a window for the application
 * \param width width in px of the window
 * \param height height in px of the window
 * \return pointer to the window created
 */
SDL_Window* CreateWindow(int width, int height)
{
    SDL_Window *window;
    // init window
    window = SDL_CreateWindow( "ISTDots", WINDOW_POSX, WINDOW_POSY, width, height, 0 );
    // check for error !
    if ( window == NULL )
    {
        printf("Failed to create window : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    return window;
}

/**
 * CreateRenderer: Creates a renderer for the application
 * \param width width in px of the window
 * \param height height in px of the window
 * \param _window represents the window for which the renderer is associated
 * \return pointer to the renderer created
 */
SDL_Renderer* CreateRenderer(int width, int height, SDL_Window *_window)
{
    SDL_Renderer *renderer;
    // init renderer
    renderer = SDL_CreateRenderer( _window, -1, 0 );

    if ( renderer == NULL )
    {
        printf("Failed to create renderer : %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // set size of renderer to the same as window
    SDL_RenderSetLogicalSize( renderer, width, height );

    return renderer;
}

//function created to request the game parametres from the user
void parametros(int *x_pos, int *y_pos, int *_n_cor, int _goal[6], char *_pname)
{
    //placeholder variables created for the purpose of transferring the parametres from the user to outside the function
    int a = 0, b = 0, c = 0, e = 0;
    /*
    l - lenght of the word supplied
    testsize - tests the size of the parametres given
    testchar - tests wether or not the parametres are numbers
    testlimit - tests wether or not parametres are within the limits
    */
    int l=9, testsize=-1, testchar=-1, testlimit=-1;
    //char used to store parametres while they're being tested
    char tester[STRING_SIZE];

    while (l>8)
    {
        printf("Por favor insira o seu nome (máx 8 caratéres): ");
        fgets(_pname, STRING_SIZE, stdin);

        l=(strlen(_pname)-1);
        if (l>8)
        {
            printf("Nome maior que o limite.\n");
        }
    }

    while ((testsize == -1 || testchar == -1 || testlimit == -1))
    {
        printf("Por favor insira o tamanho desejado para o tabuleiro de jogo (Horizontal, máx 15): ");
        fgets(tester, STRING_SIZE, stdin);
        testsize = sizetest(tester, 2);
        testchar = isnumber(tester);
        if (testsize == 0 && testchar == 0)
        {
            a=charToint(tester);
            testlimit= intsizetest(a, 15, 1);
        }

    }
    testsize=-1;
    testchar=-1;
    testlimit=-1;
    *x_pos=a;

    while ((testsize == -1 || testchar == -1 || testlimit == -1))
    {
        printf("Por favor insira o tamanho desejado para o tabuleiro de jogo (Vertical, máx 15): ");
        fgets(tester, STRING_SIZE, stdin);
        testsize = sizetest(tester, 2);
        testchar = isnumber(tester);
        if (testsize == 0 && testchar == 0)
        {
            b=charToint(tester);
            testlimit= intsizetest(b, 15, 1);
        }

    }
    *y_pos=b;
    testsize=-1;
    testchar=-1;
    testlimit=-1;

    while ((testsize == -1 || testchar == -1 || testlimit == -1))
    {
        printf("Por favor insira o número de cores desejado (máx 5): ");
        fgets(tester, STRING_SIZE, stdin);
        testsize = sizetest(tester, 1);
        testchar = isnumber(tester);
        if (testsize == 0 && testchar == 0)
        {
            c=charToint(tester);
            testlimit= intsizetest(c, 5, 1);
        }

    }
    *_n_cor=c;
    testsize=-1;
    testchar=-1;
    testlimit=-1;

    for (int i = 1; i<=c; i++)
    {
            if (i == 1)
            {
                _goal[i]=points(i);
            }
            else if (i == 2)
            {
                _goal[i]=points(i);
            }
            else if (i == 3)
            {
                _goal[i]=points(i);
            }
            else if (i == 4)
            {
                _goal[i]=points(i);
            }
            else if (i == 5)
            {
                _goal[i]=points(i);
            }

    }

    while ((testsize == -1 || testchar == -1 || testlimit == -1))
    {
        printf("Por favor insira o número de jogadas (máx 99): ");
        fgets(tester, STRING_SIZE, stdin);
        testsize = sizetest(tester, 2);
        testchar = isnumber(tester);
        if (testsize == 0 && testchar == 0)
        {
            e=charToint(tester);
            testlimit= intsizetest(e, 99, 0);
            }

    }
    _goal[0]=e;

}

//connects the dots during a turn by turning them green, when connecting or red when a line is closed. Can also be used to reset a square when the player retracts their choice
void ligacao(int _board_pos_x, int _board_pos_y, int _board_size_px[], int _pt_x, int _pt_y, SDL_Renderer* _renderer, int _r, int _g, int _b)
{
    int board_size, square_size_px, max_pos;
    SDL_Rect board, board_square;

    max_pos = MAX(_board_pos_x, _board_pos_y);
    board_size = (int)(BOARD_SIZE_PER*TABLE_SIZE);
    square_size_px = (board_size - (max_pos+1)*SQUARE_SEPARATOR) / max_pos;
    _board_size_px[0] = _board_pos_x*(square_size_px+SQUARE_SEPARATOR)+SQUARE_SEPARATOR;
    _board_size_px[1] = _board_pos_y*(square_size_px+SQUARE_SEPARATOR)+SQUARE_SEPARATOR;

    board.x = (TABLE_SIZE - _board_size_px[0]) >> 1;
    board.y = (TABLE_SIZE - _board_size_px[1] - 15);
    board.w = _board_size_px[0];
    board.h = _board_size_px[1];

    SDL_SetRenderDrawColor(_renderer, _r, _g, _b, 255 );


    board_square.x = board.x + (_pt_x+1)*SQUARE_SEPARATOR + _pt_x*square_size_px;
    board_square.y = board.y + (_pt_y+1)*SQUARE_SEPARATOR + _pt_y*square_size_px;
    board_square.w = square_size_px;
    board_square.h = square_size_px;
    SDL_RenderFillRect(_renderer, &board_square);
}

//creates the objective rectangles. created for purposes of ease of use and streamlining the code
SDL_Rect Goal(int x)
{
    SDL_Rect c;
    c.x=x;
    c.y=50;
    c.w=100;
    c.h=60;
    return c;
}

//prints the results in a text file
void resultados(char *_name,int _vit,int _der,int *_vit_j,int _game_n)
{
    FILE *fp = NULL;

    fp =fopen("resultados.txt", "w" );

    fprintf(fp, "Nome do jogador: %s\n", _name);
    fprintf(fp, "Número total de jogos: %d | Vitórias: %d | Derrotas: %d\n", _game_n, _vit, _der);

    for (int i=0; i<_game_n; i++)
    {
        fprintf(fp, "Jogo Número: %d: ", i+1);
        if (_vit_j[i] == 0)
        {
            fprintf(fp, "D\n");
        }
        else
        {
            fprintf(fp, "%d V\n", _vit_j[i]);
        }
    }

    fclose(fp);
}

//processes the end of a turn
void ProcessEndTurn(int valid, int n_cor, int *lose, int *count_j, int goal_j[6], int close, int board_pos_x, int board_pos_y, int pos_c[NUMBER], int pos_i[2][NUMBER], int board[MAX_BOARD_POS][MAX_BOARD_POS], int *n, int board_u[MAX_BOARD_POS][MAX_BOARD_POS], int goal_u[6], int n_g)
{
    /*
    -el - determines how many dots have been eliminated in a column
    -maj - determines which dot of those eliminated in a column has the highest x
    -temp - used to store values to determine maj
    -sq_open - if close=1, will search column by column for deleted dots then it will start to count through the x's until it finds another deleted dot
     upon which all those found will be marked as to be deleted too.
    -k - counts how many dots will be deleted in a column as a result of being inside a closed line (temporary number)
    -e - definitive number of dots to delete inside a column. number increased whenever sq_open turns to 0 after having become 1.
    -test, test1 and test2 are merely placeholder variables to ensure no problems while deleting dots within a closed line
    -los - same as lose
    -count - same as count_j
    -np - same as n
    -pos_sq - stores the positions of dots that will be deleted in a closed line (temporary vector)
    -pos_sqe - definitive storage of dots to be deleted in a closed line
    -s_c - checks if any dots deleted after closed line stil have the same colour as the line's
    */

    int el=0, maj=0, temp=0, sq_open=0, k=0, e=0, test=0, test1=0, test2=0, s_c=1;
    int los=0, count=*count_j, np=*n;
    int pos_sq[2][NUMBER] = {{0}};
    int pos_sqe[2][NUMBER] = {{0}};

    if (valid==1)
    {

        for ( int x=0; x<board_pos_x; x++)
        {
            for (int y=0; y<board_pos_y; y++)
            {
                board_u[x][y]=board[x][y];
            }
        }
        for (int i=1; i<6; i++)
        {
            goal_u[i]=goal_j[i];
        }

        goal_j[0]-=1;
        count++;

        //this portion is activated if the line in a turn is closed
        if (close==1)
        {
            //'deletes' (turns to 0) the outside of the closed line
            for(int i=0; i<(np-1); i++)
            {
                board[pos_i[0][i]][pos_i[1][i]]=0;
                goal_j[pos_c[0]]-=1;
            }

            //detects which dots are inside the line and 'deletes' them
            for(int x=0; x<board_pos_x; x++)
            {
                for(int y=0; y<board_pos_y; y++)
                {
                    if (board[x][y]==0 && sq_open == 1 && k!=0)
                    {
                        for(int j=0; j<k; j++)
                        {
                            pos_sqe[0][j]=pos_sq[0][j];
                            pos_sqe[1][j]=pos_sq[1][j];
                            e++;
                        }
                        sq_open=0;
                    }

                    if (sq_open==1)
                    {
                        pos_sq[0][k]=x;
                        pos_sq[1][k]=y;
                        k++;
                    }

                    if (board[x][y]==0 && sq_open == 0)
                    {
                        sq_open=1;
                    }
                }
                for (int p=0; p<e; p++)
                {
                    test1=pos_sqe[0][p];
                    test2=pos_sqe[1][p];
                    test=board[test1][test2];
                    if (test !=0)
                    {
                        goal_j[test]-=1;
                    }
                    board[pos_sqe[0][p]][pos_sqe[1][p]]=0;
                    test=0;
                    test1=0;
                    test2=0;
                }
                k=0;
                e=0;
                sq_open=0;
            }
            //'deletes' all dots of the same colour as the line
            for (int x=0; x<board_pos_x; x++)
            {
                for (int y=0; y<board_pos_y; y++)
                {
                    if (board[x][y]==pos_c[1])
                    {
                        board[x][y]=0;

                        goal_j[pos_c[0]]-=1;
                    }
                }
            }

        }
        //activated when the line is left open
        else
        {
            for(int i=0; i<np; i++)
            {
                board[pos_i[0][i]][pos_i[1][i]]=0;
                goal_j[pos_c[0]]-=1;
            }
        }

    }
    //portion of code that deals with relocating the dots above the deleted ones so that they 'fall'
    for (int y=0; y<board_pos_y; y++)
    {
        for (int x=0; x<board_pos_x; x++)
        {
            if (board[y][x]==0)
            {
                el++;
                maj=MAX(temp,x);
            }
        }
        for (int i=0; i<=maj; i++ )
        {
            board[y][maj-i]=board[y][maj-el-i];
        }
        for (int j=0; j<el; j++)
        {
            board[y][j]=0;
        }
        el=0;
        maj=0;
    }
    //actual exchange of 0 dots to new coloured dots
    if (close == 0)
    {
        for (int x=0; x<board_pos_x; x++)
        {
            for (int y=0; y<board_pos_y; y++)
            {
                if (board[x][y]==0)
                {
                    board[x][y]=((rand() % (n_cor)) + 1);
                }
            }
        }
    }
    //separated section to make sure new dots after closed line don't have the same colour.
    else
    {
        while (s_c == 1)
        {
            for (int x=0; x<board_pos_x; x++)
            {
                for (int y=0; y<board_pos_y; y++)
                {
                    if (board[x][y]==0)
                    {
                        board[x][y]=((rand() % (n_cor)) + 1);
                    }
                }
            }
            s_c=0;
            for(int x=0; x<board_pos_x; x++)
            {
                for (int y=0; y<board_pos_y; y++)
                {
                    if (board[x][y] == pos_c[0])
                    {
                        board[x][y] = 0;
                        s_c=1;
                    }
                }
            }
        }
    }
    //updates the dot number objectives
    for(int i=1; i<=n_cor; i++)
    {
        if (goal_j[i] < 0)
        {
            goal_j[i]=0;
        }
    }
    //empties the turn vectors
    for (int i=0; i<=np; i++)
    {
        pos_i[0][i]=0;
        pos_i[1][i]=0;
        pos_c[i]=0;
    }
    //lose condition detector
    if (goal_j[0]==0 && n_g != 1)
    {
        los=1;
    }
    *lose=los;
    *n=np;
    *count_j=count;

}

//processes the ongoing turn
void ProcessTurn(SDL_Renderer* _renderer, int board_size_px[], int square_size_px, int pos_i[2][NUMBER], int pos_c[NUMBER], int board[MAX_BOARD_POS][MAX_BOARD_POS], int *np, int *_close, int board_pos_x, int board_pos_y, int *_valid, int pt_x, int pt_y)
{
    /*
    cor_r - if 1, the colour of the selected dot is not the correct one and the player will have to retract his choice
    */

    int n=0;
    n=*np;
    int cor_r=0;
    int close=0;
    close=*_close;
    int valid=0;
    valid=*_valid;

    if ((pt_x != -1) && (pt_y != -1))
    {
        pos_i[0][n] = pt_x;
        pos_i[1][n] = pt_y;
        pos_c[n] = board[pt_x][pt_y];

        //detects if the player had backtracked
        if ((pos_i[0][n] == pos_i[0][n-2]) && (pos_i[1][n] == pos_i[1][n-2]) && (n != 0) && (n != 1))
        {
            if (close == 1)
            {
                ligacao(board_pos_x, board_pos_y, board_size_px, pos_i[0][n-1], pos_i[1][n-1], _renderer, 100, 255, 100);
            }
            else
            {
                ligacao(board_pos_x, board_pos_y, board_size_px, pos_i[0][n-1], pos_i[1][n-1], _renderer, 205, 193, 181);
            }
            SDL_RenderPresent(_renderer);
            pos_i[0][n-1]=0;
            pos_i[1][n-1]=0;
            pos_c[n-1]=0;
            pos_c[n] = 0;
            n= n-2;
            close=0;
            cor_r=0;

        }

        //processes each dot choice
        if ((close != 1) && (cor_r == 0))
        {
            if ((((pos_i[0][n] != pos_i[0][n-1]) || (pos_i[1][n] != pos_i[1][n-1])) && ((pos_i[0][n] == pos_i[0][n-1]) || (pos_i[1][n] == pos_i[1][n-1]))) || (n == 0))
            {
                for (int i=0; i<n; i++)
                {
                    //detects if line has been closed
                    if ((pos_i[0][n] == pos_i[0][i]) && (pos_i[1][n] == pos_i[1][i]))
                    {

                        close=1;
                    }
                }
                //detects if dot is wrong colour
                if ((pos_c[n] != pos_c[n-1]) && (n != 0))
                {
                    cor_r=1;
                }
                else if (close == 1)
                {
                    ligacao(board_pos_x, board_pos_y, board_size_px, pt_x, pt_y, _renderer, 255, 100, 100);
                }
                else
                {
                    ligacao(board_pos_x, board_pos_y, board_size_px, pt_x, pt_y, _renderer, 100, 255, 100);
                }
                n++;
                SDL_RenderPresent(_renderer);
            }
        }
        //validity detector
        if (cor_r == 0 && n>1)
        {
            valid=1;
        }
        else
        {
            valid=0;
        }
    }

    *np=n;
    *_close=close;
    *_valid=valid;
}

//processes the order to undo the turn
void undof(int board_pos_x, int board_pos_y, int board[MAX_BOARD_POS][MAX_BOARD_POS], int board_u[MAX_BOARD_POS][MAX_BOARD_POS], int goal_j[6], int goal_u[6], int *un, int *count_j)
{
    int undo =0, cj=0;
    undo=*un;
    cj=*count_j;

    if (undo == 0)
    {
        for( int i=0; i<board_pos_x; i++)
        {
            for (int j=0; j<board_pos_y; j++)
            {
                board[i][j]=board_u[i][j];
            }
        }
        for(int i=1; i<6; i++)
        {
            goal_j[i]=goal_u[i];
        }
        goal_j[0]+=1;
        cj--;
        *count_j=cj;
        undo=1;
        *un=undo;

    }

}

//tests the inserted value for lenght
int sizetest(char *_parameter, int size)
{
    int l=0;
    l=(strlen(_parameter)-1);
    if (l>size)
    {
        printf("Parametro inadequado (quantidade de carateres maior que a máxima).\n");
        return -1;
    }
    else
    {
        return 0;
    }

}

//tests whether or not the input is a number
int isnumber(char *_parameter)
{
    for (int i=0; _parameter[i+1]!='\0'; i++)
    {
        if(isdigit(_parameter[i]))
        {}
        else
        {
            printf("Parametro nao é composto somente por algarismos.\n");
            return -1;
        }
    }
    return 0;

}

//transforms the input into an int
int charToint(char *_parameter)
{
    int l=0;

    int number=0;

    l = (strlen(_parameter)-1);

    for(int i=0; i<l; i++)
    {
    number = number * 10 + ( _parameter[i] - '0' );
    }

    return number;
}

//tests wether or not the input is within the limits imposed
int intsizetest(int parameter, int up_b, int lo_b)
{
    if (parameter<lo_b || parameter>up_b)
    {
        printf("Numero fora dos limites desejados.\n");
        return -1;
    }
    else
    {
        return 0;
    }
}

//function to process the dot number inputs. created for streamlining purposes
int points(int i)
{
    int test =-1, testn=-1, testin =-1, d=0;
    char tester[100];
    char cor[5][10]={"azuis", "verdes", "vermelhos", "amarelos", "roxos"};

    while ((test == -1 || testn == -1 || testin == -1))
    {
        printf("Por favor insira o número de pontos %s (máx 99): ", cor[i-1] );
        fgets(tester, STRING_SIZE, stdin);
        test = sizetest(tester, 2);
        testn = isnumber(tester);
        if (test == 0 && testn == 0)
        {
            d=charToint(tester);
            testin= intsizetest(d, 99, 0);
        }

    }

    return d;
}

//randomizer for the board
void randomizer(int board[MAX_BOARD_POS][MAX_BOARD_POS], int n_cor, int board_pos_x, int board_pos_y)
{
    for(int i = 0 ; i < board_pos_x; i++)
    {
        for(int j = 0 ; j < board_pos_y; j++)
        {
        board[i][j]= ((rand() % (n_cor))+1);
        }
    }
}

//detects if shuffle is valid
int detectShuffle(int board[MAX_BOARD_POS][MAX_BOARD_POS], int board_pos_x, int board_pos_y)
{
    int shuffle = 0;

    for(int x=0; x<board_pos_x; x++)
    {
        for(int y=0; y<board_pos_y; y++)
        {
            if ((board[x][y]==board[x-1][y]) && (x != 0))
            {
                shuffle=2;
            }
            else if ((board[x][y]==board[x+1][y]) && (x != (board_pos_x-1)))
            {
                shuffle=2;
            }
            else if ((board[x][y]==board[x][y-1]) && (y != 0))
            {
                shuffle=2;
            }
            else if ((board[x][y]==board[x][y+1]) && (y != (board_pos_y-1)))
            {
                shuffle=2;
            }
        }
    }

    return shuffle;
}

//processes win/defeat scenarios
void WinLossProcessor(SDL_Renderer* _renderer, int goal_j[], int vit_j[], int *cv, int *cl, int *cj, int *ls, int *c_jog, int *ls_sp, int *eg, int *losetext, int *ng)
{
    int count_v = 0, count_l = 0, count_j = 0, lose = 0, count_jog = 0, lose_sp = 0, e_g = 0, losetxt = 0, n_g = 0;
    count_v=*cv;
    count_l=*cl;
    count_j=*cj;
    lose=*ls;
    count_jog=*c_jog;
    lose_sp=*ls_sp;
    e_g=*eg;
    losetxt=*losetext;
    n_g=*ng;

    SDL_Rect vict, loss;

    vict = Misc(45, 760);
    loss = Misc(45, 760);

    SDL_Color white = { 255, 255, 255 };
    SDL_SetRenderDrawColor( _renderer, 205, 193, 181, 100 );


    TTF_Font *_font;

    _font = TTF_OpenFont("OpenSans.ttf", 40);
    if(!_font)
    {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    if ((goal_j[1] == 0) && (goal_j[2] == 0) && (goal_j[3] == 0) && (goal_j[4] == 0) && (goal_j[5] == 0))
    {
        if (e_g == 0)
        {
        vit_j[count_jog]=count_j;
        count_v++;
        count_jog++;
        n_g=1;
        count_j=0;
        e_g=1;
        }

        SDL_RenderFillRect(_renderer, &vict);
        RenderText(50, 150, victory , _font, &white, _renderer );
    }
    else if (lose == 1)
    {
        if (e_g == 0 && n_g != 1)
        {
            vit_j[count_jog]=0;
            count_jog++;
            count_j=0;
            count_l++;
            n_g=1;
            lose=0;

            if (lose_sp == 0)
            {
                losetxt=1;
                e_g=1;
            }
            lose_sp=0;
        }

    }
    if ( losetxt == 1 && lose_sp == 0)
    {
        SDL_RenderFillRect(_renderer, &loss);
        RenderText(50, 150, defeat , _font, &white, _renderer );
    }

    *cv=count_v;
    *cl=count_l;
    *cj=count_j;
    *ls=lose;
    *c_jog=count_jog;
    *ls_sp=lose_sp;
    *eg=e_g;
    *losetext=losetxt;
    *ng=n_g;

    TTF_CloseFont(_font);


}

//miscellaneous rectangle creator
SDL_Rect Misc(int x, int w)
{
    SDL_Rect c;
    c.x=x;
    c.y=155;
    c.w=w;
    c.h=50;
    return c;
}

//creates the text for a shuffle situation
void shuffletxt(SDL_Renderer *_renderer)
{
    SDL_Rect shuff;

    shuff=Misc(45, 760);

    SDL_Color white = { 255, 255, 255 };
    SDL_SetRenderDrawColor( _renderer, 205, 193, 181, 100 );

    TTF_Font *_font;

    _font = TTF_OpenFont("OpenSans.ttf", 40);
    if(!_font)
    {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_RenderFillRect(_renderer, &shuff);
    RenderText(50, 150, shuffletext , _font, &white, _renderer );

    TTF_CloseFont(_font);

}

