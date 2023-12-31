#include "engine.h"

/*
engine vs engine
engine vs player
player vs engine
player vs player
*/

typedef struct
{
  char name[64];
  char author[64];
  int stdin;
  int stdout;
} Engine;

typedef struct
{
  char name[64];
} Player;

bool write_engine(Engine engine, const char string[])
{
  // printf("To engine: (%s)\n", string);

  char writeChar;
  for(int index = 0; index < strlen(string); index++)
  {
    writeChar = string[index];

    if(writeChar == '\n' || writeChar == '\0') break;

    if(write(engine.stdin, &writeChar, sizeof(writeChar)) < 1) return false;
  }

  writeChar = '\n';
  if(write(engine.stdin, &writeChar, sizeof(writeChar)) < 1) return false;

  return true;
}

bool read_engine(char* string, Engine engine)
{
  char readChar;
  int index = 0;
  int readReturn;

  while((readReturn = read(engine.stdout, &readChar, sizeof(readChar))) == 1)
  {
    if(readChar == '\n' || readChar == '\0') break;

    string[index++] = readChar;
  }

  // printf("From engine: (%s)\n", string);

  // return (readReturn != -1 && index > 0);
  return (readReturn != -1);
}

bool engine_connect(Engine* engine)
{
  if(!write_engine(*engine, "uci"))
  {
    printf("Could not write to engine\n");
    return false;
  }

  char readString[1024];
  
  do 
  {
    memset(readString, 0, sizeof(readString));

    if(!read_engine(readString, *engine))
    {
      printf("Can not read from engine\n");
      return false;
    }
  }
  while(strcmp(readString, "uciok"));

  if(!write_engine(*engine, "ucinewgame"))
  {
    printf("Could not write to engine\n");
    return false;
  }

  return true;
}

void engine_close(Engine engine)
{
  close(engine.stdin);
  close(engine.stdout);
}

bool engine1_open(Engine* engine1)
{
  printf("Trying to open engine1-stdin\n");

  engine1->stdin = open("engine1-stdin", O_WRONLY);

  if(engine1->stdin == -1)
  {
    printf("Could not open engine1-stdin\n");

    return false;
  }

  printf("Trying to open engine1-stdout\n");

  engine1->stdout = open("engine1-stdout", O_RDONLY);

  if(engine1->stdout == -1)
  {
    printf("Could not open engine1-stdout\n");

    close(engine1->stdin);

    return false;
  }
  return true;
}

bool engine2_open(Engine* engine2)
{
  engine2->stdin = open("engine2-stdin", O_WRONLY);

  if(engine2->stdin == -1)
  {
    printf("Could not open engine2-stdin\n");

    return false;
  }

  engine2->stdout = open("engine2-stdout", O_RDONLY);

  if(engine2->stdout == -1)
  {
    printf("Could not open engine2-stdout\n");

    close(engine2->stdin);

    return false;
  }
  return true;
}
bool stdin_string(char* string, const char prompt[])
{
  fflush(stdin);
  printf("%s", prompt);
  char buffer[1024];
  if(fgets(buffer, sizeof(buffer), stdin) == NULL) return false;
  return sscanf(buffer, "%[^\n]%*c", string);
}

void init_all(void)
{
  init_piece_lookup_masks();

  init_bishop_rook_relevant_bits();

  init_piece_lookup_attacks();

  init_board_lookup_lines();
}

const char FEN_START[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

void position_string(char* positionString, const char fenString[], MoveArray moveArray)
{
  sprintf(positionString, "position fen %s", fenString);

  if(moveArray.amount > 0)
  {
    strcat(positionString, " moves");
  }

  for(int index = 0; index < moveArray.amount; index++)
  {
    char moveString[8];
    move_string(moveString, moveArray.moves[index]);

    strcat(positionString, " ");

    strcat(positionString, moveString);
  }
}

void go_string(char* goString, Clock clock)
{
  sprintf(goString, "go wtime %ld btime %ld", clock.wtime, clock.btime);
}

Move input_engine_move(Engine engine, Position position, MoveArray moveArray, Clock clock)
{
  if(!write_engine(engine, "isready"))
  {
    printf("Could not write to engine\n");
    return MOVE_NONE;
  }

  char readString[1024];
  
  do 
  {
    memset(readString, 0, sizeof(readString));

    if(!read_engine(readString, engine))
    {
      printf("Can not read from engine\n");
      return MOVE_NONE;
    }
  }
  while(strcmp(readString, "readyok"));


  char positionString[1024];
  memset(positionString, 0, sizeof(positionString));

  position_string(positionString, FEN_START, moveArray);

  if(!write_engine(engine, positionString))
  {
    printf("Could not write to engine\n");
    return MOVE_NONE;
  }


  char goString[64];
  memset(goString, 0, sizeof(goString));

  go_string(goString, clock);

  if(!write_engine(engine, goString))
  {
    printf("Could not write to engine\n");
    return MOVE_NONE;
  }


  do 
  {
    memset(readString, 0, sizeof(readString));

    if(!read_engine(readString, engine))
    {
      printf("Can not read from engine\n");
      return MOVE_NONE;
    }
  }
  while(strncmp(readString, "bestmove", 8));

  Move move = parse_move(readString + 9);

  return complete_move(position.boards, move);
}

Move input_player_move(Player player, Position position)
{
  position_print(position);

  printf("Player: %s\n", player.name);

  char moveString[64];
  Move inputMove;

  do
  {
    if(!stdin_string(moveString, "Input Move: ")) return MOVE_NONE;

    inputMove = parse_move(moveString);

    inputMove = complete_move(position.boards, inputMove);
  }
  while(!move_fully_legal(position, inputMove));
  
  return inputMove;
}

bool position_alive(Position position)
{
  return true;
}

void player_vs_player(void)
{
  Player player1 = {"player1"};
  Player player2 = {"player2"};

  Position position;
  parse_fen(&position, FEN_START);

  /*
  MoveArray moveArray;

  moveArray.amount = 0;
  moveArray.moves[0] = 0;
  */

  while(position_alive(position))
  {
    if(position.side == SIDE_WHITE)
    {
      Move move = input_player_move(player1, position);

      if(move == MOVE_NONE) // The player stopped playing
      {
        break;
      }

      make_move(&position, move);
    }
    else
    {
      Move move = input_player_move(player2, position);

      if(move == MOVE_NONE) // The player stopped playing
      {
        break;
      }

      make_move(&position, move);
    }
  }
}

void player_vs_engine(void)
{
  Player player = {"player"};


  Engine engine;
  if(!engine1_open(&engine))
  {
    printf("Could not open engine\n");
    return;
  }

  if(!engine_connect(&engine))
  {
    printf("Engine could not connect\n");

    engine_close(engine);
    return;
  }
  

  Position position;
  parse_fen(&position, FEN_START);


  Clock clock = {60000, 60000, 0, 0};


  MoveArray moveArray;

  moveArray.amount = 0;
  moveArray.moves[0] = 0;

  while(position_alive(position))
  {
    if(position.side == SIDE_WHITE)
    {
      Move move = input_player_move(player, position);

      if(move == MOVE_NONE) // The player stopped playing
      {
        break;
      }

      make_move(&position, move);

      moveArray.moves[moveArray.amount++] = move;
    }
    else
    {
      position_print(position);

      Move move = input_engine_move(engine, position, moveArray, clock);

      if(move == MOVE_NONE) // The engine stopped playing
      {
        printf("Engine did ilegal move\n");
        break;
      }

      make_move(&position, move);

      moveArray.moves[moveArray.amount++] = move;
    }
  }

  write_engine(engine, "quit");

  engine_close(engine);
}



void engine_vs_player(void)
{
  Engine engine;
  if(!engine1_open(&engine))
  {
    printf("Could not open engine\n");
    return;
  }

  if(!engine_connect(&engine))
  {
    printf("Engine could not connect\n");

    engine_close(engine);
    return;
  }


  Player player = {"player"};


  Position position;
  parse_fen(&position, FEN_START);


  Clock clock = {10000, 10000, 0, 0};


  MoveArray moveArray;

  moveArray.amount = 0;
  moveArray.moves[0] = 0;


  while(position_alive(position))
  {
    if(position.side == SIDE_WHITE)
    {
      Move move = input_engine_move(engine, position, moveArray, clock);

      if(move == MOVE_NONE) // The engine stopped playing
      {
        break;
      }

      make_move(&position, move);
    }
    else
    {
      Move move = input_player_move(player, position);

      if(move == MOVE_NONE) // The player stopped playing
      {
        break;
      }

      make_move(&position, move);
    }
  }

  write_engine(engine, "quit");

  engine_close(engine);
}

void engine_vs_engine(void)
{
  Engine engine1;
  if(!engine1_open(&engine1))
  {
    printf("Could not open engine1\n");
    return;
  }

  if(!engine_connect(&engine1))
  {
    printf("Engine1 could not connect\n");

    engine_close(engine1);
    return;
  }

  Engine engine2;
  if(!engine1_open(&engine2))
  {
    printf("Could not open engine2\n");

    engine_close(engine1);
    return;
  }

  if(!engine_connect(&engine2))
  {
    printf("Engine2 could not connect\n");

    engine_close(engine1);
    engine_close(engine2);
    return;
  }


  Position position;
  parse_fen(&position, FEN_START);


  Clock clock = {10000, 10000, 0, 0};


  MoveArray moveArray;

  moveArray.amount = 0;
  moveArray.moves[0] = 0;


  while(position_alive(position))
  {
    if(position.side == SIDE_WHITE)
    {
      Move move = input_engine_move(engine1, position, moveArray, clock);

      if(move == MOVE_NONE) // Engine1 stopped playing
      {
        break;
      }

      make_move(&position, move);
    }
    else
    {
      Move move = input_engine_move(engine2, position, moveArray, clock);

      if(move == MOVE_NONE) // Engine2 stopped playing
      {
        break;
      }

      make_move(&position, move);
    }
  }

  write_engine(engine1, "quit");
  write_engine(engine2, "quit");

  engine_close(engine1);
  engine_close(engine2);
}

int main(int argc, char* argv[])
{
  Position position;
  parse_fen(&position, FEN_START);

  char fen[128];
  
  memset(fen, '\0', sizeof(fen));

  fen_create(fen, position);

  printf("fen: (%s)\n", fen);

  return 0;
}
