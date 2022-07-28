/* Diogo Martins Alves nº 86980 - 15/04/2017
 *  Projeto Intermédio de Programação - IST - Ano Letivo 2016/17 - 2º Semestre
 *  Jogo 1024 - Num tabuleiro n*n (n escolhido pelo jogador) são geradas inicialmente duas peças aleatórias (com o valor 2 ou 4);
 *  Sempre que o jogador faz uma jogada todas as peças se movem na direção escolhida e peças com o mesmo valor fundem-se
 *  numa peça com o dobro do valor se essas duas peças "chocarem". Se a jogada resultar numa alteração do tabuleiro, é
 *  gerada uma nova peça. O jogador ganha quando conseguir fazer uma peça com o valor correspondente à dificuldade (escolhida
 *  no inicio do jogo). Quando o programa é fechado, são adicionadas ao ficheiro stats.txt as estatísticas de todos os jogos
 *  jogados durante essa sessão.
 *  Funcionalidades avançadas implementadas:
 *  -São suportados até 25 níveis de undo
 *  -Quando o jogador sai da aplicação com um jogo não terminado, é possível voltar a jogar esse jogo quando a aplicação é
 *  iniciada novamente
 *
 *
 *
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define STRING_SIZE 100       // max size for some strings
#define TABLE_SIZE 650        // main game space size
#define LEFT_BAR_SIZE 150     // left white bar size
#define WINDOW_POSX 50      // initial position of the window: x
#define WINDOW_POSY 50       // initial position of the window: y
#define SQUARE_SEPARATOR 8    // square separator in px
#define BOARD_SIZE_PER 0.7f   // board size in % wrt to table size
#define MAX_BOARD_POS 11      // maximum size of the board
#define MIN_BOARD_POS 2         //tamanho mínimo do tabuleiro
#define MAX_LEVELS 23           //nível máximo de dificuldade
#define MIN_LEVELS 4            //nível mínimo de dificuldade
#define MAX_PLAYER_NAME 8       //tamanho máximo em número de caracteres do nome de jogador
#define MARGIN 5
#define NUMBER_OF_STATS 5       //numero de estatísticas que irão ser guardadas no ficheiro de estatísticas em cada jogo jogado
#define MAX_GAMES_PLAYED 100    //numero máximo de jogos jogados por execução da aplicação
#define MAX_UNDOS 25            //numero máximo de undos possíveis

// declaration of the functions related to graphical issues
void InitEverything(int , int , TTF_Font **, TTF_Font **, SDL_Surface **, SDL_Window ** , SDL_Renderer ** );
void InitSDL();
void InitFont();
SDL_Window* CreateWindow(int , int );
SDL_Renderer* CreateRenderer(int , int , SDL_Window *);
int RenderText(int, int, const char *, TTF_Font *, SDL_Color *, SDL_Renderer *);
int RenderLogo(int, int, SDL_Surface *, SDL_Renderer *);
void RenderTable(int, int *, int *, TTF_Font *, SDL_Surface **, SDL_Renderer *);
void RenderBoard(int [][MAX_BOARD_POS], SDL_Surface **, int, int, int, SDL_Renderer *);
void RenderStats( SDL_Renderer *, TTF_Font *, int , long int , int );
void DrawResult(int, SDL_Renderer*, TTF_Font*);
void LoadValues(SDL_Surface **);
void UnLoadValues(SDL_Surface **);

//declaração de funções relacionadas com a mecânica de jogo
void LerParametros(int*, char[STRING_SIZE], int*);
void gerarNovaPeca(int[MAX_BOARD_POS][MAX_BOARD_POS], int);
void moveLeft(int[MAX_BOARD_POS][MAX_BOARD_POS], int, long int*);
void encostarEsquerda (int[MAX_BOARD_POS][MAX_BOARD_POS], int);
void moveRight(int[MAX_BOARD_POS][MAX_BOARD_POS], int, long int*);
void encostarDireita(int[MAX_BOARD_POS][MAX_BOARD_POS], int);
void moveDown(int[MAX_BOARD_POS][MAX_BOARD_POS], int, long int*);
void encostarBaixo(int[MAX_BOARD_POS][MAX_BOARD_POS], int);
void moveUp(int[MAX_BOARD_POS][MAX_BOARD_POS], int, long int*);
void encostarCima(int[MAX_BOARD_POS][MAX_BOARD_POS], int);
int checkVictory(int[MAX_BOARD_POS][MAX_BOARD_POS], int, int);
int checkLoss(int[MAX_BOARD_POS][MAX_BOARD_POS], int);
void igualarMatrizes(int[MAX_BOARD_POS][MAX_BOARD_POS], int[MAX_BOARD_POS][MAX_BOARD_POS], int);
void clearBoard(int[MAX_BOARD_POS][MAX_BOARD_POS], int);
long int dois_elevado_a(int);
int maiorPeca(int[MAX_BOARD_POS][MAX_BOARD_POS], int);
int sao_diferentes(int[MAX_BOARD_POS][MAX_BOARD_POS], int[MAX_BOARD_POS][MAX_BOARD_POS], int);
void atualizar_matrizes(int[MAX_BOARD_POS][MAX_BOARD_POS], int[MAX_BOARD_POS][MAX_BOARD_POS],
                        int[MAX_UNDOS][MAX_BOARD_POS][MAX_BOARD_POS],int, long int[MAX_UNDOS], long int[MAX_GAMES_PLAYED], int*);
void startTime(int*, int*);
int getTime(int, int, int);
void stopTime(int*, int*);
void writeStats_array(char[MAX_GAMES_PLAYED][NUMBER_OF_STATS][STRING_SIZE], int, char[STRING_SIZE], long int, int, int, int);
void writeStats_file(char[MAX_GAMES_PLAYED][NUMBER_OF_STATS][STRING_SIZE], int, char[STRING_SIZE]);
void saveGame(int[MAX_BOARD_POS][MAX_BOARD_POS], int, char[STRING_SIZE], char[STRING_SIZE], long int, int, int);
int restoreSavedGame(char [STRING_SIZE], char [STRING_SIZE], int [MAX_BOARD_POS][MAX_BOARD_POS],
                      int* , long int* , int* , int*, int*, int*);
void undo(int[MAX_BOARD_POS][MAX_BOARD_POS], int[MAX_BOARD_POS][MAX_BOARD_POS], int[MAX_UNDOS][MAX_BOARD_POS][MAX_BOARD_POS],
          int, long int*, long int[MAX_UNDOS]);

// definition of some strings: they cannot be changed when the program is executed !
const char myName[] = "Diogo Martins Alves";
const char myNumber[] = "IST186980";

/**
 * main function: entry point of the program
 * only to invoke other functions !
 */
int main( int argc, char* args[] )
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font *serif = NULL;
    TTF_Font *sans = NULL;
    SDL_Surface *array_of_numbers[MAX_LEVELS], *imgs[2];
    SDL_Event event;
    int delay = 30;
    int quit = 0;
    int width = (TABLE_SIZE + LEFT_BAR_SIZE);
    int height = TABLE_SIZE;
    int square_size_px, board_size_px, board_pos = 5; //board_pos é o número de colunas/linhas do tabuleiro de jogo
    int board[MAX_BOARD_POS][MAX_BOARD_POS] = {{0}};
    //nomes dos ficheiros onde serão guardadas as estatísticas e jogo;
    char stats_file[STRING_SIZE] = "stats.txt";
    char saved_game_file_name[STRING_SIZE] = "saved_game.txt";
    //nome escolhido pelo jogador
    char player_name[STRING_SIZE] = {0};
    //dificuldade do jogo
    int difficulty = 5;
    //tempo em segundos de quando começou e terminou o jogo
    int tempo_inicial = 0;
    int tempo_final = 0;
    //tempo decorrido desde o inicio ao fim de um jogo
    int tempo_decorrido = 0;
    //variável que indica se existe um jogo a decorrer ou não (1 ou 0)
    int jogo_em_curso = 0;
    //resultado do jogo (1=vitória, -1=derrota, 0=não concluído)
    int resultado_do_jogo = 0;
    //log2 da maior peça presente no tabuleiro quando termina um jogo
    int maior_peca = 0;
    //matriz que contém as estatísticas de todos os jogos desde que o programa foi iniciado
    char stats[MAX_GAMES_PLAYED][NUMBER_OF_STATS][STRING_SIZE] = {{{0}}};
    //matriz que contém os tabuleiros guardados em jogadas anteriores, de modo a ser possível fazer vários níveis de undo
    int matrizes_undo[MAX_UNDOS][MAX_BOARD_POS][MAX_BOARD_POS] = {{{0}}};
    //matriz auxiliar que guarda os valores do tabuleiro antes de ser premida uma seta
    int matriz_anterior[MAX_BOARD_POS][MAX_BOARD_POS] = {{0}};
    //pontuação atual do jogo
    long int pontuacao = 0;
    //variável auxiliar que guarda o valor da pontuação antes de ser premida uma seta
    long int pontuacao_anterior = 0;
    //vetor que contém os valores das pontuações guardadas em jogadas anteriores
    long int pontuacoes_undo[MAX_UNDOS] = {0};
    //numero de undos que é possível fazer na jogada atual
    int undos_possiveis = 0;
    //numero de jogos iniciados desde que a aplicação foi iniciada
    int jogos_jogados = 0;
    //semente para a função rand()
    srand(time(NULL));

    //restora um jogo gravado se for possível ou, em alternativa, lê os parâmetros iniciais de jogo:
    if(restoreSavedGame(saved_game_file_name, player_name, board, &board_pos, &pontuacao,
                        &tempo_inicial, &jogo_em_curso, &difficulty, &jogos_jogados) == 0)
    {
        LerParametros(&board_pos, player_name, &difficulty);
    }

	// initialize graphics
	InitEverything(width, height, &serif, &sans, imgs, &window, &renderer);

    // loads numbers as images
    LoadValues(array_of_numbers);

  	while( quit == 0 )
    {

        // while there's events to handle
        while( SDL_PollEvent( &event ) )
        {
            //quando a aplicação é fechada
			if( event.type == SDL_QUIT )
            {
                //se existir um jogo a decorrer:
                if(jogo_em_curso)
                {
                    stopTime(&tempo_final, &jogo_em_curso); //para o contador de tempo
                    //valores relativos ao tempo decorrido durante o inicio do jogo e maior peça presente no tabuleiro
                    tempo_decorrido = getTime(tempo_inicial, tempo_final, jogo_em_curso);
                    maior_peca = maiorPeca(board, board_pos);
                    printf("Jogo terminado\n");
                    printf("Pontuação: %ld\n", pontuacao);
                    printf("Tempo decorrido: %d\n\n", tempo_decorrido);
                    //escreve as estatísticas no vetor de estatísticas
                    writeStats_array(stats, jogos_jogados, player_name, pontuacao, maior_peca, tempo_decorrido, resultado_do_jogo);
                    //guarda o jogo no ficheiro saved_game.txt
                    saveGame(board, board_pos, saved_game_file_name, player_name, pontuacao, tempo_decorrido, difficulty);
                }
                //escreve no ficheiro as estatísticas de todos os jogos jogados desde o inicio da execução da aplicação e sai da aplicação
                writeStats_file(stats, jogos_jogados, stats_file);
                quit = 1;
            }
			else if ( event.type == SDL_KEYDOWN )
			{
				switch ( event.key.keysym.sym )
				{
				    //quando é premida a tecla 'q', o comportamento é igual a quando é fechada a janela de jogo (código acima)
                    case SDLK_q:
                        if(jogo_em_curso)
                        {
                            stopTime(&tempo_final, &jogo_em_curso);
                            tempo_decorrido = getTime(tempo_inicial, tempo_final, jogo_em_curso);
                            maior_peca = maiorPeca(board, board_pos);
                            printf("Jogo terminado\n");
                            printf("Pontuação: %ld\n", pontuacao);
                            printf("Tempo decorrido: %d\n\n", tempo_decorrido);

                            writeStats_array(stats, jogos_jogados, player_name, pontuacao, maior_peca, tempo_decorrido, resultado_do_jogo);
                            saveGame(board, board_pos, saved_game_file_name, player_name, pontuacao, tempo_decorrido, difficulty);
                        }
                        writeStats_file(stats, jogos_jogados, stats_file);
                        quit = 1;
                        break;

                    //quando a tecla 'n' (novo jogo) é premida:
                    case SDLK_n:
                        //é terminado o jogo a decorrer atualmente se existir
                        if(jogo_em_curso)
                        {
                            stopTime(&tempo_final, &jogo_em_curso);
                            tempo_decorrido = getTime(tempo_inicial, tempo_final, jogo_em_curso);
                            maior_peca = maiorPeca(board, board_pos);
                            writeStats_array(stats, jogos_jogados, player_name, pontuacao, maior_peca, tempo_decorrido, resultado_do_jogo);
                        }
                        //é iniciado um novo jogo: pontuações e matrizes são iniciadas a zeros, a contagem de tempo começa,
                        //são geradas duas peças aleatórias no tabuleiro e o número de jogos jogados é incrementado
                        resultado_do_jogo = 0; //por defeito, o resultado do jogo no início é "não concluído"
                        startTime(&tempo_inicial, &jogo_em_curso);
                        jogos_jogados++;
                        undos_possiveis = 0;
                        pontuacao = 0;
                        pontuacoes_undo[0] = 0;
                        clearBoard(board, board_pos);
                        gerarNovaPeca(board, board_pos);
                        gerarNovaPeca(board, board_pos);
                        igualarMatrizes(matriz_anterior, board, board_pos);
                        igualarMatrizes(matrizes_undo[0], board, board_pos);
                        break;

                    //no caso de ser premida a tecla 'u', o tabuleiro volta ao nível de undo mais recente e o numero de
                    //undos possíveis é decrementado
                    case SDLK_u:
                        if(jogo_em_curso && undos_possiveis > 0)
                        {
                            undo(board, matriz_anterior, matrizes_undo, board_pos, &pontuacao, pontuacoes_undo);
                            undos_possiveis--;
                        }
                        break;

                    //quando é premida uma das setas, a lógica é semelhante entre elas: as variáveis auxiliares pontuacao_anterior
                    //e matriz_anterior tomam os valores presentes do jogo, as peças são movidas e fundidas na direção
                    //escolhida e os valores das matrizes undo e pontuações undo são atualizados
					case SDLK_UP:
					    if(jogo_em_curso)
                        {
                            pontuacao_anterior = pontuacao;
                            igualarMatrizes(matriz_anterior, board, board_pos);
                            moveUp(board, board_pos, &pontuacao);
                            atualizar_matrizes(board, matriz_anterior, matrizes_undo, board_pos, pontuacoes_undo, &pontuacao_anterior, &undos_possiveis);
                        }
						break;

					case SDLK_DOWN:
					    if(jogo_em_curso)
                        {
                            pontuacao_anterior = pontuacao;
                            igualarMatrizes(matriz_anterior, board, board_pos);
                            moveDown(board, board_pos, &pontuacao);
                            atualizar_matrizes(board, matriz_anterior, matrizes_undo, board_pos, pontuacoes_undo, &pontuacao_anterior, &undos_possiveis);
                        }
                        break;

				    case SDLK_LEFT:
				        if(jogo_em_curso)
                        {
                            pontuacao_anterior = pontuacao;
                            igualarMatrizes(matriz_anterior, board, board_pos);
                            moveLeft(board, board_pos, &pontuacao);
                            atualizar_matrizes(board, matriz_anterior, matrizes_undo, board_pos, pontuacoes_undo, &pontuacao_anterior, &undos_possiveis);
                        }
                        break;

					case SDLK_RIGHT:
					    if(jogo_em_curso)
                        {
                            pontuacao_anterior = pontuacao;
                            igualarMatrizes(matriz_anterior, board, board_pos);
                            moveRight(board, board_pos, &pontuacao);
                            atualizar_matrizes(board, matriz_anterior, matrizes_undo, board_pos, pontuacoes_undo, &pontuacao_anterior, &undos_possiveis);
                        }
                        break;

					default:
						break;
				}
			}
        }

        //quando existe um jogo em curso e se dá a vitória/derrota, são obtidas as variáveis necessárias
        //a escrever no vetor de estatísticas e é alterado o resultado_do_jogo
        if(checkVictory(board, board_pos, difficulty) && jogo_em_curso)
        {
            stopTime(&tempo_final, &jogo_em_curso);
            resultado_do_jogo = 1; //vitória
            tempo_decorrido = getTime(tempo_inicial, tempo_final, jogo_em_curso);
            maior_peca = maiorPeca(board, board_pos);
            printf("Vitória!\n");
            printf("Pontuação: %ld\n", pontuacao);
            printf("Tempo decorrido: %d\n\n", tempo_decorrido);
            writeStats_array(stats, jogos_jogados, player_name, pontuacao, maior_peca, tempo_decorrido, resultado_do_jogo);

        }
        else if(checkLoss(board, board_pos) && jogo_em_curso)
        {
            stopTime(&tempo_final, &jogo_em_curso);
            resultado_do_jogo = -1; //derrota
            tempo_decorrido = getTime(tempo_inicial, tempo_final, jogo_em_curso);
            maior_peca = maiorPeca(board, board_pos);
            printf("Derrota!\n");
            printf("Pontuação: %ld\n", pontuacao);
            printf("Tempo decorrido: %d\n\n", tempo_decorrido);

            writeStats_array(stats, jogos_jogados, player_name, pontuacao, maior_peca, tempo_decorrido, resultado_do_jogo);

        }

        //no caso de o jogador atingir o número máximo de jogos possíveis, a aplicação é fechada
        if(jogos_jogados == MAX_GAMES_PLAYED)
        {
            printf("Atingiu o número máximo de jogos nesta sessão, volte a executar a aplicação se quiser jogar mais");
            quit = 1;
        }


        // render game table
        RenderTable( board_pos, &board_size_px, &square_size_px, serif, imgs, renderer);
        // render board
        RenderBoard(board, array_of_numbers, board_pos, board_size_px, square_size_px, renderer);

        //desenha a dificuldade, pontuação e tempo decorrido em cima do tabuleiro
        RenderStats(renderer, sans, difficulty, pontuacao, getTime(tempo_inicial, tempo_final, jogo_em_curso));

        //desenha a barra de derrota ou vitória
        if(resultado_do_jogo != 0)
            DrawResult(resultado_do_jogo, renderer, sans);

        // render in the screen all changes above
        SDL_RenderPresent(renderer);
    	// add a delay
		SDL_Delay( delay );

    }

    // free memory allocated for images and textures and closes everything including fonts
    UnLoadValues(array_of_numbers);
    TTF_CloseFont(serif);
    TTF_CloseFont(sans);
    SDL_FreeSurface(imgs[0]);
    SDL_FreeSurface(imgs[1]);
    SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;
}


/**
 * LoadCards: Loads all images with the values
 * \param _array_of_numbers vector with all loaded numbers
 */
void LoadValues(SDL_Surface **_array_of_numbers)
{
    int i;
    char filename[STRING_SIZE];

     // loads all images to an array
    for ( i = 0 ; i < MAX_LEVELS; i++ )
    {
        // create the filename !
        sprintf(filename, ".//numbers//value_%02d.jpg", i+1);
        // loads the image !
        _array_of_numbers[i] = IMG_Load(filename);
        // check for errors: deleted files ?
        if (_array_of_numbers[i] == NULL)
        {
            printf("Unable to load image: %s\n", SDL_GetError());
            exit(EXIT_FAILURE);
        }
    }
}


/**
 * UnLoadCards: unloads all values images of the memory
 * \param _array_of_numbers vector with all loaded image numbers
 */
void UnLoadValues(SDL_Surface **_array_of_numbers)
{
    // unload all cards of the memory: +1 for the card back
    for ( int i = 0 ; i < MAX_LEVELS; i++ )
    {
        SDL_FreeSurface(_array_of_numbers[i]);
    }
}

/**
* RenderBoard: renders the board
* \param _board multidimensional array with the board numbers
* \param _array_of_numbers vector with all loaded images
* \param _board_pos number of positions in the board
* \param _board_size_px size of the board in pixels
* \param _square_size_px size of each square
* \param _renderer renderer to handle all rendering in a window
*/
void RenderBoard(int _board[][MAX_BOARD_POS], SDL_Surface **_array_of_numbers, int _board_pos,
        int _board_size_px, int _square_size_px, SDL_Renderer *_renderer )
{

    int x_corner, y_corner;
    SDL_Rect boardPos;
    SDL_Texture *number_text;

    // corner of the board
    x_corner = (TABLE_SIZE - _board_size_px) >> 1;
    y_corner = (TABLE_SIZE - _board_size_px - 15);

    // renders the squares where the numbers will appear
    for ( int i = 0; i < _board_pos; i++ )
    {
        for ( int j = 0; j < _board_pos; j++ )
        {
            // basic check
            if ( _board[j][i] > 23 || _board[j][i] < 0 )
            {
                printf("Invalid board value: RenderBoard\n");
                exit(EXIT_FAILURE);
            }
            // only draws a number if it corresponds to a non-empty space
            if ( _board[j][i] != 0 )
            {
                // define the size and copy the image to display
                boardPos.x = x_corner + (i+1)*SQUARE_SEPARATOR + i*_square_size_px;
                boardPos.y = y_corner + (j+1)*SQUARE_SEPARATOR + j*_square_size_px;
                boardPos.w = _square_size_px;
                boardPos.h = _square_size_px;
                number_text = SDL_CreateTextureFromSurface( _renderer, _array_of_numbers[ _board[j][i] - 1] );
                SDL_RenderCopy( _renderer, number_text, NULL, &boardPos );
                // destroy texture
                SDL_DestroyTexture(number_text);
            }
        }
    }
}

//desenha no ecrã uma mensagem de vitória ou derrota
void DrawResult(int _resultado_do_jogo, SDL_Renderer *_renderer, TTF_Font *_font)
{
    //cores das mensagens
    SDL_Color victory_color = { 0, 255, 0 }; //verde
    SDL_Color loss_color = { 255, 0, 0 }; //vermelho
    //atributos do retângulo que aparece por trás da mensagem
    SDL_Rect retangulo;
    retangulo.x = 0;
    retangulo.y = TABLE_SIZE/2;
    retangulo.h = 55;
    retangulo.w = TABLE_SIZE;
    SDL_SetRenderDrawColor(_renderer, 255, 255, 255 ,255); //branco
    SDL_RenderFillRect(_renderer, &retangulo);
    //render das mensagens de vitória/derrota
    if(_resultado_do_jogo == 1)
        RenderText(TABLE_SIZE/4 - MARGIN, retangulo.y, "Ganhou o jogo!!!", _font, &victory_color, _renderer);
    else if(_resultado_do_jogo == -1)
        RenderText(TABLE_SIZE/4 - MARGIN, retangulo.y, "Perdeu o jogo!", _font, &loss_color, _renderer);
}

/*
* Shows some information about the game:
* - Level to achieve (difficulty)
* - Score of the game
* - Time spent in seconds
* \param _renderer renderer to handle all rendering in a window
* \param _level current level
* \param _score actual score
* \param _time time to display in seconds
*/
void RenderStats( SDL_Renderer *_renderer, TTF_Font *_font, int _level, long int _score, int _time )
{
    char str_aux[STRING_SIZE] = {0};
    //atributos do retângulo por trás das estatísticas
    SDL_Color stats_color = { 0, 0, 0 }; //preto
    SDL_Rect retangulo;
    retangulo.x = 0;
    retangulo.y = TABLE_SIZE/10;
    retangulo.h = 55;
    retangulo.w = TABLE_SIZE;
    SDL_SetRenderDrawColor(_renderer, 255, 255, 255 ,255); //branco
    SDL_RenderFillRect(_renderer, &retangulo);
    //conversão das estatísticas para strings e render das mesmas no ecrã
    //dificuldade
    sprintf(str_aux, "%ld", dois_elevado_a(_level));
    RenderText(TABLE_SIZE/4 - 5*MARGIN, retangulo.y, str_aux, _font, &stats_color, _renderer);
    //pontuação
    sprintf(str_aux, "%ld", _score);
    RenderText(TABLE_SIZE/2 - 3*MARGIN, retangulo.y, str_aux, _font, &stats_color, _renderer);
    //tempo decorrido
    sprintf(str_aux, "%d", _time);
    RenderText(3*TABLE_SIZE/4 - 3*MARGIN, retangulo.y, str_aux, _font, &stats_color, _renderer);

}

/*
 * RenderTable: Draws the table where the game will be played, namely:
 * -  some texture for the background
 * -  the right part with the IST logo and the student name and number
 * -  the grid for game board with squares and seperator lines
 * \param _board_pos number of squares in the board
 * \param _font font used to render the text
 * \param _img surfaces with the table background and IST logo (already loaded)
 * \param _renderer renderer to handle all rendering in a window
 */
void RenderTable( int _board_pos, int *_board_size_px, int *_square_size_px, TTF_Font *_font, SDL_Surface *_img[], SDL_Renderer* _renderer )
{
    SDL_Color black = { 0, 0, 0 }; // black
    SDL_Color light = { 205, 193, 181 };
    SDL_Color dark = { 120, 110, 102 };
    SDL_Texture *table_texture;
    SDL_Rect tableSrc, tableDest, board, board_square;
    int height, board_size_px, square_size_px;

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
    height += RenderText(TABLE_SIZE+3*MARGIN, height, myNumber, _font, &black, _renderer);

    //informa o jogador das teclas para novo jogo, sair da aplicação e undo
    height += RenderText(TABLE_SIZE+3*MARGIN, height, "[n] - Novo Jogo", _font, &black, _renderer);
    height += RenderText(TABLE_SIZE+3*MARGIN, height, "[q] - Sair", _font, &black, _renderer);
    RenderText(TABLE_SIZE+3*MARGIN, height, "[u] - Undo", _font, &black, _renderer);
    // compute and adjust the size of the table and squares
    board_size_px = (int)(BOARD_SIZE_PER*TABLE_SIZE);
    square_size_px = (board_size_px - (_board_pos+1)*SQUARE_SEPARATOR) / _board_pos;
    board_size_px -= board_size_px % (_board_pos*(square_size_px+SQUARE_SEPARATOR));
    board_size_px += SQUARE_SEPARATOR;

    // renders the entire board background
    SDL_SetRenderDrawColor(_renderer, dark.r, dark.g, dark.b, dark.a );
    board.x = (TABLE_SIZE - board_size_px) >> 1;
    board.y = (TABLE_SIZE - board_size_px - 15);
    board.w = board_size_px;
    board.h = board_size_px;
    SDL_RenderFillRect(_renderer, &board);

    // renders the squares where the numbers will appear
    SDL_SetRenderDrawColor(_renderer, light.r, light.g, light.b, light.a );

    // iterate over all squares
    for ( int i = 0; i < _board_pos; i++ )
    {
        for ( int j = 0; j < _board_pos; j++ )
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
    // store some variables for later use
    *_board_size_px = board_size_px;
    *_square_size_px = square_size_px;
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
void InitEverything(int width, int height, TTF_Font **_font1, TTF_Font **_font2, SDL_Surface *_img[], SDL_Window** _window, SDL_Renderer** _renderer)
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
    *_font1 = TTF_OpenFont("FreeSerif.ttf", 16);
    if(!*_font1)
    {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    //inicialização da fonte OpenSans.ttf
    *_font2 = TTF_OpenFont("OpenSans.ttf", 40);
    if(!*_font2)
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
	window = SDL_CreateWindow( "1024", WINDOW_POSX, WINDOW_POSY, width, height, 0 );
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

//lê os parâmetros de inicio de jogo(tamanho do tabuleiro, nome de jogador e dificuldade do jogo) e garante que esses
//valores são válidos
void LerParametros(int* _board_pos, char _player_name[STRING_SIZE], int* _difficulty)
{
    //variável que guarda temporariamente o valor inserido no teclado
    char buffer[STRING_SIZE] = {0};
    //retorno da função sscanf
    int ret = 0;
    //tamanho do tabuleiro:
    do
    {
        printf("Insira o tamanho do tabuleiro (%d a %d): ", MIN_BOARD_POS, MAX_BOARD_POS);
        fgets(buffer, STRING_SIZE, stdin);
        ret = sscanf(buffer, "%d", _board_pos); //converte para int e guarda na variável correta
        if(ret != 1 || *_board_pos < MIN_BOARD_POS || *_board_pos > MAX_BOARD_POS)
        {
            printf("Tamanho inválido\n");
            ret = 0;
        }

    }while(ret != 1);

    printf("Tamanho do tabuleiro: %dx%d\n", *_board_pos, *_board_pos);
    //nome do jogador:
    do
    {
        printf("Insira o seu nome de jogador (com %d caracteres no máximo): ", MAX_PLAYER_NAME);
        fgets(buffer, STRING_SIZE, stdin);
        ret = sscanf(buffer, "%s", _player_name); //guarda na variável correta
        if(ret != 1 || strlen(_player_name) > MAX_PLAYER_NAME || strlen(_player_name) < 1)
        {
            printf("Nome inválido\n");
            ret = 0;
        }

    }while(ret != 1);

    printf("Nome de jogador: %s\n", _player_name);
    //dificuldade do jogo:
    do
    {
        printf("Insira a dificuldade do jogo (%d a %d): ", MIN_LEVELS, MAX_LEVELS);
        fgets(buffer, STRING_SIZE, stdin);
        ret = sscanf(buffer, "%d", _difficulty); //converte para int e guarda na variável correta
        if(ret != 1 || *_difficulty < MIN_LEVELS || *_difficulty > MAX_LEVELS)
        {
            printf("Dificuldade inválida\n");
            ret = 0;
        }

    }while(ret != 1);

    printf("Dificuldade: %d (O jogo termina quando fizer uma peça com o valor %ld)\n\n", *_difficulty, dois_elevado_a(*_difficulty));
}

//verifica que o jogador ganhou o jogo(isto acontece quando uma das peças do tabuleiro iguala a dificuldade)
int checkVictory(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos, int _difficulty)
{
    int i = 0;
    int j = 0;
    for(i = 0; i < _board_pos; i++)
    {
        for(j = 0; j < _board_pos; j++)
        {
            if(_board[i][j] == _difficulty)
                return 1;
        }
    }
    return 0;
}

//verifica se um jogador perdeu o jogo (isto acontece quando nenhum lugar está vazio e não existem peças adjacentes iguais)
int checkLoss(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos)
{
    int i = 0;
    int j = 0;

    //verifica se existem espaços vazios no tabuleiro
    for(i = 0; i < _board_pos; i++)
    {
        for(j = 0; j < _board_pos; j++)
        {
            if(_board[i][j] == 0)
                return 0;
        }
    }

    //verifica se existem peças adjacentes iguais
    for(i = 0; i < _board_pos - 1; i++)
    {   //verifica as peças da borda de baixo e da direita
        if(_board[i][_board_pos - 1] == _board[i + 1][_board_pos - 1] || _board[_board_pos - 1][i] == _board[_board_pos - 1][i + 1])
            return 0;
        //verifica as restantes
        for(j = 0; j < _board_pos - 1; j++)
        {
            if(_board[i][j] == _board[i][j + 1] || _board[i][j] == _board[i + 1][j])
                return 0;
        }
    }

    return 1;
}

//gera uma nova peça com o valor aleatório 2 ou 4 num local vazio aleatório do tabuleiro
void gerarNovaPeca(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos)
{
    //log2 da peça a ser gerada e posição da mesma
    int valor_peca = 1;
    int posx = 0;
    int posy = 0;
    //log2 da peça tem que ter o valor 1 ou 2
    valor_peca = ((rand()%2) + 1);
    //tenta colocar a peça até encontrar um lugar vazio
    while(1)
        {
        posx = rand()%_board_pos;
        posy = rand()%_board_pos;

        if(_board[posx][posy] == 0)
        {
            _board[posx][posy] = valor_peca;
            break;
        }

    }
}


//move e funde todas as peças na direção direita-esquerda e atualiza a pontuação
void moveLeft(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos, long int* _pontuacao)
{
    int i = 0;
    int j = 0;
    //começa por encostar todas as peças à esquerda
    encostarEsquerda(_board, _board_pos);
    //se existirem peças adjacentes iguais, funde-as e atualiza a pontuação
    for(i = 0; i < _board_pos; i++)
    {
        for(j = 0; j < _board_pos - 1; j++)
        {
            if( _board[i][j] == _board[i][j + 1] && _board[i][j] != 0)
            {
                _board[i][j] += 1;
                _board[i][j + 1] = 0;
                *_pontuacao +=  dois_elevado_a(_board[i][j]);
            }
        }
    }
    //volta a encostar todas as peças à esquerda
    encostarEsquerda(_board, _board_pos);
}

//encosta todas as peças à esquerda do tabuleiro, movendo os espaços vazios para a direita
void encostarEsquerda(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos)
{
    int i = 0;
    int j = 0;
    int k = 0;
    //contagem do numero de espaços vazios
    int countVazios = 0;

    for(i = 0; i < _board_pos; i++)
    {
        countVazios = 0;

        for(j = 0; j < _board_pos; j++)
        {
            countVazios = 0;

            //ciclo que corre enquanto um espaço do tabuleiro está vazio
            while(_board[i][j] == 0)
            {
                countVazios++;

                for(k = j; k < _board_pos - 1; k++)
                {
                    _board[i][k] = _board[i][k + 1];
                    _board[i][k + 1] = 0;
                }
                //para não deixar o ciclo a correr eternamente, sempre que o numero de espaços vazios numa linha
                //atinge o numero total de espaços da linha, o ciclo é interrompido
                if(countVazios == _board_pos)
                {
                    break;
                }

            }
        }
    }
}

//move e funde todas as peças na direção esquerda-direita e atualiza a pontuação
void moveRight(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos, long int* _pontuacao)
{
    int i = 0;
    int j = 0;
    //começa por encostar todas as peças à direita
    encostarDireita(_board, _board_pos);
    //se existirem peças adjacentes iguais, funde-as e atualiza a pontuação
    for(i = 0; i < _board_pos; i++)
    {
        for(j = _board_pos - 1; j > 0; j--)
        {
            if( _board[i][j] == _board[i][j - 1] && _board[i][j] != 0)
            {
                _board[i][j] += 1;
                _board[i][j - 1] = 0;
                *_pontuacao += dois_elevado_a(_board[i][j]);
            }
        }
    }
    //volta a encostar todas as peças à direita
    encostarDireita(_board, _board_pos);
}

//encosta todas as peças à direita do tabuleiro, movendo os espaços vazios para a esquerda
void encostarDireita(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos)
{
    int i = 0;
    int j = 0;
    int k = 0;
    //contagem do numero de espaços vazios
    int countVazios = 0;

    for(i = 0; i < _board_pos; i++)
    {
        countVazios = 0;

        for(j = _board_pos - 1; j >= 0; j--)
        {
            countVazios = 0;
            //ciclo que corre enquanto um espaço do tabuleiro está vazio
            while(_board[i][j] == 0)
            {
                countVazios++;

                for(k = j; k > 0; k--)
                {
                    _board[i][k] = _board[i][k - 1];
                    _board[i][k - 1] = 0;
                }
                //para não deixar o ciclo a correr eternamente, sempre que o numero de espaços vazios numa linha
                //atinge o numero total de espaços da linha, o ciclo é interrompido
                if(countVazios == _board_pos)
                {
                    break;
                }

            }
        }
    }
}

//move e funde todas as peças na direção cima-baixo e atualiza a pontuação
void moveDown(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos, long int* _pontuacao)
{
    int i = 0;
    int j = 0;
    //começa por encostar todas as peças em baixo
    encostarBaixo(_board, _board_pos);
    //se existirem peças adjacentes iguais, funde-as e atualiza a pontuação
    for(i = 0; i < _board_pos; i++)
    {
        for(j = _board_pos - 1; j > 0; j--)
        {
            if( _board[j][i] == _board[j - 1][i] && _board[j][i] != 0)
            {
                _board[j][i] += 1;
                _board[j - 1][i] = 0;
                *_pontuacao += dois_elevado_a(_board[j][i]);
            }
        }
    }
    //volta a encostar todas as peças abaixo
    encostarBaixo(_board, _board_pos);
}

//encosta todas as peças abaixo no tabuleiro, movendo os espaços vazios para cima
void encostarBaixo(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos)
{
    int i = 0;
    int j = 0;
    int k = 0;
    //contagem do numero de espaços vazios
    int countVazios = 0;

    for(i = 0; i < _board_pos; i++)
    {
        countVazios = 0;

        for(j = _board_pos - 1; j >= 0; j--)
        {
            countVazios = 0;
            //ciclo que corre enquanto um espaço do tabuleiro está vazio
            while(_board[j][i] == 0)
            {
                countVazios++;

                for(k = j; k > 0; k--)
                {
                    _board[k][i] = _board[k - 1][i];
                    _board[k - 1][i] = 0;
                }
                //para não deixar o ciclo a correr eternamente, sempre que o numero de espaços vazios numa coluna
                //atinge o numero total de espaços da linha, o ciclo é interrompido
                if(countVazios == _board_pos)
                {
                    break;
                }

            }
        }
    }
}

//move e funde todas as peças na direção baixo-cima e atualiza a pontuação
void moveUp(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos, long int* _pontuacao)
{
    int i = 0;
    int j = 0;
    //começa por encostar todas as peças em cima
    encostarCima(_board, _board_pos);
    //se existirem peças adjacentes iguais, funde-as e atualiza a pontuação
    for(i = 0; i < _board_pos; i++)
    {
        for(j = 0; j < _board_pos - 1; j++)
        {
            if( _board[j][i] == _board[j + 1][i] && _board[j][i] != 0)
            {
                _board[j][i] += 1;
                _board[j + 1][i] = 0;
                *_pontuacao += dois_elevado_a(_board[j][i]);
            }
        }
    }
    //volta a encostar todas as peças em cima
    encostarCima(_board, _board_pos);
}

//encosta todas as peças em cima no tabuleiro, movendo os espaços vazios para baixo
void encostarCima(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos)
{
    int i = 0;
    int j = 0;
    int k = 0;
    //contagem do numero de espaços vazios
    int countVazios = 0;

    for(i = 0; i < _board_pos; i++)
    {
        countVazios = 0;

        for(j = 0; j < _board_pos; j++)
        {
            countVazios = 0;
            //ciclo que corre enquanto um espaço do tabuleiro está vazio
            while(_board[j][i] == 0)
            {
                countVazios++;

                for(k = j; k < _board_pos - 1; k++)
                {
                    _board[k][i] = _board[k + 1][i];
                    _board[k + 1][i] = 0;
                }
                //para não deixar o ciclo a correr eternamente, sempre que o numero de espaços vazios numa coluna
                //atinge o numero total de espaços da linha, o ciclo é interrompido
                if(countVazios == _board_pos)
                {
                    break;
                }

            }
        }
    }
}

//iguala a _matriz1 à _matriz2
void igualarMatrizes(int _matriz1[MAX_BOARD_POS][MAX_BOARD_POS], int _matriz2[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos)
{
    int i = 0;
    int j = 0;

     for(i = 0; i < _board_pos; i++)
            {
                for(j = 0; j < _board_pos; j++)
                {
                    _matriz1[i][j] = _matriz2[i][j];
                }
            }
}

//impõe todos os elementos de uma matriz a 0
void clearBoard(int _matriz[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos)
{
    int i = 0;
    int j = 0;

    for(i = 0; i < _board_pos; i++)
            {
                for(j = 0; j < _board_pos; j++)
                {
                    _matriz[i][j] = 0;
                }
            }

}

//retorna um long int com o valor 2^_n
long int dois_elevado_a(int _n)
{
    long int resultado = 1;
    int i = 0;
    for(i = 0; i < _n; i++)
    {
        resultado *= 2;
    }
    return resultado;
}

//retorna 1 se as matrizes _matriz1 e _matriz2 forem diferentes e 0 se forem iguais
int sao_diferentes(int _matriz1[MAX_BOARD_POS][MAX_BOARD_POS], int _matriz2[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos)
{
    int i = 0;
    int j = 0;

    for(i = 0; i < _board_pos; i++)
    {
        for(j = 0; j < _board_pos; j++)
        {
            if(_matriz1[i][j] != _matriz2[i][j])
            {
                return 1;
            }
        }
    }
    return 0;
}

//verifica se as matrizes_undo e pontuacoes_undo devem ser atualizadas e, caso devam, atualiza-as
void atualizar_matrizes(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _matriz_anterior[MAX_BOARD_POS][MAX_BOARD_POS],
                        int _matrizes_undo[MAX_UNDOS][MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos, long int _pontuacoes_undo[MAX_UNDOS],
                        long int* _pontuacao_anterior, int* _undos_possiveis)
{
    int i = 0;
    //se as matrizes _board e _matriz anterior forem diferentes, então a jogada levou a alterações no tabuleiro,
    //logo as matrizes_undo e pontuacoes_undo serão atualizadas
    if(sao_diferentes(_board, _matriz_anterior, _board_pos))
        {
            //IMPORTANTE: as matrizes_undo e pontuacoes_undo funcionam com um sistema de "pilha", isto é, sempre que o
            //utilizador faz undo, a matriz é retirada da posição [0] e todas as outras recuam uma posição. Quando é adicionado
            //um nível de undo, a nova matriz é adicionada na posição [0] e todas as outras avançam uma posição.
            //Análogo para as pontuações
            for(i = MAX_UNDOS - 1; i > 0; i--)
            {
                igualarMatrizes(_matrizes_undo[i], _matrizes_undo[i-1], _board_pos);
                _pontuacoes_undo[i] = _pontuacoes_undo[i-1];
            }
            igualarMatrizes(_matrizes_undo[0], _matriz_anterior, _board_pos);
            _pontuacoes_undo[0] = *_pontuacao_anterior;
            (*_undos_possiveis)++;
            gerarNovaPeca(_board, _board_pos);
        }
}

//impõe o tempo de inicio de jogo e altera a variável jogo_em_curso para 1
void startTime(int* _tempo_inicial, int* _jogo_em_curso)
{
    *_tempo_inicial = time(NULL);
    *_jogo_em_curso = 1;

}

//retorna o tempo que passou desde o inicio do jogo caso o jogo esteja a decorrer ou a diferença entre o tempo_final e
//tempo_inicial caso não esteja a decorrer um jogo
int getTime(int _tempo_inicial, int _tempo_final, int _jogo_em_curso)
{
    if(_jogo_em_curso)
        return time(NULL) - _tempo_inicial;
    else
        return _tempo_final - _tempo_inicial;
}

//impõe o tempo de fim de jogo e altera a variável jogo_em_curso para 0
void stopTime(int* _tempo_final, int* _jogo_em_curso)
{
    *_jogo_em_curso = 0;
    *_tempo_final = time(NULL);
}

//retorna log2 da maior peça presente no tabuleiro quando a função é chamada
int maiorPeca(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos)
{
    int i = 0;
    int j = 0;

    int maior_peca = 0;

    for(i = 0; i < _board_pos; i++)
    {
        for(j = 0; j < _board_pos; j++)
        {
            if(_board[i][j] > maior_peca)
                maior_peca = _board[i][j];
        }
    }
    return maior_peca;
}

//escreve no vetor stats as estatísticas de um jogo quando este é terminado
void writeStats_array(char _stats[MAX_GAMES_PLAYED][NUMBER_OF_STATS][STRING_SIZE], int _jogos_jogados, char _player_name[STRING_SIZE]
                      , long int _pontuacao, int _maior_peca, int _tempo_decorrido, int _resultado_do_jogo)
{
    //copia para o vetor stats as estatísticas do jogo:
    strcpy(_stats[_jogos_jogados - 1][0], _player_name); //nome do jogador
    snprintf(_stats[_jogos_jogados - 1][1], STRING_SIZE,"%ld", _pontuacao); //pontuação
    snprintf(_stats[_jogos_jogados - 1][2], STRING_SIZE,"%ld", dois_elevado_a(_maior_peca)); //maior peça
    snprintf(_stats[_jogos_jogados - 1][3], STRING_SIZE,"%d", _tempo_decorrido); //tempo decorrido
    //resultado do jogo
    switch(_resultado_do_jogo)
        {
            case -1:
                strcpy(_stats[_jogos_jogados - 1][4], "Derrota");
                break;
            case 1:
                strcpy(_stats[_jogos_jogados - 1][4], "Vitória");
                break;
            default:
                strcpy(_stats[_jogos_jogados - 1][4], "Não terminado");
        }
}

//adiciona no ficheiro stats.txt as estatísticas de todos os jogos jogados durante o funcionamento da aplicação
void writeStats_file(char _stats[MAX_GAMES_PLAYED][NUMBER_OF_STATS][STRING_SIZE], int _jogos_jogados, char _stats_file[STRING_SIZE])
{
    int i = 0;
    int j = 0;
    FILE* fp = NULL;
    fp = fopen(_stats_file, "a+"); //abre um ficheiro no modo no modo "anexar"
    if(fp == NULL)
    {
        printf("Não foi possível abrir o ficheiro de estatísticas\n");
        return;
    }


    //verifica se o ficheiro está vazio
    fseek(fp, 0, SEEK_END);
    if(ftell(fp) == 0)
    {
        //se estiver vazio começa por escrever esta linha
        fprintf(fp, "Nome | Pontuação | Peça mais alta | Tempo decorrido | Resultado |\n");
    }

    for(j = 0; j < _jogos_jogados; j ++)
    {
        for(i = 0; i < NUMBER_OF_STATS; i++)
        {
            fprintf(fp, "%s", _stats[j][i]);
            fprintf(fp,"\t\t|\t");
        }
        fprintf(fp, "\n");
    }


    fclose(fp);
    printf("As estatísticas foram guardadas corretamente\n");

}

//função chamada quando o jogador faz undo - "retrocede" a pontuação e o tabuleiro
void undo(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _matriz_anterior[MAX_BOARD_POS][MAX_BOARD_POS],
          int _matrizes_undo[MAX_UNDOS][MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos, long int* _pontuacao,
          long int _pontuacoes_undo[MAX_UNDOS])
          {
                int i = 0;

                igualarMatrizes(_board, _matrizes_undo[0], _board_pos);
                igualarMatrizes(_matriz_anterior, _matrizes_undo[0], _board_pos);
                *_pontuacao = _pontuacoes_undo[0];

                for(i = 0; i < MAX_UNDOS - 1; i++)
                {
                    igualarMatrizes(_matrizes_undo[i], _matrizes_undo[i + 1], _board_pos);
                    _pontuacoes_undo[i] = _pontuacoes_undo[i + 1];
                }
          }

//escreve no ficheiro saved_game.txt todas as informações necessárias para retomar um jogo em curso quando a aplicação é fechada
void saveGame(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _board_pos, char _saved_game_file_name[STRING_SIZE],
              char _player_name[STRING_SIZE], long int _pontuacao, int _tempo_decorrido, int _difficulty)
{
    int i = 0;
    int j = 0;
    FILE* fp = NULL;
    fp = fopen(_saved_game_file_name, "w"); //abre o ficheiro no modo de escrita
    if(fp == NULL)
    {
        printf("Não foi possível gravar o jogo\n");
        return;
    }
    //escreve todas as informações necessárias para carregar o jogo quando o utilizador volta a carregar a aplicação
    fprintf(fp,"PlayerName: %s\n", _player_name); //nome do jogador
    fprintf(fp,"Pontuação: %ld\n", _pontuacao); //pontuação
    fprintf(fp, "Tempo: %d\n", _tempo_decorrido); //tempo decorrido
    fprintf(fp, "TamanhoTabuleiro: %d\n", _board_pos); //tamanho do tabuleiro
    fprintf(fp, "Dificuldade: %d\n", _difficulty); //dificuldade
    //tabuleiro
    for(j = 0; j < MAX_BOARD_POS; j ++)
    {
        for(i = 0; i < MAX_BOARD_POS; i++)
        {
            fprintf(fp, "%d ", _board[j][i]);

        }
        fprintf(fp, "\n");
    }


    fclose(fp);
    printf("O jogo foi guardado corretamente\n");
}

//pergunta ao jogador se pretende retomar o jogo gravado no ficheiro saved_game.txt caso pretenda, carrega todas as informações
//necessárias do ficheiro para as variáveis respetivas. Verifica também se estes dados são válidos. Retorna 0 sempre que ocorra
//algo inesperado
int restoreSavedGame(char _saved_game_file_name[STRING_SIZE], char _player_name[STRING_SIZE], int _board[MAX_BOARD_POS][MAX_BOARD_POS],
                      int* _board_pos, long int* _pontuacao, int* _tempo_inicial, int* _jogo_em_curso, int* _difficulty, int* _jogos_jogados)
{
    //variável auxiliar onde são guardadas strings do ficheiro
    char buffer[STRING_SIZE] = {0};
    int i = 0;
    int j = 0;
    //resposta dada pelo jogador (s/n)
    char resposta;
    int ret = 0;
    int tempo_jogo = 0;
    FILE* fp = NULL;

    fp = fopen(_saved_game_file_name, "r"); //abre o ficheiro em modo de leitura

    if(fp == NULL)
    {
        return 0;
    }

    //lê o nome do jogador e pergunta se deseja retomar o jogo
    fgets(buffer, STRING_SIZE, fp);
    ret = sscanf(buffer, "PlayerName: %s", _player_name);

    if(ret == 1 && strlen(_player_name) <= 8)
        printf("%s, deseja retomar o jogo guardado? [s/n] ", _player_name);
    else
    {
        fclose(fp);
        fclose(fopen(_saved_game_file_name, "w")); //apaga todo o conteúdo do ficheiro se o nome for inválido
        return 0;
    }

    do{
        fgets(buffer, STRING_SIZE, stdin);
        sscanf(buffer, "%c",&resposta);
        if(resposta == 's' || resposta =='S')
        {
            //lê a pontuação
            fgets(buffer, STRING_SIZE, fp);
            sscanf(buffer,"Pontuação: %ld", _pontuacao);
            if(*_pontuacao < 0)
            {
                printf("Dados inválidos. Não foi possível carregar o jogo guardado. Um novo jogo será iniciado\n");
                fclose(fp);
                fclose(fopen(_saved_game_file_name, "w")); //apaga todo o conteúdo do ficheiro se a pontuação for inválida
                return 0;
            }
            printf("Pontuação: %ld\n", *_pontuacao);

            //lê o tempo de jogo
            fgets(buffer, STRING_SIZE, fp);
            sscanf(buffer, "Tempo: %d", &tempo_jogo);
            if(tempo_jogo < 0)
            {
                printf("Dados inválidos. Não foi possível carregar o jogo guardado. Um novo jogo será iniciado\n");
                fclose(fp);
                fclose(fopen(_saved_game_file_name, "w")); //apaga todo o conteúdo do ficheiro se o tempo for inválido
                return 0;
            }
            *_tempo_inicial = time(NULL) - tempo_jogo;
            printf("Tempo de jogo: %d\n", tempo_jogo);

            //lê o tamanho do tabuleiro
            fgets(buffer, STRING_SIZE, fp);
            sscanf(buffer, "TamanhoTabuleiro: %d", _board_pos);
            if(*_board_pos < MIN_BOARD_POS || *_board_pos > MAX_BOARD_POS)
            {
                printf("Dados inválidos. Não foi possível carregar o jogo guardado. Um novo jogo será iniciado\n");
                fclose(fp);
                fclose(fopen(_saved_game_file_name, "w")); //apaga todo o conteúdo do ficheiro se o tamanho for inválido
                return 0;
            }
            printf("Tamanho do tabuleiro: %dx%d\n", *_board_pos, *_board_pos);

            //lê a dificuldade
            fgets(buffer, STRING_SIZE, fp);
            sscanf(buffer, "Dificuldade: %d\n", _difficulty);
            if(*_difficulty > MAX_LEVELS || *_difficulty < MIN_LEVELS)
            {
                printf("Dados inválidos. Não foi possível carregar o jogo guardado. Um novo jogo será iniciado\n");
                fclose(fp);
                fclose(fopen(_saved_game_file_name, "w")); //apaga todo o conteúdo do ficheiro se a dificuldade for inválida
                return 0;
            }
            printf("Dificuldade: %ld\n", dois_elevado_a(*_difficulty));

            //lê todas as peças do tabuleiro
            for(i = 0; i < MAX_BOARD_POS; i++)
            {
                fgets(buffer, STRING_SIZE, fp);
                sscanf(buffer, "%d %d %d %d %d %d %d %d %d %d %d ", &_board[i][0], &_board[i][1], &_board[i][2], &_board[i][3],
                       &_board[i][4], &_board[i][5], &_board[i][6], &_board[i][7], &_board[i][8], &_board[i][9], &_board[i][10]);
                for(j = 0; j < MAX_BOARD_POS; j++)
                {
                    if(_board[i][j] > MAX_LEVELS || _board[i][j] < 0)
                    {
                        printf("Dados inválidos. Não foi possível carregar o jogo guardado. Um novo jogo será iniciado\n");
                        fclose(fp);
                        fclose(fopen(_saved_game_file_name, "w")); //apaga todo o conteúdo do ficheiro se alguma peça for inválida
                        return 0;
                    }
                }
            }

            *_jogo_em_curso = 1;
            (*_jogos_jogados) ++;
            fclose(fp);
            fclose(fopen(_saved_game_file_name, "w")); //quando o jogo é carregado, é apagado do ficheiro
            return 1;
        }

        else if(resposta == 'n' || resposta =='N')
        {
            fclose(fp);
            fclose(fopen(_saved_game_file_name, "w")); //apaga o jogo gravado se o jogador não quiser retomar o jogo guardado
            return 0;
        }
        else
             printf("Resposta inválida, deseja retomar o jogo guardado? [s/n] ");

    }while(1);

}
